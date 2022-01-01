/* Copyright (C) 2012 mbed.org, MIT License
 *
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
#if defined(TARGET_WIZwiki_W7500) || defined(TARGET_WIZwiki_W7500P) || defined(TARGET_WIZwiki_W7500ECO)


#include "mbed.h"
#include "mbed_debug.h"
#include "DNSClient.h"


/*
 * MDIO via GPIO
 * mdio via gpio is supported and related functions as follows.
 *  - mdio_init(),mdio_read(),mdio_write()
 *  - input_MDIO(),output_MDIO(),turnaroud_MDIO(),idle_MDIO()
 * called by ethernet_link() and ethernet_set_link()
 */
 
#if defined (TARGET_WIZwiki_W7500) || defined(TARGET_WIZwiki_W7500ECO)

#define MDIO            GPIO_Pin_14
#define MDC             GPIO_Pin_15
#define GPIO_MDC        GPIOB
#define PHY_ADDR_IP101G 0x07 
#define PHY_ADDR        PHY_ADDR_IP101G
#define SVAL            0x2 //right shift val = 2 
#define PHYREG_CONTROL  0x0 //Control Register address (Contorl basic register)
#define PHYREG_STATUS   0x1 //Status Register address (Status basic register)
#define CNTL_DUPLEX     (0x01ul<< 7)
#define CNTL_AUTONEGO   (0x01ul<<11)
#define CNTL_SPEED      (0x01ul<<12)
#define MDC_WAIT        (1)

#elif defined (TARGET_WIZwiki_W7500P) 

#define MDIO            GPIO_Pin_15
#define MDC             GPIO_Pin_14
#define GPIO_MDC        GPIOB
#define PHY_ADDR_IP101G 0x01 
#define PHY_ADDR        PHY_ADDR_IP101G
#define SVAL            0x2 //right shift val = 2 
#define PHYREG_CONTROL  0x0 //Control Register address (Contorl basic register)
#define PHYREG_STATUS   0x1 //Status Register address (Status basic register)
#define CNTL_DUPLEX     (0x01ul<< 8)
#define CNTL_AUTONEGO   (0x01ul<<12)
#define CNTL_SPEED      (0x01ul<<13)
#define MDC_WAIT        (1)

#endif

void mdio_init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin_MDC, uint16_t GPIO_Pin_MDIO);
void mdio_write(GPIO_TypeDef* GPIOx, uint32_t PhyRegAddr, uint32_t val);
uint32_t mdio_read(GPIO_TypeDef* GPIOx, uint32_t PhyRegAddr);

WIZnet_Chip* WIZnet_Chip::inst;

WIZnet_Chip::WIZnet_Chip()
{
    inst = this;
}

bool WIZnet_Chip::setmac()
{
    reg_wr_mac(SHAR, mac);
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
    snprintf(buf, sizeof(buf), "%d.%d.%d.%d", 
            (uint8_t)((addr>>24)&0xff), 
            (uint8_t)((addr>>16)&0xff), 
            (uint8_t)((addr>>8)&0xff), 
            (uint8_t)(addr&0xff));
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
    /* S/W Reset PHY */
    mdio_write(GPIO_MDC, PHYREG_CONTROL, 0x8000);
    wait_ms(10);//for S/W reset
    wait_ms(10);//for MDC I/F RDY

    mdio_init(GPIO_MDC, MDC, MDIO);
    
    /* S/W Reset WZTOE */
    reg_wr<uint8_t>(MR, MR_RST);
    // set PAD strengh and pull-up for TXD[3:0] and TXE 
#ifdef __DEF_USED_IC101AG__ //For using IC+101AG

#if defined(TARGET_WIZwiki_W7500) || defined(TARGET_WIZwiki_W7500ECO)

    *(volatile uint32_t *)(0x41003068) = 0x64; //TXD0 
    *(volatile uint32_t *)(0x4100306C) = 0x64; //TXD1
    *(volatile uint32_t *)(0x41003070) = 0x64; //TXD2
    *(volatile uint32_t *)(0x41003074) = 0x64; //TXD3
    *(volatile uint32_t *)(0x41003050) = 0x64; //TXE
#endif

#endif  

    // set ticker counter
    reg_wr<uint32_t>(TIC100US, (SystemCoreClock/10000));
    // write MAC address inside the WZTOE MAC address register
    reg_wr_mac(SHAR, mac);
    /*
     * set RX and TX buffer size
     * for (int socket = 0; socket < MAX_SOCK_NUM; socket++) {
     *  sreg<uint8_t>(socket, Sn_RXBUF_SIZE, 2);
     *  sreg<uint8_t>(socket, Sn_TXBUF_SIZE, 2);
     * }
     */
}


