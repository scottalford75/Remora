
/*
Remora PRU firmware for LinuxCNC
Copyright (C) 2021  Scott Alford (scotta)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

// MBED includes
#include "mbed.h"
#include <cstdio>
#include <cerrno>
#include <string> 
#include "FATFileSystem.h"

#if defined TARGET_LPC176X || TARGET_STM32F1 
#include "SDBlockDevice.h"
#elif defined TARGET_SKRV2 || TARGET_OCTOPUS_446 || TARGET_BLACK_F407VE || TARGET_OCTOPUS_429
#include "SDIOBlockDevice.h"
#endif

#include "configuration.h"
#include "remora.h"

// libraries
#include "ArduinoJson.h"

// drivers
#include "RemoraComms.h"
#include "pin.h"
#include "softPwm.h"

// threads
#include "irqHandlers.h"
#include "interrupt.h"
#include "pruThread.h"
#include "createThreads.h"

// modules
#include "module.h"
#include "blink.h"
#include "debug.h"
#include "digitalPin.h"
#include "encoder.h"
#include "eStop.h"
#include "hardwarePwm.h"
#include "mcp4451.h"
#include "motorPower.h"
#include "pwm.h"
#include "rcservo.h"
#include "resetPin.h"
#include "stepgen.h"
#include "switch.h"
#include "temperature.h"
#include "tmc.h"
#include "qei.h"


/***********************************************************************
*                STRUCTURES AND GLOBAL VARIABLES                       *
************************************************************************/

// state machine
enum State {
    ST_SETUP = 0,
    ST_START,
    ST_IDLE,
    ST_RUNNING,
    ST_STOP,
    ST_RESET,
    ST_WDRESET
};

uint8_t resetCnt;

// boolean
volatile bool PRUreset;
bool configError = false;
bool threadsRunning = false;

// pointers to objects with global scope
pruThread* servoThread;
pruThread* baseThread;
pruThread* commsThread;

// unions for RX and TX data
//volatile rxData_t spiRxBuffer1;  // this buffer is used to check for valid data before moving it to rxData
//volatile rxData_t spiRxBuffer2;  // this buffer is used to check for valid data before moving it to rxData
volatile rxData_t rxBuffer;
volatile rxData_t rxData;
volatile txData_t txData;


// pointers to data
volatile rxData_t*  ptrRxData = &rxData;
volatile txData_t*  ptrTxData = &txData;
volatile int32_t* ptrTxHeader;  
volatile bool*    ptrPRUreset;
volatile int32_t* ptrJointFreqCmd[JOINTS];
volatile int32_t* ptrJointFeedback[JOINTS];
volatile uint8_t* ptrJointEnable;
volatile float*   ptrSetPoint[VARIABLES];
volatile float*   ptrProcessVariable[VARIABLES];
volatile uint8_t* ptrInputs;
volatile uint8_t* ptrOutputs;


/***********************************************************************
        OBJECTS etc                                           
************************************************************************/

// SD card access and Remora communication protocol
#if defined TARGET_SKRV1_4
    SDBlockDevice blockDevice(P0_9, P0_8, P0_7, P0_6);  // mosi, miso, sclk, cs
    RemoraComms comms(ptrRxData, ptrTxData);

#elif defined TARGET_SKRV2 || TARGET_OCTOPUS || TARGET_BLACK_F407VE || TARGET_OCTOPUS_PRO_429
    SDIOBlockDevice blockDevice;
    //RemoraComms comms(ptrRxData, ptrTxData, SPI1, PA_4);

#elif defined TARGET_ROBIN_E3
    SDBlockDevice blockDevice(PB_15, PB_14, PB_13, PA_15);  // mosi, miso, sclk, cs
    RemoraComms comms(ptrRxData, ptrTxData, SPI1, PA_4);

#elif defined TARGET_SKR_MINI_E3
    SDBlockDevice blockDevice(PA_7, PA_6, PA_5, PA_4);  // mosi, miso, sclk, cs
    RemoraComms comms(ptrRxData, ptrTxData, SPI1, PC_1);    // use PC_1 as "slave select"

#endif

// Watchdog
Watchdog& watchdog = Watchdog::get_instance();

// Json configuration file stuff
FATFileSystem fileSystem("fs");
FILE *jsonFile;
string strJson;
DynamicJsonDocument doc(JSON_BUFF_SIZE);
JsonObject module;

