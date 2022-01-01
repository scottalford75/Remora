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

#include "TCPSocketServer.h"

TCPSocketServer::TCPSocketServer() {}

// Server initialization
int TCPSocketServer::bind(int port)
{
   // set the listen_port for next connection. 
   listen_port = port;
    if (_sock_fd < 0) {
        _sock_fd = eth->new_socket();
        if (_sock_fd < 0) {
            return -1;
        }
    }
    // set TCP protocol
    eth->setProtocol(_sock_fd, WIZnet_Chip::TCP);
    // set local port
    eth->sreg<uint16_t>(_sock_fd, Sn_PORT, port);
    // connect the network
    eth->scmd(_sock_fd, WIZnet_Chip::OPEN);
    return 0;
}

int TCPSocketServer::listen(int backlog)
{
    if (_sock_fd < 0) {
        return -1;
    }
    if (backlog != 1) {
        return -1;
    }
    eth->scmd(_sock_fd, WIZnet_Chip::LISTEN);
    return 0;
}


int TCPSocketServer::accept(TCPSocketConnection& connection)
{
    if (_sock_fd < 0) {
        return -1;
    }
    Timer t;
    t.reset();
    t.start();
    while(1) {
        if (t.read_ms() > _timeout && _blocking == false) {
            return -1;
        }
        if (eth->sreg<uint8_t>(_sock_fd, Sn_SR) == WIZnet_Chip::SOCK_ESTABLISHED) {
            break;
        }
    }
    uint32_t ip = eth->sreg<uint32_t>(_sock_fd, Sn_DIPR);
    char host[16];
    snprintf(host, sizeof(host), "%d.%d.%d.%d", (ip>>24)&0xff, (ip>>16)&0xff, (ip>>8)&0xff, ip&0xff);
    uint16_t port = eth->sreg<uint16_t>(_sock_fd, Sn_DPORT);

    // change this server socket to connection socket.
    connection._sock_fd = _sock_fd;
    connection._is_connected = true;
    connection.set_address(host, port);

    // and then, for the next connection, server socket should be assigned new one.
    _sock_fd = -1; // want to assign new available _sock_fd.
    if(bind(listen_port) < 0) {
        // modified by Patrick Pollet
        error("No more socket for listening, bind error");
        return -1;
    } else {
        //return -1;
        if(listen(1) < 0) {
            // modified by Patrick Pollet
            error("No more socket for listening, listen error");
            return -1;
        }
    }

    return 0;
}
