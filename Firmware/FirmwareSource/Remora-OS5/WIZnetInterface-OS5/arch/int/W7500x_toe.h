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


#define MAX_SOCK_NUM 8

// Peripheral base address 
#define W7500x_WZTOE_BASE   (0x46000000)
#define W7500x_TXMEM_BASE   (W7500x_WZTOE_BASE + 0x00020000)
#define W7500x_RXMEM_BASE   (W7500x_WZTOE_BASE + 0x00030000)
// Common register
#define MR                  (0x2300)
#define GAR                 (0x6008)
#define SUBR                (0x600C)
#define SHAR                (0x6000)
#define SIPR                (0x6010)

// Added Common register @W7500
#define TIC100US            (0x2000) 

// Socket register
#define Sn_MR               (0x0000)
#define Sn_CR               (0x0010)
#define Sn_IR               (0x0020) //--Sn_ISR
#define Sn_SR               (0x0030)
#define Sn_PORT             (0x0114)
#define Sn_DIPR             (0x0124)
#define Sn_DPORT            (0x0120)
#define Sn_RXBUF_SIZE       (0x0200)
#define Sn_TXBUF_SIZE       (0x0220)
#define Sn_TX_FSR           (0x0204)
#define Sn_TX_WR            (0x020C)
#define Sn_RX_RSR           (0x0224)
#define Sn_RX_RD            (0x0228)
// added Socket register @W7500
#define Sn_ICR              (0x0028)
enum PHYMode {
    AutoNegotiate = 0,
    HalfDuplex10  = 1,
    FullDuplex10  = 2,
    HalfDuplex100 = 3,
    FullDuplex100 = 4,
};

//bool plink(int wait_time_ms= 3*1000);

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
enum Mode {
    MR_RST           = 0x80,   
    MR_WOL           = 0x20,   
    MR_PB            = 0x10,   
    MR_FARP          = 0x02,   
};

    WIZnet_Chip();

    /*
    * Set MAC Address to W7500x_TOE
    *
    * @return true if connected, false otherwise
    */ 
    bool setmac();

    /*
    * Connect the W7500 WZTOE to the ssid contained in the constructor.
    *
    * @return true if connected, false otherwise
    */ 
    bool setip();


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
    * Reset the W7500 WZTOE
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
        reg_wr<T>(addr, (uint8_t)(0x01+(socket<<2)), data);
    }

    template<typename T>
    T sreg(int socket, uint16_t addr) {
        return reg_rd<T>(addr, (uint8_t)(0x01+(socket<<2)));
    }

    template<typename T>
    void reg_wr(uint16_t addr, T data) {
        return reg_wr(addr, 0x00, data);
    }
    
    template<typename T>
    void reg_wr(uint16_t addr, uint8_t cb, T data) {
        uint8_t buf[sizeof(T)];
        *reinterpret_cast<T*>(buf) = data;
        /*
        for(int i = 0; i < sizeof(buf)/2; i++) { //  Little Endian to Big Endian
            uint8_t t = buf[i];
            buf[i] = buf[sizeof(buf)-1-i];
            buf[sizeof(buf)-1-i] = t;
        }
        */
        for(int i = 0; i < sizeof(buf); i++) { //  Little Endian to Big Endian
            *(volatile uint8_t *)(W7500x_WZTOE_BASE + (uint32_t)((cb<<16)+addr)+i) = buf[i];
        }
    }

    template<typename T>
    T reg_rd(uint16_t addr) {
        return reg_rd<T>(addr, (uint8_t)(0x00));
    }

    template<typename T>
    T reg_rd(uint16_t addr, uint8_t cb) {
        uint8_t buf[sizeof(T)] = {0,};
        for(int i = 0; i < sizeof(buf); i++) { //  Little Endian to Big Endian
            buf[i] = *(volatile uint8_t *)(W7500x_WZTOE_BASE + (uint32_t)((cb<<16)+addr)+i);
        }
        /*
        for(int i = 0; i < sizeof(buf)/2; i++) { // Big Endian to Little Endian
            uint8_t t = buf[i];
            buf[i] = buf[sizeof(buf)-1-i];
            buf[sizeof(buf)-1-i] = t;
        }
        */
        return *reinterpret_cast<T*>(buf);
    }

    void reg_rd_mac(uint16_t addr, uint8_t* data);
    
    void reg_wr_ip(uint16_t addr, uint8_t cb, const char* ip);

    void sreg_ip(int socket, uint16_t addr, const char* ip);

    int ethernet_link(void);
    
    void ethernet_set_link(int speed, int duplex);


protected:
    uint8_t mac[6];
    uint32_t ip;
    uint32_t netmask;
    uint32_t gateway;
    uint32_t dnsaddr;
    bool dhcp;

    static WIZnet_Chip* inst;

    void reg_wr_mac(uint16_t addr, uint8_t* data) {
        *(volatile uint8_t *)(W7500x_WZTOE_BASE + (uint32_t)(addr+3)) = data[0] ;
        *(volatile uint8_t *)(W7500x_WZTOE_BASE + (uint32_t)(addr+2)) = data[1] ;
        *(volatile uint8_t *)(W7500x_WZTOE_BASE + (uint32_t)(addr+1)) = data[2] ;
        *(volatile uint8_t *)(W7500x_WZTOE_BASE + (uint32_t)(addr+0)) = data[3] ;
        *(volatile uint8_t *)(W7500x_WZTOE_BASE + (uint32_t)(addr+7)) = data[4] ;
        *(volatile uint8_t *)(W7500x_WZTOE_BASE + (uint32_t)(addr+6)) = data[5] ;
    }
};

extern uint32_t str_to_ip(const char* str);
extern void printfBytes(char* str, uint8_t* buf, int len);
extern void printHex(uint8_t* buf, int len);
extern void debug_hex(uint8_t* buf, int len);
