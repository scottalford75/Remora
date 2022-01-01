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

#include "WIZnetInterface.h"
#include "DHCPClient.h"

#if (not defined TARGET_WIZwiki_W7500) && (not defined TARGET_WIZwiki_W7500P) && (not defined TARGET_WIZwiki_W7500ECO)
WIZnetInterface::WIZnetInterface(PinName mosi, PinName miso, PinName sclk, PinName cs, PinName reset) :
        WIZnet_Chip(mosi, miso, sclk, cs, reset)
{
    ip_set = false;
}

WIZnetInterface::WIZnetInterface(SPI* spi, PinName cs, PinName reset) :
        WIZnet_Chip(spi, cs, reset)
{
    ip_set = false;
}
#endif

int WIZnetInterface::init()
{
    dhcp = true;
    reset();
    
    return 0;
}

int WIZnetInterface::init(uint8_t * mac)
{
    dhcp = true;
    //
    for (int i =0; i < 6; i++) this->mac[i] = mac[i];
    //
    reset();
    
    return 0;
}

int WIZnetInterface::init(uint8_t * mac, const char* ip, const char* mask, const char* gateway)
{
    dhcp = false;
    //
    for (int i =0; i < 6; i++) this->mac[i] = mac[i];
    //
    this->ip = str_to_ip(ip);
    strcpy(ip_string, ip);
    ip_set = true;
    this->netmask = str_to_ip(mask);
    this->gateway = str_to_ip(gateway);
    reset();

    // @Jul. 8. 2014 add code. should be called to write chip.
    setmac();
    setip();
    
    return 0;
}

// Connect Bring the interface up, start DHCP if needed.
int WIZnetInterface::connect()
{
    if (dhcp) {
        int r = IPrenew();
        if (r < 0) {
            return r;
        }
    }
    
    if (WIZnet_Chip::setip() == false) return -1;
    return 0;
}

// Disconnect Bring the interface down.
int WIZnetInterface::disconnect()
{
    //if (WIZnet_Chip::disconnect() == false) return -1;
    return 0;
}


char* WIZnetInterface::getIPAddress()
{
    uint32_t ip = reg_rd<uint32_t>(SIPR);
    snprintf(ip_string, sizeof(ip_string), "%d.%d.%d.%d", 
                (uint8_t)((ip>>24)&0xff), 
                (uint8_t)((ip>>16)&0xff), 
                (uint8_t)((ip>>8)&0xff), 
                (uint8_t)(ip&0xff));
    return ip_string;
}

char* WIZnetInterface::getNetworkMask()
{
    uint32_t ip = reg_rd<uint32_t>(SUBR);
    snprintf(mask_string, sizeof(mask_string), "%d.%d.%d.%d", 
                (uint8_t)((ip>>24)&0xff), 
                (uint8_t)((ip>>16)&0xff), 
                (uint8_t)((ip>>8)&0xff), 
                (uint8_t)(ip&0xff));
    return mask_string;
}

char* WIZnetInterface::getGateway()
{
    uint32_t ip = reg_rd<uint32_t>(GAR);
    snprintf(gw_string, sizeof(gw_string), "%d.%d.%d.%d", 
                (uint8_t)((ip>>24)&0xff), 
                (uint8_t)((ip>>16)&0xff), 
                (uint8_t)((ip>>8)&0xff), 
                (uint8_t)(ip&0xff));
    return gw_string;
}

char* WIZnetInterface::getMACAddress()
{
    uint8_t mac[6];
    reg_rd_mac(SHAR, mac);
    snprintf(mac_string, sizeof(mac_string), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    //ethernet_address(mac_string);
    return mac_string; 
    
}

int WIZnetInterface::IPrenew(int timeout_ms)
{
    DHCPClient dhcp;
    int err = dhcp.setup(timeout_ms);
    if (err == (-1)) {
        return -1;
    }
//    printf("Connected, IP: %d.%d.%d.%d\n", dhcp.yiaddr[0], dhcp.yiaddr[1], dhcp.yiaddr[2], dhcp.yiaddr[3]);
    ip      = (dhcp.yiaddr[0] <<24) | (dhcp.yiaddr[1] <<16) | (dhcp.yiaddr[2] <<8) | dhcp.yiaddr[3];
    gateway = (dhcp.gateway[0]<<24) | (dhcp.gateway[1]<<16) | (dhcp.gateway[2]<<8) | dhcp.gateway[3];
    netmask = (dhcp.netmask[0]<<24) | (dhcp.netmask[1]<<16) | (dhcp.netmask[2]<<8) | dhcp.netmask[3];
    dnsaddr = (dhcp.dnsaddr[0]<<24) | (dhcp.dnsaddr[1]<<16) | (dhcp.dnsaddr[2]<<8) | dhcp.dnsaddr[3];
    return 0;
}
