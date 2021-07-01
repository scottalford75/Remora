
/*
Remora PRU "Smoothieboard" firmware for LinuxCNC
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
#include "SDBlockDevice.h"
#include "FATFileSystem.h"
#include "MODDMA.h"
#include "FastAnalogIn.h"
 
// Local includes
#include "configuration.h"
#include "remora.h"

#include "lib/ArduinoJson6/ArduinoJson.h"

#include "drivers/pin/pin.h"
#include "drivers/softPwm/softPwm.h"

#include "thread/pruThread.h"
#include "thread/interrupt.h"

#include "modules/module.h"
#include "modules/resetPin/resetPin.h"
#include "modules/digipot/mcp4451.h"
#include "modules/stepgen/stepgen.h"
#include "modules/blink/blink.h"
#include "modules/digitalPin/digitalPin.h"
#include "modules/encoder/encoder.h"
#include "modules/pwm/pwm.h"
#include "modules/pwm/hardwarePwm.h"
#include "modules/temperature/temperature.h"
#include "modules/tmcStepper/tmcStepper.h"
#include "modules/rcservo/rcservo.h"
#include "modules/switch/switch.h"
#include "modules/eStop/eStop.h"
#include "modules/qei/qei.h"

#include "sensors/thermistor/thermistor.h"


SDBlockDevice blockDevice(MOSI1, MISO1, SCK1, SSEL1);  // mosi, miso, sclk, cs
FATFileSystem fileSystem("fs");

// SPI slave - RPi is the SPI master
SPISlave spiSlave(MOSI0, MISO0, SCK0, SSEL0);

// DMA controller
MODDMA dma;

// Watchdog
Watchdog& watchdog = Watchdog::get_instance();

// Json configuration file stuff
FILE *jsonFile;
string strJson;
DynamicJsonDocument doc(JSON_BUFF_SIZE);


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
volatile uint8_t rejectCnt;

// boolean
static volatile bool SPIdata;
static volatile bool SPIdataError;
static volatile bool PRUreset;
bool configError = false;
bool threadsRunning = false;


// pointers to objects with global scope
pruThread* servoThread;
pruThread* baseThread;
pruThread* commsThread;
MODDMA_Config* spiDMArx1 = NULL;
MODDMA_Config* spiDMArx2 = NULL;
MODDMA_Config* spiDMAtx1 = NULL;
MODDMA_Config* spiDMAtx2 = NULL;
MODDMA_Config* spiDMAmemcpy1 = NULL;
MODDMA_Config* spiDMAmemcpy2 = NULL;


// unions for RX and TX data
volatile rxData_t spiRxBuffer1;  // this buffer is used to check for valid data before moveing it to rxData
volatile rxData_t spiRxBuffer2;  // this buffer is used to check for valid data before moveing it to rxData
volatile rxData_t rxData;
volatile txData_t txData;

// pointers to data
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
        INTERRUPT HANDLERS - add NVIC_SetVector etc to setup()
************************************************************************/


void TIMER0_IRQHandler()
{
    // Base thread interrupt handler
    unsigned int isrMask = LPC_TIM0->IR;
    LPC_TIM0->IR = isrMask; /* Clear the Interrupt Bit */

    Interrupt::TIMER0_Wrapper();
}


void TIMER1_IRQHandler(void)
{
    // Servo thread interrupt handler
    unsigned int isrMask = LPC_TIM1->IR;
    LPC_TIM1->IR = isrMask; /* Clear the Interrupt Bit */

    Interrupt::TIMER1_Wrapper();
}


void TIMER2_IRQHandler(void)
{
    // Servo thread interrupt handler
    unsigned int isrMask = LPC_TIM2->IR;
    LPC_TIM2->IR = isrMask; /* Clear the Interrupt Bit */

    Interrupt::TIMER2_Wrapper();
}


void QEI_IRQHandler(void)
{
    // QEI (quatrature encoder interface) index interrupt handler
    LPC_QEI->QEICLR = ((uint32_t)(1<<0));   
    Interrupt:: QEI_Wrapper();
}

