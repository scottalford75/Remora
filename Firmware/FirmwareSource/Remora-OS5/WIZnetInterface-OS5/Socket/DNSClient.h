// DNSClient.h 2013/4/5
#pragma once

#include "WIZnet_UDPSocket.h"
 
class DNSClient {
public:
    DNSClient(const char* hostname = NULL);
    DNSClient(Endpoint* pHost);
    virtual ~DNSClient();
    bool lookup(const char* hostname = NULL);
    uint32_t ip;
protected:
    void poll();
    void callback();
    int response(uint8_t buf[], int size);
    int query(uint8_t buf[], int size, const char* hostname);
    void resolve(const char* hostname);
    uint8_t m_id[2];
    Timer m_interval;
    int m_retry;
    const char* m_hostname;
private:
    enum MyNetDnsState
    {
        MYNETDNS_START,
        MYNETDNS_PROCESSING, //Req has not completed
        MYNETDNS_NOTFOUND,
        MYNETDNS_ERROR,
        MYNETDNS_OK
    };
    MyNetDnsState m_state;
    WIZnet_UDPSocket *m_udp;
};
