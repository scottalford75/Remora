// DHCPClient.h 2013/4/10
#ifndef DHCPCLIENT_H
#define DHCPCLIENT_H
#include "eth_arch.h"
#include "WIZnet_UDPSocket.h"

#define DHCP_OFFSET_OP 0
#define DHCP_OFFSET_XID 4
#define DHCP_OFFSET_YIADDR 16
#define DHCP_OFFSET_SIADDR 20
#define DHCP_OFFSET_OPTIONS 240
#define DHCP_MAX_PACKET_SIZE 600

// DHCP Message Type
#define DHCPDISCOVER 1
#define DHCPOFFER    2
#define DHCPREQUEST  3
#define DHCPDECLINE  4
#define DHCPACK      5
#define DHCPNAK      6
#define DHCPRELEASE  7
#define DHCPINFORM   8

class DHCPClient {
public:
    DHCPClient();
    int setup(int timeout_ms = 15*1000);
    uint8_t chaddr[6]; // MAC
    uint8_t yiaddr[4]; // IP
    uint8_t dnsaddr[4]; // DNS
    uint8_t gateway[4];
    uint8_t netmask[4];
    uint8_t siaddr[4];
private:
    int discover();
    int request();
    int offer(uint8_t buf[], int size);
    void add_buf(uint8_t* buf, int len);
    void fill_buf(int len, uint8_t data = 0x00);
    void add_buf(uint8_t c);
    void add_option(uint8_t code, uint8_t* buf = NULL, int len = 0);
    bool verify(uint8_t buf[], int len);
    void callback();
    WIZnet_UDPSocket* m_udp;
    Endpoint m_server;
    uint8_t xid[4];
    bool exit_flag;
    Timer m_interval;
    int m_retry;
    uint8_t m_buf[DHCP_MAX_PACKET_SIZE];
    int m_pos;
    WIZnet_Chip* eth;
};
#endif //DHCPCLIENT_H
