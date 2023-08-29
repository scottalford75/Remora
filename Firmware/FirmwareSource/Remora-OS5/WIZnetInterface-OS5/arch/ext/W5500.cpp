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
 */
#include "eth_arch.h"
#if (not defined TARGET_WIZwiki_W7500) && (not defined TARGET_WIZwiki_W7500P) && (not defined TARGET_WIZwiki_W7500ECO)



#include "mbed.h"
#include "mbed_debug.h"
#include "DNSClient.h"


//Debug is disabled by default
#if 0
#define DBG(...) do{debug("%p %d %s ", this,__LINE__,__PRETTY_FUNCTION__); debug(__VA_ARGS__); } while(0);
//#define DBG(x, ...) debug("[W5500:DBG]"x"\r\n", ##__VA_ARGS__);
#define WARN(x, ...) debug("[W5500:WARN]"x"\r\n", ##__VA_ARGS__);
#define ERR(x, ...) debug("[W5500:ERR]"x"\r\n", ##__VA_ARGS__);
#else
#define DBG(x, ...)
#define WARN(x, ...)
#define ERR(x, ...)
#endif

#if 1
#define INFO(x, ...) debug("[W5500:INFO]"x"\r\n", ##__VA_ARGS__);
#else
#define INFO(x, ...)
#endif

#define DBG_SPI 0

WIZnet_Chip* WIZnet_Chip::inst;

WIZnet_Chip::WIZnet_Chip(PinName mosi, PinName miso, PinName sclk, PinName _cs, PinName _reset):
    cs(_cs), reset_pin(_reset)
{
    spi = new SPI(mosi, miso, sclk);
    cs = 1;
    reset_pin = 1;
    inst = this;
    sock_any_port = SOCK_ANY_PORT_NUM;
}

WIZnet_Chip::WIZnet_Chip(SPI* spi, PinName _cs, PinName _reset):
    cs(_cs), reset_pin(_reset)
{
    this->spi = spi;
    cs = 1;
    reset_pin = 1;
    inst = this;
    sock_any_port = SOCK_ANY_PORT_NUM;
}

bool WIZnet_Chip::setmac()
{

    for (int i =0; i < 6; i++) reg_wr<uint8_t>(SHAR+i, mac[i]);

    return true;
}

// Set the IP
bool WIZnet_Chip::setip()
{
    reg_wr<uint32_t>(SIPR, ip);
    reg_wr<uint32_t>(GAR, gateway);
    reg_wr<uint32_t>(SUBR, netmask);
    return true;
}

bool WIZnet_Chip::setProtocol(int socket, Protocol p)
{
    if (socket < 0) {
        return false;
    }
    sreg<uint8_t>(socket, Sn_MR, p);
    return true;
}

bool WIZnet_Chip::connect(int socket, const char * host, int port, int timeout_ms)
{
    if (socket < 0) {
        return false;
    }
    sreg<uint8_t>(socket, Sn_MR, TCP);
    scmd(socket, OPEN);
    sreg_ip(socket, Sn_DIPR, host);
    sreg<uint16_t>(socket, Sn_DPORT, port);
    sreg<uint16_t>(socket, Sn_PORT, new_port());
    scmd(socket, CONNECT);
    Timer t;
    t.reset();
    t.start();
    while(!is_connected(socket)) {
        if (t.read_ms() > timeout_ms) {
            return false;
        }
    }
    return true;
}

bool WIZnet_Chip::gethostbyname(const char* host, uint32_t* ip)
{
    uint32_t addr = str_to_ip(host);
    char buf[17];
    snprintf(buf, sizeof(buf), "%d.%d.%d.%d", (addr>>24)&0xff, (addr>>16)&0xff, (addr>>8)&0xff, addr&0xff);
    if (strcmp(buf, host) == 0) {
        *ip = addr;
        return true;
    }
    DNSClient client;
    if(client.lookup(host)) {
        *ip = client.ip;
        return true;
    }
    return false;
}

bool WIZnet_Chip::disconnect()
{
    return true;
}

bool WIZnet_Chip::is_connected(int socket)
{
    /*
        if (sreg<uint8_t>(socket, Sn_SR) == SOCK_ESTABLISHED) {
            return true;
        }
    */
    uint8_t tmpSn_SR;
    tmpSn_SR = sreg<uint8_t>(socket, Sn_SR);
    // packet sending is possible, when state is SOCK_CLOSE_WAIT.
    if ((tmpSn_SR == SOCK_ESTABLISHED) || (tmpSn_SR == SOCK_CLOSE_WAIT)) {
        return true;
    }
    return false;
}

// Reset the chip & set the buffer
void WIZnet_Chip::reset()
{
#if defined(USE_WIZ550IO_MAC)
    //read the MAC address inside the module    
    reg_rd_mac(SHAR, mac); 
#endif
    // hw reset
    reset_pin = 1;
    reset_pin = 0;
    wait_us(500); // 500us (w5500)
    reset_pin = 1;
    wait_ms(400); // 400ms (w5500)
#if defined(USE_WIZ550IO_MAC)
    // write MAC address inside the WZTOE MAC address register
    reg_wr_mac(SHAR, mac);
#endif
    // set RX and TX buffer size
#if 0
    for (int socket = 0; socket < MAX_SOCK_NUM; socket++) {
        sreg<uint8_t>(socket, Sn_RXBUF_SIZE, 2);
        sreg<uint8_t>(socket, Sn_TXBUF_SIZE, 2);
    }
#endif
}