void tc0_callback ()
{
    // SPI Tx
    MODDMA_Config *config = dma.getConfig();
    dma.Disable( (MODDMA::CHANNELS)config->channelNum() );

    // Clear DMA IRQ flags.
    if (dma.irqType() == MODDMA::TcIrq) dma.clearTcIrq();
    if (dma.irqType() == MODDMA::ErrIrq) dma.clearErrIrq();

    dma.Prepare( spiDMAtx2 );
}

void tc1_callback ()
{
    // SPI Tx
    MODDMA_Config *config = dma.getConfig();
    dma.Disable( (MODDMA::CHANNELS)config->channelNum() );

    // Clear DMA IRQ flags.
    if (dma.irqType() == MODDMA::TcIrq) dma.clearTcIrq();
    if (dma.irqType() == MODDMA::ErrIrq) dma.clearErrIrq();

    dma.Prepare( spiDMAtx1 );
}

void tc2_callback ()
{
    // SPI Rx
    MODDMA_Config *config = dma.getConfig();
    dma.Disable( (MODDMA::CHANNELS)config->channelNum() );

    SPIdata = false;
    SPIdataError = false;

    // Clear DMA IRQ flags.
    if (dma.irqType() == MODDMA::TcIrq) dma.clearTcIrq();
    if (dma.irqType() == MODDMA::ErrIrq) dma.clearErrIrq();

    // Check and move the recieved SPI data payload
    switch (spiRxBuffer1.header)
    {
      case PRU_READ:
        SPIdata = true;
        rejectCnt = 0;
        dma.Disable( spiDMAmemcpy2->channelNum()  );
        break;

      case PRU_WRITE:
        SPIdata = true;
        rejectCnt = 0;
        dma.Prepare( spiDMAmemcpy1 );
        break;

      default:
        rejectCnt++;
        if (rejectCnt > 5)
        {
            SPIdataError = true;
        }
        dma.Disable( spiDMAmemcpy2->channelNum()  );
    }

    // swap Rx buffers
    dma.Prepare( spiDMArx2 );
}

void tc3_callback ()
{
    // SPI Rx
    MODDMA_Config *config = dma.getConfig();
    dma.Disable( (MODDMA::CHANNELS)config->channelNum() );

    SPIdata = false;
    SPIdataError = false;

    // Clear DMA IRQ flags.
    if (dma.irqType() == MODDMA::TcIrq) dma.clearTcIrq();
    if (dma.irqType() == MODDMA::ErrIrq) dma.clearErrIrq();

    // Check and move the recieved SPI data payload
    switch (spiRxBuffer2.header)
    {
      case PRU_READ:
        SPIdata = true;
        rejectCnt = 0;
        dma.Disable( spiDMAmemcpy1->channelNum()  );
        break;

      case PRU_WRITE:
        SPIdata = true;
        rejectCnt = 0;
        dma.Prepare( spiDMAmemcpy2 );
        break;

      default:
        rejectCnt++;
        if (rejectCnt > 5)
        {
            SPIdataError = true;
        }
        dma.Disable( spiDMAmemcpy1->channelNum()  );
    }

    // swap Rx buffers
    dma.Prepare( spiDMArx1 );
}

void err_callback () {
    cout << "err\r\n" << endl;
}

