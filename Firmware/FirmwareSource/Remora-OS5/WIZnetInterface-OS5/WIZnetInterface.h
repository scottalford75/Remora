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

#pragma once
#include "eth_arch.h"
 /** Interface using Wiznet chip to connect to an IP-based network
 *
 */
class WIZnetInterface: public WIZnet_Chip {
public:

#if (not defined TARGET_WIZwiki_W7500) && (not defined TARGET_WIZwiki_W7500P) && (not defined TARGET_WIZwiki_W7500ECO)

    /**
    * Constructor
    *
    * \param mosi mbed pin to use for SPI
    * \param miso mbed pin to use for SPI
    * \param sclk mbed pin to use for SPI
    * \param cs chip select of the WIZnet_Chip
    * \param reset reset pin of the WIZnet_Chip
    */
    WIZnetInterface(PinName mosi, PinName miso, PinName sclk, PinName cs, PinName reset);
    WIZnetInterface(SPI* spi, PinName cs, PinName reset);
#endif

  /** Initialize the interface with DHCP.
  * Initialize the interface and configure it to use DHCP (no connection at this point).
  * \return 0 on success, a negative number on failure
  */
  int init();              //With DHCP
  int init(uint8_t * mac); //With DHCP

  /** Initialize the interface with a static IP address.
  * Initialize the interface and configure it with the following static configuration (no connection at this point).
  * \param ip the IP address to use
  * \param mask the IP address mask
  * \param gateway the gateway to use
  * \return 0 on success, a negative number on failure
  */
  int init(uint8_t * mac, const char* ip, const char* mask, const char* gateway);

  /** Connect
  * Bring the interface up, start DHCP if needed.
  * \return 0 on success, a negative number on failure
  */
  int connect();
  
  /** Disconnect
  * Bring the interface down
  * \return 0 on success, a negative number on failure
  */
  int disconnect();
  
  /** Get IP address & MAC address
  *
  * @ returns ip address
  */
  char* getIPAddress();
  char* getNetworkMask();
  char* getGateway();
  char* getMACAddress();

  int IPrenew(int timeout_ms = 15*1000);
    
private:
    char ip_string[20];
    char mask_string[20];
    char gw_string[20];
    char mac_string[20];
    bool ip_set;
};

#include "TCPSocketConnection.h"
#include "TCPSocketServer.h"
#include "WIZnet_UDPSocket.h"