bool WIZnet_Chip::close(int socket)
{
    if (socket < 0) {
        return false;
    }
    // if not connected, return
    if (sreg<uint8_t>(socket, Sn_SR) == SOCK_CLOSED) {
        return true;
    }
    if (sreg<uint8_t>(socket, Sn_MR) == TCP) {
        scmd(socket, DISCON);
    }
    scmd(socket, CLOSE);
    sreg<uint8_t>(socket, Sn_IR, 0xff);
    return true;
}

int WIZnet_Chip::wait_readable(int socket, int wait_time_ms, int req_size)
{
    if (socket < 0) {
        return -1;
    }
    Timer t;
    t.reset();
    t.start();
    while(1) {
        //int size = sreg<uint16_t>(socket, Sn_RX_RSR);
        // during the reading Sn_RX_RXR, it has the possible change of this register.
        // so read twice and get same value then use size information.
        int size, size2;
        do {
            size = sreg<uint16_t>(socket, Sn_RX_RSR);
            size2 = sreg<uint16_t>(socket, Sn_RX_RSR);
        } while (size != size2);
        
        if (size > req_size) {
            return size;
        }
        break;
        //if (wait_time_ms != (-1) && t.read_ms() > wait_time_ms) {
        //    break;
        //}
    }
    return -1;
}

int WIZnet_Chip::wait_writeable(int socket, int wait_time_ms, int req_size)
{
    if (socket < 0) {
        return -1;
    }
    Timer t;
    t.reset();
    t.start();
    while(1) {
        //int size = sreg<uint16_t>(socket, Sn_TX_FSR);
        // during the reading Sn_TX_FSR, it has the possible change of this register.
        // so read twice and get same value then use size information.
        int size, size2;
        do {
            size = sreg<uint16_t>(socket, Sn_TX_FSR);
            size2 = sreg<uint16_t>(socket, Sn_TX_FSR);
        } while (size != size2);
        
        if (size > req_size) {
            return size;
        }
        break;
        //if (wait_time_ms != (-1) && t.read_ms() > wait_time_ms) {
        //    break;
        //}
    }
    return -1;
}

int WIZnet_Chip::send(int socket, const char * str, int len)
{
    if (socket < 0) {
        return -1;
    }
    uint16_t ptr = sreg<uint16_t>(socket, Sn_TX_WR);
    uint8_t cntl_byte = (0x14 + (socket << 5));
    spi_write(ptr, cntl_byte, (uint8_t*)str, len);
    sreg<uint16_t>(socket, Sn_TX_WR, ptr + len);
    scmd(socket, SEND);
    uint8_t tmp_Sn_IR;
    while (( (tmp_Sn_IR = sreg<uint8_t>(socket, Sn_IR)) & INT_SEND_OK) != INT_SEND_OK) {
        // @Jul.10, 2014 fix contant name, and udp sendto function.
        switch (sreg<uint8_t>(socket, Sn_SR)) {
            case SOCK_CLOSED :
                close(socket);
                return 0;
                //break;
            case SOCK_UDP :
                // ARP timeout is possible.
                if ((tmp_Sn_IR & INT_TIMEOUT) == INT_TIMEOUT) {
                    sreg<uint8_t>(socket, Sn_IR, INT_TIMEOUT);
                    return 0;
                }
                break;
            default :
                break;
        }
    }
    /*
        while ((sreg<uint8_t>(socket, Sn_IR) & INT_SEND_OK) != INT_SEND_OK) {
            if (sreg<uint8_t>(socket, Sn_SR) == CLOSED) {
                close(socket);
                return 0;
            }
        }
    */
    sreg<uint8_t>(socket, Sn_IR, INT_SEND_OK);

    return len;
}

int WIZnet_Chip::recv(int socket, char* buf, int len)
{
    if (socket < 0) {
        return -1;
    }
    uint16_t ptr = sreg<uint16_t>(socket, Sn_RX_RD);
    uint8_t cntl_byte = (0x18 + (socket << 5));
    spi_read(ptr, cntl_byte, (uint8_t*)buf, len);
    sreg<uint16_t>(socket, Sn_RX_RD, ptr + len);
    scmd(socket, RECV);
    return len;
}

int WIZnet_Chip::new_socket()
{
    for(int s = 0; s < MAX_SOCK_NUM; s++) {
        if (sreg<uint8_t>(s, Sn_SR) == SOCK_CLOSED) {
            return s;
        }
    }
    return -1;
}

uint16_t WIZnet_Chip::new_port()
{
    uint16_t port = rand();
    port |= 49152;
    return port;
}

