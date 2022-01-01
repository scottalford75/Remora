/* Copyright (C) 2012 mbed.org, MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#pragma once

#include "mbed.h"
#include "mbed_debug.h"

#define TEST_ASSERT(A) while(!(A)){debug("\n\n%s@%d %s ASSERT!\n\n",__PRETTY_FUNCTION__,__LINE__,#A);exit(1);};

#define DEFAULT_WAIT_RESP_TIMEOUT 500

#define SOCK_ERROR            0        
#define SOCKERR_SOCKNUM       (SOCK_ERROR - 1)     ///< Invalid socket number
#define SOCKERR_SOCKOPT       (SOCK_ERROR - 2)     ///< Invalid socket option
#define SOCKERR_SOCKINIT      (SOCK_ERROR - 3)     ///< Socket is not initialized
#define SOCKERR_SOCKCLOSED    (SOCK_ERROR - 4)     ///< Socket unexpectedly closed.
#define SOCKERR_SOCKMODE      (SOCK_ERROR - 5)     ///< Invalid socket mode for socket operation.
#define SOCKERR_SOCKFLAG      (SOCK_ERROR - 6)     ///< Invalid socket flag
#define SOCKERR_SOCKSTATUS    (SOCK_ERROR - 7)     ///< Invalid socket status for socket operation.
#define SOCKERR_ARG           (SOCK_ERROR - 10)    ///< Invalid argrument.
#define SOCKERR_PORTZERO      (SOCK_ERROR - 11)    ///< Port number is zero
#define SOCKERR_IPINVALID     (SOCK_ERROR - 12)    ///< Invalid IP address
#define SOCKERR_TIMEOUT       (SOCK_ERROR - 13)    ///< Timeout occurred
#define SOCKERR_DATALEN       (SOCK_ERROR - 14)    ///< Data length is zero or greater than buffer max size.
#define SOCKERR_BUFFER        (SOCK_ERROR - 15)    ///< Socket buffer is not enough for data communication.

#define SOCK_ANY_PORT_NUM  0xC000;


#define MAX_SOCK_NUM 8

#define MR        0x0000
#define GAR       0x0001
#define SUBR      0x0005
#define SHAR      0x0009
#define SIPR      0x000f
#define IR        0x0015
#define IMR       0x0016
#define SIR       0x0017
#define SIMR      0x0018
#define RTR       0x0019
#define RCR       0x001b
#define UIPR      0x0028
#define UPORTR    0x002c
#define PHYCFGR   0x002e

// W5500 socket register
#define Sn_MR         0x0000
#define Sn_CR         0x0001
#define Sn_IR         0x0002
#define Sn_SR         0x0003
#define Sn_PORT       0x0004
#define Sn_DHAR       0x0006
#define Sn_DIPR       0x000c
#define Sn_DPORT      0x0010
#define Sn_RXBUF_SIZE 0x001e
#define Sn_TXBUF_SIZE 0x001f
#define Sn_TX_FSR     0x0020
#define Sn_TX_RD      0x0022
#define Sn_TX_WR      0x0024
#define Sn_RX_RSR     0x0026
#define Sn_RX_RD      0x0028
#define Sn_RX_WR      0x002a
#define Sn_IMR        0x002c


//Define for Socket Command register option value
#define Sn_CR_OPEN      0x01
#define Sn_CR_LISTEN    0x02
#define Sn_CR_CONNECT   0x04
#define Sn_CR_DISCON    0x08
#define Sn_CR_CLOSE     0x10
#define Sn_CR_SEND      0x20
#define Sn_CR_SEND_MAC  0x21
#define Sn_CR_SEND_KEEP 0x22
#define Sn_CR_RECV      0x40


//Define for Socket Mode register option value
#define Sn_MR_CLOSE   0x00
#define Sn_MR_TCP     0x01
#define Sn_MR_UDP     0x02
#define Sn_MR_MACRAW  0x04
#define Sn_MR_UCASTB  0x10
#define Sn_MR_ND      0x20
#define Sn_MR_BCASTB  0x40
#define Sn_MR_MULTI   0x80

#define Sn_IR_SENDOK                 0x10

//Sn_IR values

#define Sn_IR_TIMEOUT                0x08
#define Sn_IR_RECV                   0x04
#define Sn_IR_DISCON                 0x02
#define Sn_IR_CON                    0x01

/* PHYCFGR register value */
#define PHYCFGR_RST                  ~(1<<7)  //< For PHY reset, must operate AND mask.
#define PHYCFGR_OPMD                 (1<<6)   // Configre PHY with OPMDC value
#define PHYCFGR_OPMDC_ALLA           (7<<3)
#define PHYCFGR_OPMDC_PDOWN          (6<<3)
#define PHYCFGR_OPMDC_NA             (5<<3)
#define PHYCFGR_OPMDC_100FA          (4<<3)
#define PHYCFGR_OPMDC_100F           (3<<3)
#define PHYCFGR_OPMDC_100H           (2<<3)
#define PHYCFGR_OPMDC_10F            (1<<3)
#define PHYCFGR_OPMDC_10H            (0<<3)           
#define PHYCFGR_DPX_FULL             (1<<2)
#define PHYCFGR_DPX_HALF             (0<<2)
#define PHYCFGR_SPD_100              (1<<1)
#define PHYCFGR_SPD_10               (0<<1)
#define PHYCFGR_LNK_ON               (1<<0)
#define PHYCFGR_LNK_OFF              (0<<0)