void DMAsetup()
{
    // Create MODDMA configuration objects for the SPI transfer and memory copy
    spiDMAmemcpy1 = new MODDMA_Config;
    spiDMAmemcpy2 = new MODDMA_Config;
    spiDMAtx1 = new MODDMA_Config;
    spiDMAtx2 = new MODDMA_Config;
    spiDMArx1 = new MODDMA_Config;
    spiDMArx2 = new MODDMA_Config;

    // Setup DMA configurations
    spiDMAtx1
     ->channelNum    ( MODDMA::Channel_0 )
     ->srcMemAddr    ( (uint32_t) &txData )
     ->dstMemAddr    ( 0 )
     ->transferSize  ( SPI_BUFF_SIZE )
     ->transferType  ( MODDMA::m2p )
     ->srcConn       ( 0 )
     ->dstConn       ( MODDMA::SSP0_Tx )
     ->attach_tc     ( &tc0_callback )
     ->attach_err    ( &err_callback )
    ;

    spiDMAtx2
     ->channelNum    ( MODDMA::Channel_1 )
     ->srcMemAddr    ( (uint32_t) &txData )
     ->dstMemAddr    ( 0 )
     ->transferSize  ( SPI_BUFF_SIZE )
     ->transferType  ( MODDMA::m2p )
     ->srcConn       ( 0 )
     ->dstConn       ( MODDMA::SSP0_Tx )
     ->attach_tc     ( &tc1_callback )
     ->attach_err    ( &err_callback )
    ;

    spiDMArx1
     ->channelNum    ( MODDMA::Channel_2 )
     ->srcMemAddr    ( 0 )
     ->dstMemAddr    ( (uint32_t) &spiRxBuffer1 )
     ->transferSize  ( SPI_BUFF_SIZE )
     ->transferType  ( MODDMA::p2m )
     ->srcConn       ( MODDMA::SSP0_Rx )
     ->dstConn       ( 0 )
     ->attach_tc     ( &tc2_callback )
     ->attach_err    ( &err_callback )
    ;

    spiDMArx2
     ->channelNum    ( MODDMA::Channel_3 )
     ->srcMemAddr    ( 0 )
     ->dstMemAddr    ( (uint32_t) &spiRxBuffer2 )
     ->transferSize  ( SPI_BUFF_SIZE )
     ->transferType  ( MODDMA::p2m )
     ->srcConn       ( MODDMA::SSP0_Rx )
     ->dstConn       ( 0 )
     ->attach_tc     ( &tc3_callback )
     ->attach_err    ( &err_callback )
    ;

    spiDMAmemcpy1
     ->channelNum    ( MODDMA::Channel_4 )
     ->srcMemAddr    ( (uint32_t) &spiRxBuffer1 )
     ->dstMemAddr    ( (uint32_t) &rxData )
     ->transferSize  ( SPI_BUFF_SIZE )
     ->transferType  ( MODDMA::m2m )
    ;

    spiDMAmemcpy2
     ->channelNum    ( MODDMA::Channel_5 )
     ->srcMemAddr    ( (uint32_t) &spiRxBuffer2 )
     ->dstMemAddr    ( (uint32_t) &rxData )
     ->transferSize  ( SPI_BUFF_SIZE )
     ->transferType  ( MODDMA::m2m )
    ;

    // Pass the configurations to the controller
    dma.Prepare( spiDMArx1 );
    dma.Prepare( spiDMAtx1 );

    // Enable SSP0 for DMA
    LPC_SSP0->DMACR = 0;
    LPC_SSP0->DMACR = (1<<1)|(1<<0); // TX,RX DMA Enable
}