bool WIZnet_Chip::link(int wait_time_ms)
{
    Timer t;
    t.reset();
    t.start();
    while(1) {
        int is_link = ethernet_link();
        
        if (is_link) {
            return true;
        }
        if (wait_time_ms != (-1) && t.read_ms() > wait_time_ms) {
            break;
        }
    }
    return 0;
}

void WIZnet_Chip::set_link(PHYMode phymode)
{
    int speed = -1;
    int duplex = 0;

    switch(phymode) {
        case AutoNegotiate : speed = -1; duplex = 0; break;
        case HalfDuplex10  : speed = 0;  duplex = 0; break;
        case FullDuplex10  : speed = 0;  duplex = 1; break;
        case HalfDuplex100 : speed = 1;  duplex = 0; break;
        case FullDuplex100 : speed = 1;  duplex = 1; break;
    }

    ethernet_set_link(speed, duplex);
}

void WIZnet_Chip::scmd(int socket, Command cmd)
{
    sreg<uint8_t>(socket, Sn_CR, cmd);
    while(sreg<uint8_t>(socket, Sn_CR));
}

void WIZnet_Chip::spi_write(uint16_t addr, uint8_t cb, const uint8_t *buf, uint16_t len)
{
    cs = 0;
    spi->write(addr >> 8);
    spi->write(addr & 0xff);
    spi->write(cb);
    for(int i = 0; i < len; i++) {
        spi->write(buf[i]);
    }
    cs = 1;

#if DBG_SPI 
    debug("[SPI]W %04x(%02x %d)", addr, cb, len);
    for(int i = 0; i < len; i++) {
        debug(" %02x", buf[i]);
        if (i > 16) {
            debug(" ...");
            break;
        }
    }
    debug("\r\n");
#endif
}

void WIZnet_Chip::spi_read(uint16_t addr, uint8_t cb, uint8_t *buf, uint16_t len)
{
    cs = 0;
    spi->write(addr >> 8);
    spi->write(addr & 0xff);
    spi->write(cb);
    for(int i = 0; i < len; i++) {
        buf[i] = spi->write(0);
    }
    cs = 1;

#if DBG_SPI
    debug("[SPI]R %04x(%02x %d)", addr, cb, len);
    for(int i = 0; i < len; i++) {
        debug(" %02x", buf[i]);
        if (i > 16) {
            debug(" ...");
            break;
        }
    }
    debug("\r\n");
    if ((addr&0xf0ff)==0x4026 || (addr&0xf0ff)==0x4003) {
        wait_ms(200);
    }
#endif
}

uint32_t str_to_ip(const char* str)
{
    uint32_t ip = 0;
    char* p = (char*)str;
    for(int i = 0; i < 4; i++) {
        ip |= atoi(p);
        p = strchr(p, '.');
        if (p == NULL) {
            break;
        }
        ip <<= 8;
        p++;
    }
    return ip;
}

void printfBytes(char* str, uint8_t* buf, int len)
{
    printf("%s %d:", str, len);
    for(int i = 0; i < len; i++) {
        printf(" %02x", buf[i]);
    }
    printf("\n");
}

void printHex(uint8_t* buf, int len)
{
    for(int i = 0; i < len; i++) {
        if ((i%16) == 0) {
            printf("%p", buf+i);
        }
        printf(" %02x", buf[i]);
        if ((i%16) == 15) {
            printf("\n");
        }
    }
    printf("\n");
}

void debug_hex(uint8_t* buf, int len)
{
    for(int i = 0; i < len; i++) {
        if ((i%16) == 0) {
            debug("%p", buf+i);
        }
        debug(" %02x", buf[i]);
        if ((i%16) == 15) {
            debug("\n");
        }
    }
    debug("\n");
}

int WIZnet_Chip::ethernet_link(void) {
    int val = getPHYCFGR();
    return (val&0x01);
}

void WIZnet_Chip::ethernet_set_link(int speed, int duplex) {
    uint32_t val=0;
    if((speed < 0) || (speed > 1)) {
        val = (PHYCFGR_OPMDC_ALLA)<<3; 
    } else {
        val = (((speed&0x01)<<1)+ (duplex&0x01))<<3; 
    }
    setPHYCFGR((uint8_t)(PHYCFGR_RST&(PHYCFGR_OPMD|val)));
    wait(0.2);
    setPHYCFGR((uint8_t)((~PHYCFGR_RST)|(PHYCFGR_OPMD|val)));
    wait(0.2);
}

    void WIZnet_Chip::reg_rd_mac(uint16_t addr, uint8_t* data) {
        spi_read(addr, 0x00, data, 6);
    }

    void WIZnet_Chip::reg_wr_ip(uint16_t addr, uint8_t cb, const char* ip) {
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
    
    void WIZnet_Chip::sreg_ip(int socket, uint16_t addr, const char* ip) {
        reg_wr_ip(addr, (0x0C + (socket << 5)), ip);
    }
    
    void WIZnet_Chip::reg_rd_ip_byte(uint16_t addr, uint8_t* data) {
        spi_read(addr, 0x00, data, 4);
    }
    
    void WIZnet_Chip::reg_wr_ip_byte(uint16_t addr, uint8_t* data) {
        spi_write(addr, 0x04, data, 4);
    }


#endif