//PHY status define
#define PHY_CONFBY_HW            0     ///< Configured PHY operation mode by HW pin
#define PHY_CONFBY_SW            1     ///< Configured PHY operation mode by SW register   
#define PHY_MODE_MANUAL          0     ///< Configured PHY operation mode with user setting.
#define PHY_MODE_AUTONEGO        1     ///< Configured PHY operation mode with auto-negotiation
#define PHY_SPEED_10             0     ///< Link Speed 10
#define PHY_SPEED_100            1     ///< Link Speed 100
#define PHY_DUPLEX_HALF          0     ///< Link Half-Duplex
#define PHY_DUPLEX_FULL          1     ///< Link Full-Duplex
#define PHY_LINK_OFF             0     ///< Link Off
#define PHY_LINK_ON              1     ///< Link On
#define PHY_POWER_NORM           0     ///< PHY power normal mode
#define PHY_POWER_DOWN           1     ///< PHY power down mode 

enum PHYMode {
    AutoNegotiate = 0,
    HalfDuplex10  = 1,
    FullDuplex10  = 2,
    HalfDuplex100 = 3,
    FullDuplex100 = 4,
};
class WIZnet_Chip {
public:
enum Protocol {
    CLOSED = 0,
    TCP    = 1,
    UDP    = 2,
};

enum Command {
    OPEN      = 0x01,
    LISTEN    = 0x02,
    CONNECT   = 0x04,
    DISCON    = 0x08,
    CLOSE     = 0x10,
    SEND      = 0x20,
    SEND_MAC  = 0x21, 
    SEND_KEEP = 0x22,
    RECV      = 0x40,
    
};  

enum Interrupt {
    INT_CON     = 0x01,
    INT_DISCON  = 0x02,
    INT_RECV    = 0x04,
    INT_TIMEOUT = 0x08,
    INT_SEND_OK = 0x10,
};

enum Status {
    SOCK_CLOSED      = 0x00,
    SOCK_INIT        = 0x13,
    SOCK_LISTEN      = 0x14,
    SOCK_SYNSENT     = 0x15,
    SOCK_ESTABLISHED = 0x17,
    SOCK_CLOSE_WAIT  = 0x1c,
    SOCK_UDP         = 0x22,
};

    
    uint16_t sock_any_port;
     
    /*
    * Constructor
    *
    * @param spi spi class
    * @param cs cs of the W5500
    * @param reset reset pin of the W5500
    */
    WIZnet_Chip(PinName mosi, PinName miso, PinName sclk, PinName cs, PinName reset);
    WIZnet_Chip(SPI* spi, PinName cs, PinName reset);

    /*
    * Set MAC Address to W5500
    *
    * @return true if connected, false otherwise
    */ 
    bool setmac();

    /*
    * Set Network Informations (SrcIP, Netmask, Gataway)
    *
    * @return true if connected, false otherwise
    */ 
    bool setip();