bool WIZnet_Chip::close(int socket)
{
    if (socket < 0) {
        return false;
    }
    // if SOCK_CLOSED, return
    if (sreg<uint8_t>(socket, Sn_SR) == SOCK_CLOSED) {
        return true;
    }
    // if SOCK_ESTABLISHED, send FIN-Packet to peer 
    if (sreg<uint8_t>(socket, Sn_MR) == TCP) {
        scmd(socket, DISCON);
    }
    // close socket
    scmd(socket, CLOSE);
    // clear Socket Interrupt Register
    sreg<uint8_t>(socket, Sn_ICR, 0xff);
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
        int size = sreg<uint16_t>(socket, Sn_RX_RSR);
        if (size > req_size) {
            return size;
        }
        if (wait_time_ms != (-1) && t.read_ms() > wait_time_ms) {
            break;
        }
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
        int size = sreg<uint16_t>(socket, Sn_TX_FSR);
        if (size > req_size) {
            return size;
        }
        if (wait_time_ms != (-1) && t.read_ms() > wait_time_ms) {
            break;
        }
    }
    return -1;
}

int WIZnet_Chip::send(int socket, const char * str, int len)
{
    if (socket < 0) {
        return -1;
    }

    uint16_t ptr = sreg<uint16_t>(socket, Sn_TX_WR);
    uint32_t sn_tx_base = W7500x_TXMEM_BASE + (uint32_t)(socket<<18); 

    for(int i=0; i<len; i++)
        *(volatile uint8_t *)(sn_tx_base + ((ptr+i)&0xFFFF)) = str[i];

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
                    sreg<uint8_t>(socket, Sn_ICR, INT_TIMEOUT);
                    return 0;
                }
                break;
            default :
                break;
        }
    }

    sreg<uint8_t>(socket, Sn_ICR, INT_SEND_OK);

    return len;
}

int WIZnet_Chip::recv(int socket, char* buf, int len)
{
    if (socket < 0) {
        return -1;
    }
    uint16_t ptr = sreg<uint16_t>(socket, Sn_RX_RD);
    uint32_t sn_rx_base = W7500x_RXMEM_BASE + (uint32_t)(socket<<18); 

    for(int i=0; i<len; i++)
        buf[i] = *(volatile uint8_t *)(sn_rx_base + ((ptr+i)&0xFFFF));

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

void WIZnet_Chip::scmd(int socket, Command cmd)
{
    sreg<uint8_t>(socket, Sn_CR, cmd);
    while(sreg<uint8_t>(socket, Sn_CR));
}


void mdio_init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin_MDC, uint16_t GPIO_Pin_MDIO)
{
    /* Set GPIOs for MDIO and MDC */
    GPIO_InitTypeDef MDIO_InitDef;  
    HAL_PAD_AFConfig(PAD_PB, GPIO_Pin_MDIO, PAD_AF1);  
    HAL_PAD_AFConfig(PAD_PB, GPIO_Pin_MDC, PAD_AF1);  
    MDIO_InitDef.GPIO_Pin = GPIO_Pin_MDC | GPIO_Pin_MDIO;
    MDIO_InitDef.GPIO_Mode = GPIO_Mode_OUT;
    HAL_GPIO_Init(GPIOx, &MDIO_InitDef);
}

void output_MDIO(GPIO_TypeDef* GPIOx, uint32_t val, uint32_t n)
{
    for(val <<= (32-n); n; val<<=1, n--)
    {
        if(val & 0x80000000)
            HAL_GPIO_SetBits(GPIOx, MDIO); 
        else
            HAL_GPIO_ResetBits(GPIOx, MDIO);

        wait_ms(MDC_WAIT);
        HAL_GPIO_SetBits(GPIOx, MDC); 
        wait_ms(MDC_WAIT);
        HAL_GPIO_ResetBits(GPIOx, MDC);
    }
}

