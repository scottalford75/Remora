// pico_string.h 2013/8/27
#pragma once
class pico_string {
public:
    pico_string(){
        _len = 0;
        _buf = (char*)malloc(1);
        if (_buf) {
            _buf[0] = '\0';
        }
    }
    ~pico_string() {
        if (_buf) {
            free(_buf);
        }
    }
    bool empty() {
        return _len == 0;
    }
    void append(const char* s, int len) {
        if (_buf == NULL) {
            return;
        }
        char* p = (char*)malloc(_len+len+1);
        if (p == NULL) {
            return;
        }
        memcpy(p, _buf, _len);
        memcpy(p+_len, s, len);
        p[_len+len] = '\0';
        free(_buf);
        _buf = p;
    }
    void append(const char* s) {
        append(s, strlen(s));
    }
    char* c_str() {
        if (_buf) {
            return _buf;
        }
        return "";
    }
private:
    char* _buf;
    int _len;
};