// ethernet testing stuff
// ----------------------------------------------------------------------
#include "WIZnetInterface.h"

#define SRC_PORT   27181
#define DST_PORT   27181
const char * SERVER_ADDRESS =   "10.10.10.11";  // Server IP address
const char * IP_Addr    =       "10.10.10.10";
const char * IP_Subnet  =       "255.255.255.0";
const char * IP_Gateway =       "10.10.10.1";

#define MOSI0               PA_7           
#define MISO0               PA_6
#define SCK0                PA_5
#define SSEL0               PA_4

SPI spi(MOSI0, MISO0, SCK0); // mosi, miso, sclk
WIZnetInterface eth(&spi, SSEL0, PC_4); // spi, cs, reset    
WIZnet_UDPSocket udp;
Endpoint server;


/***********************************************************************
        INTERRUPT HANDLERS - add NVIC_SetVector etc to setup()
************************************************************************/

// Add these to /thread/irqHandlers.h in the TARGET_target


/***********************************************************************
        ROUTINES
************************************************************************/

void readJsonConfig()
{
    printf("1. Reading json configuration file\n");

    // Try to mount the filesystem
    printf("Mounting the filesystem... ");
    fflush(stdout);
 
    int err = fileSystem.mount(&blockDevice);
    printf("%s\n", (err ? "Fail :(" : "OK"));
    if (err) {
        printf("No filesystem found... ");
        fflush(stdout);
     }

    // Open the config file
    printf("Opening \"/fs/config.txt\"... ");
    fflush(stdout);
    jsonFile = fopen("/fs/config.txt", "r+");
    printf("%s\n", (!jsonFile ? "Fail :(" : "OK"));

    fseek (jsonFile, 0, SEEK_END);
    int32_t length = ftell (jsonFile);
    fseek (jsonFile, 0, SEEK_SET);

    printf("Json config file lenght = %2d\n", length);

    strJson.reserve(length+1);

    while (!feof(jsonFile)) {
        int c = fgetc(jsonFile);
        strJson.push_back(c);
    }

    // Remove comments from next line to print out the JSON config file
    //printf("%s\n", strJson.c_str());

    printf("\rClosing \"/fs/config.txt\"... ");
    fflush(stdout);
    fclose(jsonFile);
}


void setup()
{
    printf("\n2. Setting up DMA and threads\n");

    // TODO: we can probably just deinit the blockdevice for all targets....?

    #if defined TARGET_STM32F4
    // deinitialise the SDIO device to avoid DMA issues with the SPI DMA Slave on the STM32F4
    blockDevice.deinit();
    #endif

    #if defined TARGET_SKR_MINI_E3
    // remove the SD device as we are sharing the SPI with the comms module
    blockDevice.deinit();
    #endif

    // initialise the Remora comms 
    comms.init();
    comms.start();

    createThreads();
}


void loadModules()
{
    printf("\n3. Parsing json configuration file\n");

    const char *json = strJson.c_str();

    // parse the json configuration file
    DeserializationError error = deserializeJson(doc, json);

    printf("Config deserialisation - ");

    switch (error.code())
    {
        case DeserializationError::Ok:
            printf("Deserialization succeeded\n");
            break;
        case DeserializationError::InvalidInput:
            printf("Invalid input!\n");
            configError = true;
            break;
        case DeserializationError::NoMemory:
            printf("Not enough memory\n");
            configError = true;
            break;
        default:
            printf("Deserialization failed\n");
            configError = true;
            break;
    }

    if (configError) return;

    printf("\n4. Loading modules\n");

    JsonArray Modules = doc["Modules"];

    // create objects from json data
    for (JsonArray::iterator it=Modules.begin(); it!=Modules.end(); ++it)
    {
        module = *it;
        
        const char* thread = module["Thread"];
        const char* type = module["Type"];

        if (!strcmp(thread,"Base"))
        {
            printf("\nBase thread object\n");

            if (!strcmp(type,"Stepgen"))
            {
                createStepgen();
            }
            else if (!strcmp(type,"Encoder"))
            {
                createEncoder();
            }
            else if (!strcmp(type,"RCServo"))
            {
                createRCServo();
            }
        }
        else if (!strcmp(thread,"Servo"))
        {
            printf("\nServo thread object\n");

            if (!strcmp(type, "eStop"))
            {
                createEStop();
            }
            else if (!strcmp(type, "Reset Pin"))
            {
                createResetPin();
            }
            else if (!strcmp(type, "Blink"))
            {
                createBlink();
            }
            else if (!strcmp(type,"Digital Pin"))
            {
                createDigitalPin();
            }
            else if (!strcmp(type,"PWM"))
            {
                createPWM();
            }
            else if (!strcmp(type,"Temperature"))
            { 
                createTemperature();
            }
            else if (!strcmp(type,"Switch"))
            {
                createSwitch();
            }
            else if (!strcmp(type,"QEI"))
            {
                createQEI();
            }
        }
        else if (!strcmp(thread,"On load"))
        {
            printf("\nOn load - run once module\n");


            if (!strcmp(type,"MCP4451")) // digipot
            {
				createMCP4451();
            }
            else if (!strcmp(type,"Motor Power"))
            {
                createMotorPower();
            }
            else if (!strcmp(type,"TMC2208"))
            {
                createTMC2208();
            }
            else if (!strcmp(type,"TMC2209"))
            {
                createTMC2209();
            }
        }
    }
}