uint32_t input_MDIO( GPIO_TypeDef* GPIOx )
{
    uint32_t i, val=0; 
    for(i=0; i<16; i++)
    {
        val <<=1;
        HAL_GPIO_SetBits(GPIOx, MDC); 
        wait_ms(MDC_WAIT);
        HAL_GPIO_ResetBits(GPIOx, MDC);
        wait_ms(MDC_WAIT);
        val |= HAL_GPIO_ReadInputDataBit(GPIOx, MDIO);
    }
    return (val);
}

void turnaround_MDIO( GPIO_TypeDef* GPIOx)
{
    GPIOx->OUTENCLR = MDIO ;
    HAL_GPIO_SetBits(GPIOx, MDC); 
    wait_ms(MDC_WAIT);
    HAL_GPIO_ResetBits(GPIOx, MDC);
    wait_ms(MDC_WAIT);
}

void idle_MDIO( GPIO_TypeDef* GPIOx )
{
    GPIOx->OUTENSET = MDIO ;
    HAL_GPIO_SetBits(GPIOx,MDC); 
    wait_ms(MDC_WAIT);
    HAL_GPIO_ResetBits(GPIOx, MDC);
    wait_ms(MDC_WAIT);
}

uint32_t mdio_read(GPIO_TypeDef* GPIOx, uint32_t PhyRegAddr)
{
    output_MDIO(GPIOx, 0xFFFFFFFF, 32);
    output_MDIO(GPIOx, 0x06, 4);
    output_MDIO(GPIOx, PHY_ADDR, 5);
    output_MDIO(GPIOx, PhyRegAddr, 5);
    turnaround_MDIO(GPIOx);
    uint32_t val = input_MDIO(GPIOx );
    idle_MDIO(GPIOx);
    return val;
}

void mdio_write(GPIO_TypeDef* GPIOx, uint32_t PhyRegAddr, uint32_t val)
{
    output_MDIO(GPIOx, 0xFFFFFFFF, 32);
    output_MDIO(GPIOx, 0x05, 4);
    output_MDIO(GPIOx, PHY_ADDR, 5);
    output_MDIO(GPIOx, PhyRegAddr, 5);
    output_MDIO(GPIOx, 0x02, 2);
    output_MDIO(GPIOx, val, 16);
    idle_MDIO(GPIOx);
}

int WIZnet_Chip::ethernet_link(void) {
    return ((mdio_read(GPIO_MDC, PHYREG_STATUS)>>SVAL)&0x01); 
}

void WIZnet_Chip::ethernet_set_link(int speed, int duplex) {
    uint32_t val=0;
    if((speed < 0) || (speed > 1)) {
        val = CNTL_AUTONEGO; 
    } else {
        val = ((CNTL_SPEED&(speed<<11))|(CNTL_DUPLEX&(duplex<<7))); 
    }
    mdio_write(GPIO_MDC, PHYREG_CONTROL, val);
}

   void WIZnet_Chip::reg_rd_mac(uint16_t addr, uint8_t* data) 
   {
        data[0] = *(volatile uint8_t *)(W7500x_WZTOE_BASE + (uint32_t)(addr+3));
        data[1] = *(volatile uint8_t *)(W7500x_WZTOE_BASE + (uint32_t)(addr+2));
        data[2] = *(volatile uint8_t *)(W7500x_WZTOE_BASE + (uint32_t)(addr+1));
        data[3] = *(volatile uint8_t *)(W7500x_WZTOE_BASE + (uint32_t)(addr+0));
        data[4] = *(volatile uint8_t *)(W7500x_WZTOE_BASE + (uint32_t)(addr+7));
        data[5] = *(volatile uint8_t *)(W7500x_WZTOE_BASE + (uint32_t)(addr+6));
    }

    void WIZnet_Chip::reg_wr_ip(uint16_t addr, uint8_t cb, const char* ip)
    {
        uint8_t buf[4]={0,};
        uint32_t wr_ip = 0;
        char* p = (char*)ip;
        
        for(int i = 0; i < 4; i++) {
            wr_ip = (wr_ip<<8);
            buf[i] = atoi(p);
            wr_ip |= buf[i];
            p = strchr(p, '.');
            if (p == NULL) break;
            p++;
        }
        *(volatile uint32_t *)(W7500x_WZTOE_BASE + (uint32_t)((cb<<16)+addr)) = wr_ip;
    }

    void WIZnet_Chip::sreg_ip(int socket, uint16_t addr, const char* ip) {
        reg_wr_ip(addr,  (uint8_t)(0x01+(socket<<2)), ip);
    }

#endif


