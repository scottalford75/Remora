
#include "SoftwareSerial.h"
#include <cstdint>



SoftwareSerial::SoftwareSerial(std::string tx, std::string rx)
{
    if (!tx.empty()) TXportAndPin = tx;
    if (!rx.empty()) RXportAndPin = rx;
    halfDuplex = !TXportAndPin.compare(RXportAndPin);

    if(halfDuplex)
    {
        this->rxpin = new Pin(RXportAndPin,1);
        this->txpin = this->rxpin;
        setTX();
    }
    else
    {
        this->txpin = new Pin(TXportAndPin,0);
        setTX();

        this->rxpin = new Pin(RXportAndPin,1);
        setRX();
    }
    
    qin = 0;
    qout = 0;
    activeTx = false;
    activeRx = false;
}


void SoftwareSerial::begin(int baudrate)
{
    #ifdef FORCE_BAUD_RATE
    baudrate = FORCE_BAUD_RATE;     // 19200 fastest stable baud rate
    #endif
    baudRate = baudrate;
    //ticker.attach_us(callback(this, &SoftwareSerial::tickerHandler), 1000000.0 / (baudRate * 3.0));
}

void SoftwareSerial::setSpeed(int baudrate)
{
    //ticker.detach();
  
    //ticker.attach_us(callback(this, &SoftwareSerial::tickerHandler), 1000000.0 / (baudrate * 3.0));
    this->baudRate = baudrate;
}

void SoftwareSerial::setTX(void)
{
    // First write, then set output. If we do this the other way around,
    // the pin would be output low for a short while before switching to
    // output hihg. Now, it is input with pullup for a short while, which
    // is fine. With inverse logic, either order is fine.

    //this->txpin->set(1);                  // works for LPC1768 but not STM32
    this->txpin->setAsOutput();
    this->txpin->set(1);
}

void SoftwareSerial::setRX(void)
{

    this->rxpin->setAsInput();
    this->rxpin->pull_up();
}

void SoftwareSerial::setRXTX(bool input)
{
    if (halfDuplex)
    {
        if (input)
        {
            setRX();
            rxBitCnt = -1;
            rxTickCnt = 2;
            activeRx = true;
        }
        else
        {
            if (activeRx)
            {
                setTX();
                activeRx = false;
            }
        }
    }
}

bool SoftwareSerial::listen()
{
    if (rxpin != nullptr)
    {
        setRXTX(true);
        return true;
    }
    return false;
}

void SoftwareSerial::end(void)
{
    
}

void SoftwareSerial::tickerHandler(void)
{
    if (activeTx) this->send();
    if (activeRx) this->receive();
}

void SoftwareSerial::send(void)
{
    if (--txTickCnt <= 0)
    {
        if (txBitCnt++ < TX_BITS)   // count out the bits in the txBuffer
        {
            this->txpin->set(txBuffer & 0x01);   // set output equal to the LSB in txBuffer
            txBuffer >>= 1;                     // shift txBuffer to right
            txTickCnt = OVERSAMPLE;             // reset the tick counter
        }
        else    // transmit finished, stay active or wait for a period before swapping to Rx mode if half duplex mode
        {
            txTickCnt = 1;
            if (outputPending)
            {
                activeTx = false;    // output pending allow new byte to be written to txBuffer from write()
            }
            else if (txBitCnt > 10 + OVERSAMPLE*5)
            {
                if (halfDuplex)
                {
                    setRXTX(true);        // switch to receive mode
                }
                activeTx = false;
            }
        }
    }
}

void SoftwareSerial::receive()
{
    if (--rxTickCnt <= 0)
    {
        uint8_t inbit = this->rxpin->get();   // read the rx line
        if (rxBitCnt == -1)                 // waiting for start bit
        {
            if (!inbit)
            {
                // got a start bit
                rxBitCnt = 0;
                rxTickCnt = OVERSAMPLE + 1;
                rxBuffer = 0;
            }
            else
            {
                rxTickCnt = 1;
            }
        }
        else if (rxBitCnt >= RX_BITS)     // full byte has been read
        {
            // add stop bit to buffer
            inbuf[qin] = rxBuffer;
			if ( ++qin >= IN_BUF_SIZE )
            {
			    // overflow - reset inbuf-index
				qin = 0;
			}
            rxTickCnt = 1;
            rxBitCnt = -1;              // flag waiting for start bit
        }
        else                            // read data bits
        {
            rxBuffer >>= 1;
            if (inbit)  rxBuffer |= 0x80;
            rxBitCnt++;
            rxTickCnt = OVERSAMPLE;
        }
    }
}


int SoftwareSerial::available()
{
    return (qout - qin);
}


void SoftwareSerial::printStr(char* str)
{
    int i = 0;
    int len = strlen(str); 
    for(i = 0; i<len; i++)
    {
        write(str[i]);
    }
}


void SoftwareSerial::write(int b)
{
    outputPending = true;               // notify ticker handler that there are more bytes to transmit after current
    while (activeTx)                    // wait for current transmission to complete
    {
        idle();
    }
    txBuffer =      (b << 1) | 0x200;   // add start and stop bits
    txBitCnt =      0;
    txTickCnt =     OVERSAMPLE;
    if (halfDuplex) setRXTX(false);
    outputPending = false;
    activeTx =      true;
}

int16_t SoftwareSerial::read()
{
    if (qout == qin) return -1;

    char d = inbuf[qout] & 0xFF;

    if ( ++qout >= IN_BUF_SIZE ) {qout = 0;}
    
    return d;
}