    /*
    * Disconnect the connection
    *
    * @ returns true 
    */
    bool disconnect();

    /*
    * Open a tcp connection with the specified host on the specified port
    *
    * @param host host (can be either an ip address or a name. If a name is provided, a dns request will be established)
    * @param port port
    * @ returns true if successful
    */
    bool connect(int socket, const char * host, int port, int timeout_ms = 10*1000);

    /*
    * Set the protocol (UDP or TCP)
    *
    * @param p protocol
    * @ returns true if successful
    */
    bool setProtocol(int socket, Protocol p);

    /*
    * Reset the W5500
    */
    void reset();
   
    int wait_readable(int socket, int wait_time_ms, int req_size = 0);

    int wait_writeable(int socket, int wait_time_ms, int req_size = 0);

    /*
    * Check if an ethernet link is pressent or not.
    *
    * @returns true if successful
    */
    bool link(int wait_time_ms= 3*1000);

    /*
    * Sets the speed and duplex parameters of an ethernet link.
    *
    * @returns true if successful
    */
    void set_link(PHYMode phymode);

    /*
    * Check if a tcp link is active
    *
    * @returns true if successful
    */
    bool is_connected(int socket);

    /*
    * Close a tcp connection
    *
    * @ returns true if successful
    */
    bool close(int socket);

    /*
    * @param str string to be sent
    * @param len string length
    */
    int send(int socket, const char * str, int len);

    int recv(int socket, char* buf, int len);

    /*
    * Return true if the module is using dhcp
    *
    * @returns true if the module is using dhcp
    */
    bool isDHCP() {
        return dhcp;
    }

    bool gethostbyname(const char* host, uint32_t* ip);

    static WIZnet_Chip * getInstance() {
        return inst;
    };

    int new_socket();
    uint16_t new_port();
    void scmd(int socket, Command cmd);

    template<typename T>
    void sreg(int socket, uint16_t addr, T data) {
        reg_wr<T>(addr, (0x0C + (socket << 5)), data);
    }

    template<typename T>
    T sreg(int socket, uint16_t addr) {
        return reg_rd<T>(addr, (0x08 + (socket << 5)));
    }

    template<typename T>
    void reg_wr(uint16_t addr, T data) {
        return reg_wr(addr, 0x04, data);
    }
    
    template<typename T>
    void reg_wr(uint16_t addr, uint8_t cb, T data) {
        uint8_t buf[sizeof(T)];
        *reinterpret_cast<T*>(buf) = data;
        for(int i = 0; i < sizeof(buf)/2; i++) { //  Little Endian to Big Endian
            uint8_t t = buf[i];
            buf[i] = buf[sizeof(buf)-1-i];
            buf[sizeof(buf)-1-i] = t;
        }
        spi_write(addr, cb, buf, sizeof(buf));
    }

    template<typename T>
    T reg_rd(uint16_t addr) {
        return reg_rd<T>(addr, 0x00);
    }

    template<typename T>
    T reg_rd(uint16_t addr, uint8_t cb) {
        uint8_t buf[sizeof(T)];
        spi_read(addr, cb, buf, sizeof(buf));
        for(int i = 0; i < sizeof(buf)/2; i++) { // Big Endian to Little Endian
            uint8_t t = buf[i];
            buf[i] = buf[sizeof(buf)-1-i];
            buf[sizeof(buf)-1-i] = t;
        }
        return *reinterpret_cast<T*>(buf);
    }

    void reg_rd_mac(uint16_t addr, uint8_t* data);
/*     {
        spi_read(addr, 0x00, data, 6);
    }*/

    void reg_wr_ip(uint16_t addr, uint8_t cb, const char* ip);
    /* {
        uint8_t buf[4];
        char* p = (char*)ip;
        for(int i = 0; i < 4; i++) {
            buf[i] = atoi(p);
            p = strchr(p, '.');
            if (p == NULL) {
                break;
            }
            p++;
        }
        spi_write(addr, cb, buf, sizeof(buf));
    }
    */
    void sreg_ip(int socket, uint16_t addr, const char* ip);
/*     {
        reg_wr_ip(addr, (0x0C + (socket << 5)), ip);
    }*/
    