void DMAreset()
{
    // disable and re-prepare DMA SPI transfers
    printf("   Resetting DMA SPI transfers\n");
    
    // disable all transfers
    dma.Disable( spiDMAmemcpy1->channelNum()  );
    dma.Disable( spiDMAmemcpy2->channelNum()  );
    dma.Disable( spiDMAtx1->channelNum()  );
    dma.Disable( spiDMAtx2->channelNum()  );
    dma.Disable( spiDMArx1->channelNum()  );
    dma.Disable( spiDMArx2->channelNum()  );

    // pass the configurations to the controller
    dma.Prepare( spiDMArx1 );
    dma.Prepare( spiDMAtx1 );
}

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

    spiSlave.frequency(48000000);

    DMAsetup();

    txData.header = PRU_DATA;


    // Create the thread objects and set the interrupt vectors to RAM. This is needed
    // as we are using the USB bootloader that requires a different code starting
    // address. Also set interrupt priority with NVIC_SetPriority.
    //
    // Note: DMAC has highest priority, then Base thread and then Servo thread
    //       to ensure SPI data transfer is reliable

    NVIC_SetPriority(DMA_IRQn, 1);

    baseThread = new pruThread(LPC_TIM0, TIMER0_IRQn, PRU_BASEFREQ);
    NVIC_SetVector(TIMER0_IRQn, (uint32_t)TIMER0_IRQHandler);
    NVIC_SetPriority(TIMER0_IRQn, 2);

    servoThread = new pruThread(LPC_TIM1, TIMER1_IRQn, PRU_SERVOFREQ);
    NVIC_SetVector(TIMER1_IRQn, (uint32_t)TIMER1_IRQHandler);
    NVIC_SetPriority(TIMER1_IRQn, 3);

    commsThread = new pruThread(LPC_TIM2, TIMER2_IRQn, PRU_COMMSFREQ);
    NVIC_SetVector(TIMER2_IRQn, (uint32_t)TIMER2_IRQHandler);
    NVIC_SetPriority(TIMER2_IRQn, 4);

    // Other interrupt sources

    // for QEI modudule
    NVIC_SetVector(QEI_IRQn, (uint32_t)QEI_IRQHandler);
    NVIC_SetPriority(QEI_IRQn, 5);
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

    JsonArray Modules = doc["Modules"];

    // create objects from json data
    for (JsonArray::iterator it=Modules.begin(); it!=Modules.end(); ++it)
    {
        JsonObject module = *it;
        
        const char* thread = module["Thread"];
        const char* type = module["Type"];

        if (!strcmp(thread,"Base"))
        {
            printf("\nBase thread object\n");

            if (!strcmp(type,"Stepgen"))
            {
                const char* comment = module["Comment"];
                printf("%s\n",comment);

                int joint = module["Joint Number"];
                const char* enable = module["Enable Pin"];
                const char* step = module["Step Pin"];
                const char* dir = module["Direction Pin"];

                // configure pointers to data source and feedback location
                ptrJointFreqCmd[joint] = &rxData.jointFreqCmd[joint];
                ptrJointFeedback[joint] = &txData.jointFeedback[joint];
                ptrJointEnable = &rxData.jointEnable;

                // create the step generator, register it in the thread
                Module* stepgen = new Stepgen(PRU_BASEFREQ, joint, enable, step, dir, STEPBIT, *ptrJointFreqCmd[joint], *ptrJointFeedback[joint], *ptrJointEnable);
                baseThread->registerModule(stepgen);
            }
            else if (!strcmp(type,"Encoder"))
            {
                const char* comment = module["Comment"];
                printf("%s\n",comment);
    
                int pv = module["PV[i]"];
                const char* pinA = module["ChA Pin"];
                const char* pinB = module["ChB Pin"];
                const char* pinI = module["Index Pin"];
                int dataBit = module["Data Bit"];
                const char* modifier = module["Modifier"];
            
                printf("Creating Quadrature Encoder at pins %s and %s\n", pinA, pinB);

                int mod;

                if (!strcmp(modifier,"Open Drain"))
                {
                    mod = OPENDRAIN;
                }
                else if (!strcmp(modifier,"Pull Up"))
                {
                    mod = PULLUP;
                }
                else if (!strcmp(modifier,"Pull Down"))
                {
                    mod = PULLDOWN;
                }
                else if (!strcmp(modifier,"Pull None"))
                {
                    mod = PULLNONE;
                }
                else
                {
                    mod = NONE;
                }
                
                ptrProcessVariable[pv]  = &txData.processVariable[pv];
                ptrInputs = &txData.inputs;

                if (pinI == nullptr)
                {
                    Module* encoder = new Encoder(*ptrProcessVariable[pv], pinA, pinB, mod);
                    baseThread->registerModule(encoder);
                }
                else
                {
                    printf("  Encoder has index at pin %s\n", pinI);
                    Module* encoder = new Encoder(*ptrProcessVariable[pv], *ptrInputs, dataBit, pinA, pinB, pinI, mod);
                    baseThread->registerModule(encoder);
                }

            }
            else if (!strcmp(type,"RCServo"))
            {
                const char* comment = module["Comment"];
                printf("%s\n",comment);
    
                int sp = module["SP[i]"];
                const char* pin = module["Servo Pin"];
            
                printf("Make RC Servo at pin %s\n", pin);
                
                ptrSetPoint[sp] = &rxData.setPoint[sp];

                // slow module with 10 hz update
                int updateHz = 10;
                Module* rcservo = new RCServo(*ptrSetPoint[sp], pin, PRU_BASEFREQ, updateHz);
                baseThread->registerModule(rcservo);

            }
        }
        else if (!strcmp(thread,"Servo"))
        {
            printf("\nServo thread object\n");

            if (!strcmp(type, "eStop"))
            {
                const char* comment = module["Comment"];
                printf("%s\n",comment);
    
                const char* pin = module["Pin"];
            
                ptrTxHeader = &txData.header;
    
                printf("Make eStop at pin %s\n", pin);

                Module* estop = new eStop(*ptrTxHeader, pin);
                servoThread->registerModule(estop);

            }
            else if (!strcmp(type, "Reset Pin"))
            {
                const char* comment = module["Comment"];
                printf("%s\n",comment);
    
                const char* pin = module["Pin"];
            
                ptrPRUreset = &PRUreset;
    
                printf("Make Reset Pin at pin %s\n", pin);

                Module* resetPin = new ResetPin(*ptrPRUreset, pin);
                servoThread->registerModule(resetPin);

            }
            else if (!strcmp(type, "Blink"))
            {
                const char* pin = module["Pin"];
                int frequency = module["Frequency"];
                
                printf("Make Blink at pin %s\n", pin);
                    
                Module* blink = new Blink(pin, PRU_SERVOFREQ, frequency);
                servoThread->registerModule(blink);
            }
            else if (!strcmp(type,"Digital Pin"))
            {
                const char* comment = module["Comment"];
                printf("%s\n",comment);
    
                const char* pin = module["Pin"];
                const char* mode = module["Mode"];
                bool invert = module["Invert"];
                const char* modifier = module["Modifier"];
                int dataBit = module["Data Bit"];

                int mod;

                if (!strcmp(modifier,"Open Drain"))
                {
                    mod = OPENDRAIN;
                }
                else if (!strcmp(modifier,"Pull Up"))
                {
                    mod = PULLUP;
                }
                else if (!strcmp(modifier,"Pull Down"))
                {
                    mod = PULLDOWN;
                }
                else if (!strcmp(modifier,"Pull None"))
                {
                    mod = PULLNONE;
                }
                else
                {
                    mod = NONE;
                }

                ptrOutputs = &rxData.outputs;
                ptrInputs = &txData.inputs;
    
                printf("Make Digital %s at pin %s\n", mode, pin);
    
                if (!strcmp(mode,"Output"))
                {
                    //Module* digitalPin = new DigitalPin(*ptrOutputs, 1, pin, dataBit, invert);
                    Module* digitalPin = new DigitalPin(*ptrOutputs, 1, pin, dataBit, invert, mod);
                    servoThread->registerModule(digitalPin);
                }
                else if (!strcmp(mode,"Input"))
                {
                    //Module* digitalPin = new DigitalPin(*ptrInputs, 0, pin, dataBit, invert);
                    Module* digitalPin = new DigitalPin(*ptrInputs, 0, pin, dataBit, invert, mod);
                    servoThread->registerModule(digitalPin);
                }
                else
                {
                    printf("Error - incorrectly defined Digital Pin\n");
                }
            }
            else if (!strcmp(type,"PWM"))
            {
                const char* comment = module["Comment"];
                printf("%s\n",comment);
    
                int sp = module["SP[i]"];
                int pwmMax = module["PWM Max"];
                const char* pin = module["PWM Pin"];

                const char* hardware = module["Hardware PWM"];
                const char* variable = module["Variable Freq"];
                int period_sp = module["Period SP[i]"];
                int period = module["Period us"];
            
                printf("Make PWM at pin %s\n", pin);
                
                ptrSetPoint[sp] = &rxData.setPoint[sp];

                if (!strcmp(hardware,"True"))
                {
                    // Hardware PWM
                    if (!strcmp(variable,"True"))
                    {
                        // Variable frequency hardware PWM
                        ptrSetPoint[period_sp] = &rxData.setPoint[period_sp];

                        Module* pwm = new HardwarePWM(*ptrSetPoint[period_sp], *ptrSetPoint[sp], period, pin);
                        servoThread->registerModule(pwm);
                    }
                    else
                    {
                        // Fixed frequency hardware PWM
                        Module* pwm = new HardwarePWM(*ptrSetPoint[sp], period, pin);
                        servoThread->registerModule(pwm);
                    }
                }
                else
                {
                    // Software PWM
                    if (pwmMax != 0) // use configuration file value for pwmMax - useful for 12V on 24V systems
                    {
                        Module* pwm = new PWM(*ptrSetPoint[sp], pin, pwmMax);
                        servoThread->registerModule(pwm);
                    }
                    else // use default value of pwmMax
                    {
                        Module* pwm = new PWM(*ptrSetPoint[sp], pin);
                        servoThread->registerModule(pwm);
                    }
                }
            }
            else if (!strcmp(type,"Temperature"))
            { 
                printf("Make Temperature measurement object\n");
                const char* comment = module["Comment"];
                printf("%s\n",comment);

                int pv = module["PV[i]"];
                const char* sensor = module["Sensor"];

                ptrProcessVariable[pv]  = &txData.processVariable[pv];

                if (!strcmp(sensor, "Thermistor"))
                {
                    const char* pinSensor = module["Thermistor"]["Pin"];
                    float beta =  module["Thermistor"]["beta"];
                    int r0 = module["Thermistor"]["r0"];
                    int t0 = module["Thermistor"]["t0"];

                    // slow module with 1 hz update
                    int updateHz = 1;
                    Module* temperature = new Temperature(*ptrProcessVariable[pv], PRU_SERVOFREQ, updateHz, sensor, pinSensor, beta, r0, t0);
                    servoThread->registerModule(temperature);
                }
            }
            else if (!strcmp(type,"Switch"))
            {
                const char* comment = module["Comment"];
                printf("%s\n",comment);
    
                const char* pin = module["Pin"];
                const char* mode = module["Mode"];
                int pv = module["PV[i]"];
                float sp = module["SP"];
            
                printf("Make Switch (%s) at pin %s\n", mode, pin);
    
                if (!strcmp(mode,"On"))
                {
                    Module* SoftSwitch = new Switch(sp, *ptrProcessVariable[pv], pin, 1);
                    servoThread->registerModule(SoftSwitch);
                }
                else if (!strcmp(mode,"Off"))
                {
                    Module* SoftSwitch = new Switch(sp, *ptrProcessVariable[pv], pin, 0);
                    servoThread->registerModule(SoftSwitch);
                }
                else
                {
                    printf("Error - incorrectly defined Switch\n");
                }
            }
             else if (!strcmp(type,"QEI"))
            {
                const char* comment = module["Comment"];
                printf("%s\n",comment);
    
                int pv = module["PV[i]"];
                int dataBit = module["Data Bit"];
                const char* index = module["Enable Index"];
            
                printf("Creating QEI, hardware quadrature encoder interface\n");
           
                ptrProcessVariable[pv]  = &txData.processVariable[pv];
                ptrInputs = &txData.inputs;

                if (!strcmp(index,"True"))
                {
                    printf("  Encoder has index\n");
                    Module* qei = new QEI(*ptrProcessVariable[pv], *ptrInputs, dataBit);
                    baseThread->registerModule(qei);
                }
                else
                {
                    Module* qei = new QEI(*ptrProcessVariable[pv]);
                    baseThread->registerModule(qei);
                }
            }
        }
        else if (!strcmp(thread,"On load"))
        {
            printf("\nOn load - run once module\n");

            if (!strcmp(type,"MCP4451")) // digipot
            {
                printf("Make MCP4451 Digipot object\n");

                const char* sda = module["I2C SDA pin"];
                const char* scl = module["I2C SCL pin"];
                int address = module["I2C address"];
                float maxCurrent = module["Max current"];
                float factor = module["Factor"];
                float c0 = module["Current 0"];
                float c1 = module["Current 1"];
                float c2 = module["Current 2"];
                float c3 = module["Current 3"];

                Module* digipot = new MCP4451(sda, scl, address, maxCurrent, factor, c0, c1, c2, c3);
                digipot->update();
                delete digipot;
            }
            else if (!strcmp(type,"TMC stepper"))
            {
                printf("Make TMC");

                const char* driver = module["Driver"];
                printf("%s\n", driver);

                const char* comment = module["Comment"];
                printf("%s\n",comment);

                const char* RxPin = module["RX pin"];
                float RSense = module["RSense"];
                uint8_t address = module["Address"];
                uint16_t current = module["Current"];
                uint16_t microsteps = module["Microsteps"];
                const char* stealth = module["Stealth chop"];
                uint16_t stall = module["Stall sensitivity"];

                bool stealthchop;

                if (!strcmp(stealth, "on"))
                {
                    stealthchop = true;
                }
                else
                {
                    stealthchop = false;   
                }

                printf("%s\n", driver);

                if (!strcmp(driver, "2208"))
                {
                    // SW Serial pin, RSense, mA, microsteps, stealh
                    // TMC2208(std::string, float, uint8_t, uint16_t, uint16_t, bool);
                    Module* tmc = new TMC2208(RxPin, RSense, current, microsteps, stealthchop);
                
                    printf("\nStarting the COMMS thread\n");
                    commsThread->startThread();
                    commsThread->registerModule(tmc);
                    
                    tmc->configure();

                    printf("\nStopping the COMMS thread\n");
                    commsThread->stopThread();
                    commsThread->unregisterModule(tmc);
                    delete tmc;
                }
                else if (!strcmp(driver, "2209"))
                {
                    // SW Serial pin, RSense, addr, mA, microsteps, stealh, stall
                    // TMC2209(std::string, float, uint8_t, uint16_t, uint16_t, bool, uint16_t);
                    Module* tmc = new TMC2209(RxPin, RSense, address, current, microsteps, stealthchop, stall);
                
                    printf("\nStarting the COMMS thread\n");
                    commsThread->startThread();
                    commsThread->registerModule(tmc);
                    
                    tmc->configure();

                    printf("\nStopping the COMMS thread\n");
                    commsThread->stopThread();
                    commsThread->unregisterModule(tmc);
                    delete tmc;
                }
            }
        }
    }
}


