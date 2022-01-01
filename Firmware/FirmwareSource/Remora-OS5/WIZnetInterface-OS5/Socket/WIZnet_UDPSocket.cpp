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

#include "WIZnet_UDPSocket.h"

static int udp_local_port;

WIZnet_UDPSocket::WIZnet_UDPSocket()
{
}
// After init function, bind() should be called.
int WIZnet_UDPSocket::init(void)
{
    if (_sock_fd < 0) {
        _sock_fd = eth->new_socket();
    }
    if (eth->setProtocol(_sock_fd, WIZnet_Chip::UDP) == false) return -1;
    return 0;
}

// Server initialization
int WIZnet_UDPSocket::bind(int port)
{
    if (_sock_fd < 0) {
        _sock_fd = eth->new_socket();
        if (_sock_fd < 0) {
            return -1;
        }
    }
    // set local port
    if (port != 0) {
        eth->sreg<uint16_t>(_sock_fd, Sn_PORT, port);
    } else {
        udp_local_port++;
        eth->sreg<uint16_t>(_sock_fd, Sn_PORT, udp_local_port);
    }
    // set udp protocol
    eth->setProtocol(_sock_fd, WIZnet_Chip::UDP);
    eth->scmd(_sock_fd, WIZnet_Chip::OPEN);
    return 0;
}

// -1 if unsuccessful, else number of bytes written
int WIZnet_UDPSocket::sendTo(Endpoint &remote, char *packet, int length)
{
    int size = eth->wait_writeable(_sock_fd, _blocking ? -1 : _timeout, length-1);
    if (size < 0) {
        return -1;
    }
    confEndpoint(remote);
    int ret = eth->send(_sock_fd, packet, length);
    return ret;
}

// -1 if unsuccessful, else number of bytes received
int WIZnet_UDPSocket::receiveFrom(Endpoint &remote, char *buffer, int length)
{
    uint8_t info[8];
    int size = eth->wait_readable(_sock_fd, _blocking ? -1 : _timeout, sizeof(info));
    if (size < 0) {
        return -1;
    }
    eth->recv(_sock_fd, (char*)info, sizeof(info));
    readEndpoint(remote, info);
    int udp_size = info[6]<<8|info[7];
    //TEST_ASSERT(udp_size <= (size-sizeof(info)));
    if (udp_size > (size-sizeof(info))) {
        return -1;
    }

    /* Perform Length check here to prevent buffer overrun */
    /* fixed by Sean Newton (https://developer.mbed.org/users/SeanNewton/) */
    if (udp_size > length) {
        //printf("udp_size: %d\n",udp_size);
        return -1;
    }
    return eth->recv(_sock_fd, buffer, udp_size);
}

void WIZnet_UDPSocket::confEndpoint(Endpoint & ep)
{
    char * host = ep.get_address();
    // set remote host
    eth->sreg_ip(_sock_fd, Sn_DIPR, host);
    // set remote port
    eth->sreg<uint16_t>(_sock_fd, Sn_DPORT, ep.get_port());
}

void WIZnet_UDPSocket::readEndpoint(Endpoint & ep, uint8_t info[])
{
    char addr[17];
    snprintf(addr, sizeof(addr), "%d.%d.%d.%d", info[0], info[1], info[2], info[3]);
    uint16_t port = info[4]<<8|info[5];
    ep.set_address(addr, port);
}
