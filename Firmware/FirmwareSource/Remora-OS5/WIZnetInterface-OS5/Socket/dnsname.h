// dnsname.h 2013/8/27
#pragma once
//#include <string>
#include "pico_string.h"
class dnsname {
public:
    uint8_t *buf;
    pico_string str;
    dnsname(uint8_t *s) {
        buf = s;
    }
    int decode(int pos) {
        while(1) {
            int len = buf[pos++];
            if (len == 0x00) {
                break;
            }
            if ((len&0xc0) == 0xc0) { //compress
                int offset = (len&0x3f)<<8|buf[pos];
                decode(offset);
                return pos+1;
            }
            if (!str.empty()) {
                str.append(".");
            }
            str.append((const char*)(buf+pos), len);
            pos += len;
        }
        return pos;
    }

    int encode(int pos, char* s) {
        while(*s) {  
            char *f = strchr(s, '.');
            if (f == NULL) {
                int len = strlen(s);
                buf[pos++] = len;
                memcpy(buf+pos, s, len);
                pos += len;
                break;
            }
            int len = f - s;
            buf[pos++] = len;
            memcpy(buf+pos, s, len);
            s = f+1;
            pos += len;
        }
        buf[pos++] = 0x00;
        return pos;
    }
};