int main() {
    
    enum State currentState;
    enum State prevState;

    SPIdata = false;
    SPIdataError = false;
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
                printf("## Entering SETUP state\n");
            }
            prevState = currentState;

            readJsonConfig();
            setup();
            loadModules();

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
                // RPi outputs default is high until configured when LinuxCNC spiPRU component is started, PRUreset pin will be high
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

            // check to see if there there has been SPI errors
            if (SPIdataError)
            {
                printf("SPI data error:\n");
                printf("    spiRxBuffer1.header = %x\n", spiRxBuffer1.header);
                printf("    spiRxBuffer2.header = %x\n", spiRxBuffer2.header);
                SPIdataError = false;
            }

            //wait for SPI data before changing to running state
            if (SPIdata)
            {
                currentState = ST_RUNNING;
            }

            if (PRUreset) 
            {
                currentState = ST_WDRESET;
            }

            break;

        case ST_RUNNING:
            // do running tasks
            if (currentState != prevState)
            {
                printf("\n## Entering RUNNING state\n");
            }
            prevState = currentState;

            // check to see if there there has been SPI errors 
            if (SPIdataError)
            {
                printf("SPI data error:\n");
                printf("    spiRxBuffer1.header = %x\n", spiRxBuffer1.header);
                printf("    spiRxBuffer2.header = %x\n", spiRxBuffer2.header);
                SPIdataError = false;
            }
            
            if (SPIdata)
            {
                // SPI data received by DMA
                resetCnt = 0;
                SPIdata = false;
            }
            else
            {
                // no SPI data received by DMA
                resetCnt++;
            }

            if (resetCnt > SPI_ERR_MAX)
            {
                // reset threshold reached, reset the PRU
                printf("   SPI data error limit reached, resetting\n");
                resetCnt = 0;
                currentState = ST_RESET;
            }

            if (PRUreset) 
            {
                currentState = ST_WDRESET;
            }

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

            // reset DMA transfers - this does not work properly... :-( ??
            //LPC_SPI->SPCR |= (0<<2);    // reset SPI enable bit, this will clear the 16-byte Rx FIFO
            //DMAreset();
            //LPC_SPI->SPCR |= (1<<2);

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

    wait(LOOP_TIME);
    }
}
