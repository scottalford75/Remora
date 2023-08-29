// DNSClient.cpp 2013/8/27
#include "mbed.h"
#include "mbed_debug.h"
#include "DNSClient.h"
#include "WIZnet_UDPSocket.h"
#include "dnsname.h"
#include "eth_arch.h"

#define DBG_DNS 0

#if DBG_DNS
#define DBG2(...) do{debug("[DNS]%p %d %s ", this,__LINE__,__PRETTY_FUNCTION__); debug(__VA_ARGS__); } while(0);
#else
#define DBG2(...) while(0);
#endif

DNSClient::DNSClient(const char* hostname) : m_state(MYNETDNS_START), m_udp(NULL) {
    m_hostname = hostname;
}

DNSClient::DNSClient(Endpoint* pHost) : m_state(MYNETDNS_START), m_udp(NULL) {
}

DNSClient::~DNSClient() {
    if (m_udp) {
        delete m_udp;
    }
}

void DNSClient::callback()
{
    uint8_t buf[512];
    Endpoint host;
    int len = m_udp->receiveFrom(host, (char*)buf, sizeof(buf));
    if (len < 0) {
        return;
    }
    if (memcmp(buf+0, m_id, 2) != 0) { //verify
        return;
    }
    int rcode = response(buf, len);
    if (rcode == 0) {
        m_state = MYNETDNS_OK;
    } else {
        m_state = MYNETDNS_NOTFOUND;
    }
}

int DNSClient::response(uint8_t buf[], int size) {
    int rcode = buf[3] & 0x0f;
    if (rcode != 0) {
        return rcode;
    }
    int qdcount = buf[4]<<8|buf[5];
    int ancount = buf[6]<<8|buf[7];
    int pos = 12;
    while(qdcount-- > 0) {
        dnsname qname(buf);
        pos = qname.decode(pos); // qname
        pos += 4; // qtype qclass
    }
    while(ancount-- > 0) {
        dnsname name(buf);
        pos = name.decode(pos); // name
        int type = buf[pos]<<8|buf[pos+1];
        pos += 8; // type class TTL  
        int rdlength = buf[pos]<<8|buf[pos+1]; pos += 2;
        int rdata_pos = pos;
        pos += rdlength;
        if (type == 1) { // A record
            ip = (buf[rdata_pos]<<24) | (buf[rdata_pos+1]<<16) | (buf[rdata_pos+2]<<8) | buf[rdata_pos+3];
        }
#if DBG_DNS
        printf("%s", name.str.c_str());
        if (type == 1) {
            printf(" A %d.%d.%d.%d\n", 
                buf[rdata_pos],buf[rdata_pos+1],buf[rdata_pos+2],buf[rdata_pos+3]);
        } else if (type == 5) {
            dnsname rdname(buf);
            rdname.decode(rdata_pos);
            printf(" CNAME %s\n", rdname.str.c_str());
        } else {
            printf(" TYPE:%d", type);
            printfBytes(" RDATA:", &buf[rdata_pos], rdlength);
        }
#endif
    }
    return rcode;
}

int DNSClient::query(uint8_t buf[], int size, const char* hostname) {
    const uint8_t header[] = {
        0x00,0x00,0x01,0x00, // id=0x0000 QR=0 rd=1 opcode=0 rcode=0
        0x00,0x01,0x00,0x00, // qdcount=1 ancount=0
        0x00,0x00,0x00,0x00};// nscount=0 arcount=0 
    const uint8_t tail[] = {0x00,0x01,0x00,0x01}; // qtype=A qclass=IN
    memcpy(buf, header, sizeof(header));
    int t = rand();
    m_id[0] = t>>8;
    m_id[1] = t;
    memcpy(buf, m_id, 2); 
    dnsname qname(buf);
    int pos = qname.encode(sizeof(header), (char*)hostname);
    memcpy(buf+pos, tail, sizeof(tail));
    pos += sizeof(tail);
    return pos;
}

void DNSClient::resolve(const char* hostname) {
    if (m_udp == NULL) {
        m_udp = new WIZnet_UDPSocket;
    }
    m_udp->init();
    m_udp->set_blocking(false);
    Endpoint server;
    server.set_address("8.8.8.8", 53); // DNS
    m_udp->bind(rand()&0x7fff);
    uint8_t buf[256];                
    int size = query(buf, sizeof(buf), hostname);
#if DBG_DNS
    printf("hostname:[%s]\n", hostname);
    printHex(buf, size);
#endif
    m_udp->sendTo(server, (char*)buf, size);
    m_interval.reset();
    m_interval.start();
}

void DNSClient::poll() {
#if DBG_DNS
    printf("%p m_state: %d, m_udp: %p\n", this, m_state, m_udp);
    wait_ms(400);
#endif
    switch(m_state) {
        case MYNETDNS_START:
            m_retry = 0;
            resolve(m_hostname);
            m_state = MYNETDNS_PROCESSING;
            break;
        case MYNETDNS_PROCESSING: 
            break;
        case MYNETDNS_NOTFOUND: 
            break;
        case MYNETDNS_ERROR: 
            break;
        case MYNETDNS_OK:
            DBG2("m_retry=%d, m_interval=%d\n", m_retry, m_interval.read_ms());
            break;
    }
    if (m_interval.read_ms() > 1000) {
        m_interval.stop();
        DBG2("timeout m_retry=%d\n", m_retry);
        if (++m_retry >= 2) {
            m_state = MYNETDNS_ERROR;
        } else {
            resolve(m_hostname);
            m_state = MYNETDNS_PROCESSING;
        }
    }
}

bool DNSClient::lookup(const char* hostname) {
    m_hostname = hostname;
    m_state = MYNETDNS_START;
    while(1) {
        poll();
        callback();
        if (m_state != MYNETDNS_PROCESSING) {
            break;
        } 
    }
    return m_state == MYNETDNS_OK;
}