    void reg_rd_ip_byte(uint16_t addr, uint8_t* data);
/*     {
        spi_read(addr, 0x00, data, 4);
    }*/
    
    void reg_wr_ip_byte(uint16_t addr, uint8_t* data);
/*     {
        spi_write(addr, 0x04, data, 4);
    }*/
       
/////////////////////////////////
// Common Register I/O function //
/////////////////////////////////
/**
 * @ingroup Common_register_access_function
 * @brief Set Mode Register
 * @param (uint8_t)mr The value to be set.
 * @sa getMR()
 */
    void setMR(uint8_t mr) {
        reg_wr<uint8_t>(MR,mr);
    }


/**
 * @ingroup Common_register_access_function
 * @brief Get Mode Register
 * @return uint8_t. The value of Mode register.
 * @sa setMR()
 */
    uint8_t getMR() {
        return reg_rd<uint8_t>(MR);
    }

/**
 * @ingroup Common_register_access_function
 * @brief Set gateway IP address
 * @param (uint8_t*)gar Pointer variable to set gateway IP address. It should be allocated 4 bytes.
 * @sa getGAR()
 */
    void setGAR(uint8_t * gar) {
        reg_wr_ip_byte(GAR,gar);
    }

/**
 * @ingroup Common_register_access_function
 * @brief Get gateway IP address
 * @param (uint8_t*)gar Pointer variable to get gateway IP address. It should be allocated 4 bytes.
 * @sa setGAR()
 */
    void getGAR(uint8_t * gar) { 
        reg_rd_ip_byte(GAR,gar);
    }

/**
 * @ingroup Common_register_access_function
 * @brief Set subnet mask address
 * @param (uint8_t*)subr Pointer variable to set subnet mask address. It should be allocated 4 bytes.
 * @sa getSUBR()
 */
    void setSUBR(uint8_t * subr) {
        reg_wr_ip_byte(SUBR, subr);
    }


/**
 * @ingroup Common_register_access_function
 * @brief Get subnet mask address
 * @param (uint8_t*)subr Pointer variable to get subnet mask address. It should be allocated 4 bytes.
 * @sa setSUBR()
 */
    void getSUBR(uint8_t * subr) {
        reg_rd_ip_byte(SUBR, subr);
    }

/**
 * @ingroup Common_register_access_function
 * @brief Set local MAC address
 * @param (uint8_t*)shar Pointer variable to set local MAC address. It should be allocated 6 bytes.
 * @sa getSHAR()
 */
    void setSHAR(uint8_t * shar) {
        reg_wr_mac(SHAR, shar);
    }

/**
 * @ingroup Common_register_access_function
 * @brief Get local MAC address
 * @param (uint8_t*)shar Pointer variable to get local MAC address. It should be allocated 6 bytes.
 * @sa setSHAR()
 */
    void getSHAR(uint8_t * shar) {
        reg_rd_mac(SHAR, shar);
    }

/**
 * @ingroup Common_register_access_function
 * @brief Set local IP address
 * @param (uint8_t*)sipr Pointer variable to set local IP address. It should be allocated 4 bytes.
 * @sa getSIPR()
 */
    void setSIPR(uint8_t * sipr) {
        reg_wr_ip_byte(SIPR, sipr);
    }

/**
 * @ingroup Common_register_access_function
 * @brief Get local IP address
 * @param (uint8_t*)sipr Pointer variable to get local IP address. It should be allocated 4 bytes.
 * @sa setSIPR()
 */
    void getSIPR(uint8_t * sipr) {
        reg_rd_ip_byte(SIPR, sipr);
    }

/**
 * @ingroup Common_register_access_function
 * @brief Set @ref IR register
 * @param (uint8_t)ir Value to set @ref IR register.
 * @sa getIR()
 */
    void setIR(uint8_t ir) { 
        reg_wr<uint8_t>(IR, (ir & 0xF0));
    }

/**
 * @ingroup Common_register_access_function
 * @brief Get @ref IR register
 * @return uint8_t. Value of @ref IR register.
 * @sa setIR()
 */
    uint8_t getIR() {
       return reg_rd<uint8_t>(IR & 0xF0);
    }
    
/**
 * @ingroup Common_register_access_function
 * @brief Set @ref IMR register
 * @param (uint8_t)imr Value to set @ref IMR register.
 * @sa getIMR()
 */
    void setIMR(uint8_t imr) {
        reg_wr<uint8_t>(IMR, imr);
    }

/**
 * @ingroup Common_register_access_function
 * @brief Get @ref IMR register
 * @return uint8_t. Value of @ref IMR register.
 * @sa setIMR()
 */
    uint8_t getIMR() {
        return reg_rd<uint8_t>(IMR);
    }


/**
 * @ingroup Common_register_access_function
 * @brief Set @ref SIR register
 * @param (uint8_t)sir Value to set @ref SIR register.
 * @sa getSIR()
 */
    void setSIR(uint8_t sir) {
        reg_wr<uint8_t>(SIR, sir);
    }

/**
 * @ingroup Common_register_access_function
 * @brief Get @ref SIR register
 * @return uint8_t. Value of @ref SIR register.
 * @sa setSIR()
 */
    uint8_t getSIR() {
        return reg_rd<uint8_t>(SIR);
    }
/**
 * @ingroup Common_register_access_function
 * @brief Set @ref SIMR register
 * @param (uint8_t)simr Value to set @ref SIMR register.
 * @sa getSIMR()
 */
    void setSIMR(uint8_t simr) {
        reg_wr<uint8_t>(SIMR, simr);
    }

/**
 * @ingroup Common_register_access_function
 * @brief Get @ref SIMR register
 * @return uint8_t. Value of @ref SIMR register.
 * @sa setSIMR()
 */
    uint8_t getSIMR() {
        return reg_rd<uint8_t>(SIMR);
    }

/**
 * @ingroup Common_register_access_function
 * @brief Set @ref RTR register
 * @param (uint16_t)rtr Value to set @ref RTR register.
 * @sa getRTR()
 */
    void setRTR(uint16_t rtr)   {
        reg_wr<uint16_t>(RTR, rtr);
    } 

/**
 * @ingroup Common_register_access_function
 * @brief Get @ref RTR register
 * @return uint16_t. Value of @ref RTR register.
 * @sa setRTR()
 */
    uint16_t getRTR() {
        return reg_rd<uint16_t>(RTR);
    }

/**
 * @ingroup Common_register_access_function
 * @brief Set @ref RCR register
 * @param (uint8_t)rcr Value to set @ref RCR register.
 * @sa getRCR()
 */
    void setRCR(uint8_t rcr) {
        reg_wr<uint8_t>(RCR, rcr);
    }

/**
 * @ingroup Common_register_access_function
 * @brief Get @ref RCR register
 * @return uint8_t. Value of @ref RCR register.
 * @sa setRCR()
 */
    uint8_t getRCR() {
        return reg_rd<uint8_t>(RCR);
    }

//================================================== test done ===========================================================

/**
 * @ingroup Common_register_access_function
 * @brief Set @ref PHYCFGR register
 * @param (uint8_t)phycfgr Value to set @ref PHYCFGR register.
 * @sa setPHYCFGR()
 */
   void setPHYCFGR(uint8_t phycfgr) {
        reg_wr<uint8_t>(PHYCFGR, phycfgr);
   }

/**
 * @ingroup Common_register_access_function
 * @brief Get @ref PHYCFGR register
 * @return uint8_t. Value of @ref PHYCFGR register.
 * @sa getPHYCFGR()
 */
     uint8_t getPHYCFGR() {
        return reg_rd<uint8_t>(PHYCFGR);
    }


/////////////////////////////////////

///////////////////////////////////
// Socket N register I/O function //
///////////////////////////////////
/**
 * @ingroup Socket_register_access_function
 * @brief Set @ref Sn_MR register
 * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
 * @param (uint8_t)mr Value to set @ref Sn_MR
 * @sa getSn_MR()
 */
    void setSn_MR(uint8_t sn, uint8_t mr) {
        sreg<uint8_t>(sn, MR, mr);
    }

/**
 * @ingroup Socket_register_access_function
 * @brief Get @ref Sn_MR register
 * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
 * @return uint8_t. Value of @ref Sn_MR.
 * @sa setSn_MR()
 */
    uint8_t getSn_MR(uint8_t sn) {
        return sreg<uint8_t>(sn, Sn_MR);
    }

/**
 * @ingroup Socket_register_access_function
 * @brief Set @ref Sn_CR register
 * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
 * @param (uint8_t)cr Value to set @ref Sn_CR
 * @sa getSn_CR()
 */
    void setSn_CR(uint8_t sn, uint8_t cr) {
        sreg<uint8_t>(sn, Sn_CR, cr);
    }

/**
 * @ingroup Socket_register_access_function
 * @brief Get @ref Sn_CR register
 * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
 * @return uint8_t. Value of @ref Sn_CR.
 * @sa setSn_CR()
 */
    uint8_t getSn_CR(uint8_t sn) {
        return sreg<uint8_t>(sn, Sn_CR);
    }

/**
 * @ingroup Socket_register_access_function
 * @brief Set @ref Sn_IR register
 * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
 * @param (uint8_t)ir Value to set @ref Sn_IR
 * @sa getSn_IR()
 */
    void setSn_IR(uint8_t sn, uint8_t ir) { 
        sreg<uint8_t>(sn, Sn_IR, (ir & 0x1F));
    }

/**
 * @ingroup Socket_register_access_function
 * @brief Get @ref Sn_IR register
 * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
 * @return uint8_t. Value of @ref Sn_IR.
 * @sa setSn_IR()
 */
    uint8_t getSn_IR(uint8_t sn) {
        return (sreg<uint8_t>(sn, Sn_IR)) & 0x1F;
    }

/**
 * @ingroup Socket_register_access_function
 * @brief Set @ref Sn_IMR register
 * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
 * @param (uint8_t)imr Value to set @ref Sn_IMR
 * @sa getSn_IMR()
 */
    void setSn_IMR(uint8_t sn, uint8_t imr) {
        sreg<uint8_t>(sn, Sn_IMR, (imr & 0x1F));
    }

/**
 * @ingroup Socket_register_access_function
 * @brief Get @ref Sn_IMR register
 * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
 * @return uint8_t. Value of @ref Sn_IMR.
 * @sa setSn_IMR()
 */
    uint8_t getSn_IMR(uint8_t sn) {
        return (sreg<uint8_t>(sn, Sn_IMR)) & 0x1F;
    }

/**
 * @ingroup Socket_register_access_function
 * @brief Get @ref Sn_SR register
 * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
 * @return uint8_t. Value of @ref Sn_SR.
 */
    uint8_t getSn_SR(uint8_t sn) {
        return sreg<uint8_t>(sn, Sn_SR);
    }

/**
 * @ingroup Socket_register_access_function
 * @brief Set @ref Sn_PORT register
 * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
 * @param (uint16_t)port Value to set @ref Sn_PORT.
 * @sa getSn_PORT()
 */
    void setSn_PORT(uint8_t sn, uint16_t port)  { 
        sreg<uint16_t>(sn, Sn_PORT, port );
    } 

/**
 * @ingroup Socket_register_access_function
 * @brief Get @ref Sn_PORT register
 * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
 * @return uint16_t. Value of @ref Sn_PORT.
 * @sa setSn_PORT()
 */
    uint16_t getSn_PORT(uint8_t sn) {
        return sreg<uint16_t>(sn, Sn_PORT);
    }

/**
 * @ingroup Socket_register_access_function
 * @brief Set @ref Sn_DHAR register
 * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
 * @param (uint8_t*)dhar Pointer variable to set socket n destination hardware address. It should be allocated 6 bytes.
 * @sa getSn_DHAR()
 */
    void setSn_DHAR(uint8_t sn, uint8_t * dhar) {
        spi_write(Sn_DHAR, (0x0C + (sn << 5)), dhar, 6);
    }

/**
 * @ingroup Socket_register_access_function
 * @brief Get @ref Sn_MR register
 * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
 * @param (uint8_t*)dhar Pointer variable to get socket n destination hardware address. It should be allocated 6 bytes.
 * @sa setSn_DHAR()
 */
    void getSn_DHAR(uint8_t sn, uint8_t * dhar) {
        spi_read(Sn_DHAR, (0x08 + (sn << 5)), dhar, 6);
    }

/**
 * @ingroup Socket_register_access_function
 * @brief Set @ref Sn_DIPR register
 * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
 * @param (uint8_t*)dipr Pointer variable to set socket n destination IP address. It should be allocated 4 bytes.
 * @sa getSn_DIPR()
 */
    void setSn_DIPR(uint8_t sn, uint8_t * dipr) {
        spi_write(Sn_DIPR, (0x0C + (sn << 5)), dipr, 4);
    }

/**
 * @ingroup Socket_register_access_function
 * @brief Get @ref Sn_DIPR register
 * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
 * @param (uint8_t*)dipr Pointer variable to get socket n destination IP address. It should be allocated 4 bytes.
 * @sa SetSn_DIPR()
 */
    void getSn_DIPR(uint8_t sn, uint8_t * dipr) {
        spi_read(Sn_DIPR, (0x08 + (sn << 5)), dipr, 4);
    }

/**
 * @ingroup Socket_register_access_function
 * @brief Set @ref Sn_DPORT register
 * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
 * @param (uint16_t)dport Value to set @ref Sn_DPORT
 * @sa getSn_DPORT()
 */
    void setSn_DPORT(uint8_t sn, uint16_t dport) { 
        sreg<uint16_t>(sn, Sn_DPORT, dport);
    } 

/**
 * @ingroup Socket_register_access_function
 * @brief Get @ref Sn_DPORT register
 * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
 * @return uint16_t. Value of @ref Sn_DPORT.
 * @sa setSn_DPORT()
 */
    uint16_t getSn_DPORT(uint8_t sn) {
        return sreg<uint16_t>(sn, Sn_DPORT);
    }


/**
 * @ingroup Socket_register_access_function
 * @brief Set @ref Sn_RXBUF_SIZE register
 * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
 * @param (uint8_t)rxbufsize Value to set @ref Sn_RXBUF_SIZE
 * @sa getSn_RXBUF_SIZE()
 */
    void setSn_RXBUF_SIZE(uint8_t sn, uint8_t rxbufsize) {
        sreg<uint8_t>(sn, Sn_RXBUF_SIZE ,rxbufsize);
    }


/**
 * @ingroup Socket_register_access_function
 * @brief Get @ref Sn_RXBUF_SIZE register
 * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
 * @return uint8_t. Value of @ref Sn_RXBUF_SIZE.
 * @sa setSn_RXBUF_SIZE()
 */
    uint8_t getSn_RXBUF_SIZE(uint8_t sn) {
        return sreg<uint8_t>(sn, Sn_RXBUF_SIZE);
    }

/**
 * @ingroup Socket_register_access_function
 * @brief Set @ref Sn_TXBUF_SIZE register
 * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
 * @param (uint8_t)txbufsize Value to set @ref Sn_TXBUF_SIZE
 * @sa getSn_TXBUF_SIZE()
 */
    void setSn_TXBUF_SIZE(uint8_t sn, uint8_t txbufsize) {
        sreg<uint8_t>(sn, Sn_TXBUF_SIZE, txbufsize);
    }

/**
 * @ingroup Socket_register_access_function
 * @brief Get @ref Sn_TXBUF_SIZE register
 * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
 * @return uint8_t. Value of @ref Sn_TXBUF_SIZE.
 * @sa setSn_TXBUF_SIZE()
 */
    uint8_t getSn_TXBUF_SIZE(uint8_t sn) {
        return sreg<uint8_t>(sn, Sn_TXBUF_SIZE);
    }

/**
 * @ingroup Socket_register_access_function
 * @brief Get @ref Sn_TX_FSR register
 * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
 * @return uint16_t. Value of @ref Sn_TX_FSR.
 */
    uint16_t getSn_TX_FSR(uint8_t sn) {
        return sreg<uint16_t>(sn, Sn_TX_FSR);
    }


/**
 * @ingroup Socket_register_access_function
 * @brief Get @ref Sn_TX_RD register
 * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
 * @return uint16_t. Value of @ref Sn_TX_RD.
 */
    uint16_t getSn_TX_RD(uint8_t sn) {
        return sreg<uint16_t>(sn, Sn_TX_RD);
    }

/**
 * @ingroup Socket_register_access_function
 * @brief Set @ref Sn_TX_WR register
 * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
 * @param (uint16_t)txwr Value to set @ref Sn_TX_WR
 * @sa GetSn_TX_WR()
 */
    void setSn_TX_WR(uint8_t sn, uint16_t txwr) { 
        sreg<uint16_t>(sn, Sn_TX_WR, txwr); 
    }

/**
 * @ingroup Socket_register_access_function
 * @brief Get @ref Sn_TX_WR register
 * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
 * @return uint16_t. Value of @ref Sn_TX_WR.
 * @sa setSn_TX_WR()
 */
    uint16_t getSn_TX_WR(uint8_t sn) {
        return sreg<uint16_t>(sn, Sn_TX_WR);
    }


/**
 * @ingroup Socket_register_access_function
 * @brief Get @ref Sn_RX_RSR register
 * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
 * @return uint16_t. Value of @ref Sn_RX_RSR.
 */
    uint16_t getSn_RX_RSR(uint8_t sn) {
        return sreg<uint16_t>(sn, Sn_RX_RSR);
    }


/**
 * @ingroup Socket_register_access_function
 * @brief Set @ref Sn_RX_RD register
 * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
 * @param (uint16_t)rxrd Value to set @ref Sn_RX_RD
 * @sa getSn_RX_RD()
 */
    void setSn_RX_RD(uint8_t sn, uint16_t rxrd) { 
        sreg<uint16_t>(sn, Sn_RX_RD, rxrd); 
    }

/**
 * @ingroup Socket_register_access_function
 * @brief Get @ref Sn_RX_RD register
 * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
 * @regurn uint16_t. Value of @ref Sn_RX_RD.
 * @sa setSn_RX_RD()
 */
    uint16_t getSn_RX_RD(uint8_t sn) {
        return sreg<uint16_t>(sn, Sn_RX_RD);
    }

/**
 * @ingroup Socket_register_access_function
 * @brief Get @ref Sn_RX_WR register
 * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
 * @return uint16_t. Value of @ref Sn_RX_WR.
 */
    uint16_t getSn_RX_WR(uint8_t sn) { 
        return sreg<uint16_t>(sn, Sn_RX_WR);
    }


//////////////////////////////////////

/////////////////////////////////////
// Sn_TXBUF & Sn_RXBUF IO function //
/////////////////////////////////////
/**  
 * @brief Gets the max buffer size of socket sn passed as parameter.
 * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
 * @return uint16_t. Value of Socket n RX max buffer size.
 */
    uint16_t getSn_RxMAX(uint8_t sn) {
        return (getSn_RXBUF_SIZE(sn) << 10);
    }

/**  
 * @brief Gets the max buffer size of socket sn passed as parameters.
 * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
 * @return uint16_t. Value of Socket n TX max buffer size.
 */
//uint16_t getSn_TxMAX(uint8_t sn);
    uint16_t getSn_TxMAX(uint8_t sn) {
        return (getSn_TXBUF_SIZE(sn) << 10);
    }


    int ethernet_link(void);
    void ethernet_set_link(int speed, int duplex);
protected:
    uint8_t mac[6];
    uint32_t ip;
    uint32_t netmask;
    uint32_t gateway;
    uint32_t dnsaddr;
    bool dhcp;
    
    void spi_write(uint16_t addr, uint8_t cb, const uint8_t *buf, uint16_t len);
    void spi_read(uint16_t addr, uint8_t cb, uint8_t *buf, uint16_t len);
    SPI* spi;
    DigitalOut cs;
    DigitalOut reset_pin;
    static WIZnet_Chip* inst;

    void reg_wr_mac(uint16_t addr, uint8_t* data) {
        spi_write(addr, 0x04, data, 6);
    }

};


extern uint32_t str_to_ip(const char* str);
extern void printfBytes(char* str, uint8_t* buf, int len);
extern void printHex(uint8_t* buf, int len);
extern void debug_hex(uint8_t* buf, int len);