void debugThreadHigh()
{
    //Module* debugOnB = new Debug("PC_1", 1);
    //baseThread->registerModule(debugOnB);

    //Module* debugOnS = new Debug("PC_3", 1);
    //servoThread->registerModule(debugOnS);

    //Module* debugOnC = new Debug("PE_6", 1);
    //commsThread->registerModule(debugOnC);
}

void debugThreadLow()
{
    //Module* debugOffB = new Debug("PC_1", 0);
    //baseThread->registerModule(debugOffB); 

    //Module* debugOffS = new Debug("PC_3", 0);
    //servoThread->registerModule(debugOffS);

    //commsThread->startThread();
    //Module* debugOffC = new Debug("PE_6", 0);
    //commsThread->registerModule(debugOffC); 
}

int ethernetInit()
{
    //-------------------------------------------------------
    spi.frequency(12000000);

    uint8_t mac[6];
    mbed_mac_address((char *)mac);

    printf("\nUsing Static address\n");
    int ret = eth.init(mac, IP_Addr, IP_Subnet, IP_Gateway);
    
    if (!ret) {
        printf("Initialized, MAC: %s\n", eth.getMACAddress());
    } else {
        printf("Error eth.init() - ret = %d\n", ret);
        return -1;
    }
 
    ret = eth.connect();
    if (!ret) {
        printf("IP: %s, MASK: %s, GW: %s\n",
                  eth.getIPAddress(), eth.getNetworkMask(), eth.getGateway());
    } else {
        printf("Error eth.connect() - ret = %d\n", ret);
        return -1;
    }
 

    if(udp.init() == -1)
    {
        printf("UDP init failed \n");
    }
    else
    {
        printf("UDP init ok \n");
    }

    if(udp.bind(SRC_PORT) == -1)
    {
        printf("UDP bind failed \n");
    }
    else
    {
        printf("UDP bind ok \n");
    }
 
    if(server.set_address(SERVER_ADDRESS, DST_PORT) == -1)
    {
        printf("Endpoint init failed \n");
    }
    else
    {
        printf("Endpoint init ok \n");
        printf("Endpoint IP %s\n", server.get_address());
        printf("Endpoint Port %i\n", server.get_port());
    }

    return 1;

    //-------------------------------------------------------
}

