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

#include "TCPSocketConnection.h"
#include <cstring>

using std::memset;
using std::memcpy;

// not a big code.
// refer from EthernetInterface by mbed official driver
TCPSocketConnection::TCPSocketConnection() :
    _is_connected(false)
{
}

int TCPSocketConnection::connect(const char* host, const int port)
{
    if (_sock_fd < 0) {
        _sock_fd = eth->new_socket();
        if (_sock_fd < 0) {
            return -1;
        }
    }
    if (set_address(host, port) != 0) {
        return -1;
    }
    if (!eth->connect(_sock_fd, get_address(), port)) {
        return -1;
    }
    set_blocking(false);
    // add code refer from EthernetInterface.
    _is_connected = true;

    return 0;
}

bool TCPSocketConnection::is_connected(void)
{
    // force update recent state.
    _is_connected = eth->is_connected(_sock_fd);
    return _is_connected;
}

int TCPSocketConnection::send(char* data, int length)
{
    if((_sock_fd<0) || !(eth->is_connected(_sock_fd)))
        return -1;

    int size = eth->wait_writeable(_sock_fd, _blocking ? -1 : _timeout);
    if (size < 0) 
        return -1;

    if (size > length) 
        size = length;

    return eth->send(_sock_fd, data, size);
}

// -1 if unsuccessful, else number of bytes written
int TCPSocketConnection::send_all(char* data, int length)
{
    int writtenLen = 0;

    if(_sock_fd<0)
        return -1;

    while (writtenLen < length) {

        if(!(eth->is_connected(_sock_fd)))
            return -1;

        int size = eth->wait_writeable(_sock_fd, _blocking ? -1 : _timeout);
        if (size < 0) {
            return -1;
        }
        if (size > (length-writtenLen)) {
            size = (length-writtenLen);
        }
        int ret = eth->send(_sock_fd, data + writtenLen, size);
        if (ret < 0) {
            return -1;
        }
        writtenLen += ret;
    }
    return writtenLen;
}

// -1 if unsuccessful, else number of bytes received
int TCPSocketConnection::receive(char* data, int length)
{
    if((_sock_fd<0) || !(eth->is_connected(_sock_fd)))
        return -1;

    int size = eth->wait_readable(_sock_fd, _blocking ? -1 : _timeout);
    if (size < 0) {
        return -1;
    }
    if (size > length) {
        size = length;
    }
    return eth->recv(_sock_fd, data, size);
}

// -1 if unsuccessful, else number of bytes received
int TCPSocketConnection::receive_all(char* data, int length)
{
    if(_sock_fd<0)
        return -1;

    int readLen = 0;
    while (readLen < length) {

        if(!(eth->is_connected(_sock_fd)))
            return -1;

        int size = eth->wait_readable(_sock_fd, _blocking ? -1 :_timeout);
        if (size <= 0) {
            break;
        }
        if (size > (length - readLen)) {
            size = length - readLen;
        }
        int ret = eth->recv(_sock_fd, data + readLen, size);
        if (ret < 0) {
            return -1;
        }
        readLen += ret;
    }
    return readLen;
}
