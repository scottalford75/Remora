// DHCPClient.cpp 2013/4/10
#include "mbed.h"
#include "mbed_debug.h"
#include "WIZnet_UDPSocket.h"
#include "DHCPClient.h"

#define DBG_DHCP 0

#if DBG_DHCP
#define DBG(...) do{debug("[%s:%d]", __PRETTY_FUNCTION__,__LINE__);debug(__VA_ARGS__);} while(0);
#define DBG_HEX(A,B) do{debug("[%s:%d]\r\n", __PRETTY_FUNCTION__,__LINE__);debug_hex(A,B);} while(0);
#else
#define DBG(...) while(0);
#define DBG_HEX(A,B) while(0);
#endif

int DHCPClient::discover()
{
    m_pos = 0;
    const uint8_t header[] = {0x01,0x01,0x06,0x00};
    add_buf((uint8_t*)header, sizeof(header));
    uint32_t x = time(NULL) + rand();
    xid[0] = x>>24; xid[1] = x>>16; xid[2] = x>>8; xid[3] = x;
    add_buf(xid, 4);
    fill_buf(20, 0x00);
    add_buf(chaddr, 6);
    fill_buf(10+192, 0x00);
    const uint8_t options[] = {0x63,0x82,0x53,0x63, // magic cookie
                               53,1,DHCPDISCOVER,   // DHCP option 53: DHCP Discover
                               55,4,1,3,15,6,
                               255}; 
    add_buf((uint8_t*)options, sizeof(options));
    return m_pos;
}

int DHCPClient::request()
{
    m_pos = 0;
    const uint8_t header[] = {0x01,0x01,0x06,0x00};
    add_buf((uint8_t*)header, sizeof(header));
    add_buf(xid, 4);
    fill_buf(12, 0x00);
    add_buf(siaddr, 4);
    fill_buf(4, 0x00); // giaddr
    add_buf(chaddr, 6);
    fill_buf(10+192, 0x00);
    const uint8_t options[] = {0x63,0x82,0x53,0x63, // magic cookie
                               53,1,DHCPREQUEST,    // DHCP option 53: DHCP Request
                               55,4,1,3,15,6,       // DHCP option 55:
                               };
    add_buf((uint8_t*)options, sizeof(options));
    add_option(50, yiaddr, 4);
    add_option(54, siaddr, 4);
    add_option(255);
    return m_pos;
}

int DHCPClient::offer(uint8_t buf[], int size) {
    memcpy(yiaddr, buf+DHCP_OFFSET_YIADDR, 4);   
    memcpy(siaddr, buf+DHCP_OFFSET_SIADDR, 4);   
    uint8_t *p;
    int msg_type = -1;
    p = buf + DHCP_OFFSET_OPTIONS;
    while(*p != 255 && p < (buf+size)) {
        uint8_t code = *p++;
        if (code == 0) { // Pad Option
            continue;
        }
        int len = *p++;
 
        DBG("DHCP option: %d\r\n", code);
        DBG_HEX(p, len);

        switch(code) {
            case 53:
                msg_type = *p;
                break;
            case 1:
                memcpy(netmask, p, 4); // Subnet mask address
                break;
            case 3:
                memcpy(gateway, p, 4); // Gateway IP address
                break; 
            case 6:  // DNS server
                memcpy(dnsaddr, p, 4);
                break;
            case 51: // IP lease time 
                break;
            case 54: // DHCP server
                memcpy(siaddr, p, 4);
                break;
        }
        p += len;
    }
    return msg_type;
}

bool DHCPClient::verify(uint8_t buf[], int len) {
    if (len < DHCP_OFFSET_OPTIONS) {
        return false;
    }
    if (buf[DHCP_OFFSET_OP] != 0x02) {
        return false;
    }
    if (memcmp(buf+DHCP_OFFSET_XID, xid, 4) != 0) {
        return false;
    }
    return true;
}

void DHCPClient::callback()
{
    Endpoint host;
    int recv_len = m_udp->receiveFrom(host, (char*)m_buf, sizeof(m_buf));
    if (recv_len < 0) {
        return;
    }
    if (!verify(m_buf, recv_len)) {
        return;
    }
    int r = offer(m_buf, recv_len);
    if (r == DHCPOFFER) {
        int send_size = request();
        m_udp->sendTo(m_server, (char*)m_buf, send_size);
    } else if (r == DHCPACK) {
        exit_flag = true;
    }
}

void  DHCPClient::add_buf(uint8_t c)
{
    m_buf[m_pos++] = c;
}

void  DHCPClient::add_buf(uint8_t* buf, int len)
{
    for(int i = 0; i < len; i++) {
        add_buf(buf[i]);
    }
}

void DHCPClient::fill_buf(int len, uint8_t data)
{
    while(len-- > 0) {
        add_buf(data);
    }
}

void  DHCPClient::add_option(uint8_t code, uint8_t* buf, int len)
{
    add_buf(code);
    if (len > 0) {
        add_buf((uint8_t)len);
        add_buf(buf, len);
    }
}

int DHCPClient::setup(int timeout_ms)
{
    eth = WIZnet_Chip::getInstance();
    if (eth == NULL) {
        return -1;
    }    
    eth->reg_rd_mac(SHAR, chaddr);
    int interval_ms = 5*1000; // 5000msec
    if (timeout_ms < interval_ms) {
        interval_ms = timeout_ms;
    }
    m_udp = new WIZnet_UDPSocket;
    m_udp->init();
    m_udp->set_blocking(false);
    eth->reg_wr<uint32_t>(SIPR, 0x00000000); // local ip "0.0.0.0"
    m_udp->bind(68); // local port
    m_server.set_address("255.255.255.255", 67); // DHCP broadcast
    exit_flag = false;
    int err = 0;
    int seq = 0;
    int send_size;
    while(!exit_flag) {
        switch(seq) {
            case 0:
                m_retry = 0;
                seq++;
                break;
            case 1:
                send_size = discover();
                m_udp->sendTo(m_server, (char*)m_buf, send_size);
                m_interval.reset();
                m_interval.start();
                seq++;
                break;
            case 2:
                callback();
                if (m_interval.read_ms() > interval_ms) {
                    DBG("m_retry: %d\n", m_retry);
                    if (++m_retry >= (timeout_ms/interval_ms)) {
                        err = -1;
                        exit_flag = true;
                    }
                    seq--;
                }
                break;
        }
    }
    DBG("m_retry: %d, m_interval: %d\n", m_retry, m_interval.read_ms());
    delete m_udp;
    return err;
}

DHCPClient::DHCPClient() {
}