int main()
{
    int sentBytes, recBytes;

    enum State currentState;
    enum State prevState;

    //comms.setStatus(false);
    //comms.setError(false);
    currentState = ST_SETUP;
    prevState = ST_RESET;

    printf("\nRemora PRU - Programmable Realtime Unit\n");

    watchdog.start(2000);

    while(1)
    {
      // the main loop does very little, keeping the Watchdog serviced and
      // resetting the rxData buffer if there is a loss of SPI commmunication
      // with LinuxCNC. Everything else is done via DMA and within the
      // two threads- Base and Servo threads that run the Modules.

    watchdog.kick();

    switch(currentState){
        case ST_SETUP:
            // do setup tasks
            if (currentState != prevState)
            {
                printf("\n## Entering SETUP state\n");
            }
            prevState = currentState;

            readJsonConfig();
            setup();
            deserialiseJSON();
            configThreads();
            createThreads();
            //debugThreadHigh();
            loadModules();
            //debugThreadLow();
            ethernetInit();

            currentState = ST_START;
            break; 

        case ST_START:
            // do start tasks
            if (currentState != prevState)
            {
                printf("\n## Entering START state\n");
            }
            prevState = currentState;

            if (!threadsRunning)
            {
                // Start the threads
                printf("\nStarting the BASE thread\n");
                baseThread->startThread();
                
                printf("\nStarting the SERVO thread\n");
                servoThread->startThread();

                threadsRunning = true;

                // wait for threads to read IO before testing for PRUreset
                wait(1);
            }

            if (PRUreset)
            {
                // RPi outputs default is high until configured when LinuxCNC Remora component is started, PRUreset pin will be high
                // stay in start state until LinuxCNC is started
                currentState = ST_START;
            }
            else
            {
                currentState = ST_IDLE;
            }
            
            break;


        case ST_IDLE:
            // do something when idle
            if (currentState != prevState)
            {
                printf("\n## Entering IDLE state\n");
            }
            prevState = currentState;

            /*
            // check to see if there there has been SPI errors
            if (comms.getError())
            {
                printf("Communication data error\n");
                comms.setError(false);
            }

            //wait for SPI data before changing to running state
            if (comms.getStatus())
            {
                currentState = ST_RUNNING;
            }

            if (PRUreset) 
            {
                currentState = ST_WDRESET;
            }
            */

            // ethernet testing - jump straight into RUNNING
            currentState = ST_RUNNING;

            break;

        case ST_RUNNING:
            // do running tasks
            if (currentState != prevState)
            {
                printf("\n## Entering RUNNING state\n");
            }
            prevState = currentState;


            recBytes = udp.receiveFrom(server, (char*)&rxBuffer.rxBuffer, SPI_BUFF_SIZE);
            
            if ((recBytes == 4) && (rxBuffer.header == PRU_READ))
            {
                // read request from LinuxCNC
                txData.header = PRU_DATA;
                sentBytes = udp.sendTo(server, (char*)&txData.txBuffer, SPI_BUFF_SIZE);
            }
            else if ((recBytes == 64) && (rxBuffer.header == PRU_WRITE))
            {
                // write request from LinuxCNC
                // reply with a 4 byte header response only
                txData.header = PRU_ACKNOWLEDGE;
                sentBytes = udp.sendTo(server, (char*)&txData.txBuffer, sizeof(txData.header));

                // then move the data
                for (int i = 0; i < SPI_BUFF_SIZE; i++)
                {
                    rxData.rxBuffer[i] = rxBuffer.rxBuffer[i];
                }
            }
            else if (recBytes >= 0)
            {
                // error condition we still need to reply so LinuxCNC receive does not time out
                txData.header = PRU_ERR;
                sentBytes = udp.sendTo(server, (char*)&txData.txBuffer, sizeof(txData.header));
                printf("PRU_ERR\n");
            }
            if (sentBytes < 0) printf("Send error\n");
            sentBytes = 0;


            /*
            // check to see if there there has been SPI errors 
            if (comms.getError())
            {
                printf("Communication data error\n");
                comms.setError(false);
            }
            
            if (comms.getStatus())
            {
                // SPI data received by DMA
                resetCnt = 0;
                comms.setStatus(false);
            }
            else
            {
                // no data received by DMA
                resetCnt++;
            }

            if (resetCnt > SPI_ERR_MAX)
            {
                // reset threshold reached, reset the PRU
                printf("   Communication data error limit reached, resetting\n");
                resetCnt = 0;
                currentState = ST_RESET;
            }
            
            if (PRUreset) 
            {
                currentState = ST_WDRESET;
            }
            */

            break;

        case ST_STOP:
            // do stop tasks
            if (currentState != prevState)
            {
                printf("\n## Entering STOP state\n");
            }
            prevState = currentState;


            currentState = ST_STOP;
            break;

        case ST_RESET:
            // do reset tasks
            if (currentState != prevState)
            {
                printf("\n## Entering RESET state\n");
            }
            prevState = currentState;

            // set all of the rxData buffer to 0
            // rxData.rxBuffer is volatile so need to do this the long way. memset cannot be used for volatile
            printf("   Resetting rxBuffer\n");
            {
                int n = sizeof(rxData.rxBuffer);
                while(n-- > 0)
                {
                    rxData.rxBuffer[n] = 0;
                }
            }

            currentState = ST_IDLE;
            break;

        case ST_WDRESET:
            // do a watch dog reset
            printf("\n## Entering WDRESET state\n");

            // force a watchdog reset by looping here
            while(1){}

            break;
      }

    //wait(LOOP_TIME);
    wait_ns(100000);
    }
}
