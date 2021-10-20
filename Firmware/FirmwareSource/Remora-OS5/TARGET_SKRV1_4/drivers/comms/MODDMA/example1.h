#include "mbed.h"
#include "MODDMA.h"
#include "MODSERIAL.h"

DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);
MODDMA dma;
MODSERIAL pc(USBTX, USBRX);

// Function prototypes for IRQ callbacks.
// See definitions following main() below.
void dmaTCCallback(void);
void dmaERRCallback(void);
void TC0_callback(void);
void ERR0_callback(void);

int main() {
    char s[] = "**DMA** ABCDEFGHIJKLMNOPQRSTUVWXYZ **DMA**";
    
    pc.baud(PC_BAUD);
    
    dma.attach_tc( &dmaTCCallback );
    dma.attach_err( &dmaERRCallback );
    
    MODDMA_Config *config = new MODDMA_Config;
    config
     ->channelNum    ( MODDMA::Channel_0 )
     ->srcMemAddr    ( (uint32_t) &s )
     ->dstMemAddr    ( 0 )
     ->transferSize  ( sizeof(s) )
     ->transferType  ( MODDMA::m2p )
     ->transferWidth ( 0 )
     ->srcConn       ( 0 )
     ->dstConn       ( MODDMA::UART0_Tx )
     ->dmaLLI        ( 0 )
     ->attach_tc     ( &TC0_callback )
     ->attach_err    ( &ERR0_callback )
    ; // config end
    
    // Setup the configuration.
    dma.Setup(config);
    
    //dma.Enable( MODDMA::Channel_0 );
    //dma.Enable( config->channelNum() );
    dma.Enable( config );
    
    while (1) {
        led1 = !led1;
        wait(0.25);        
    }
}

// Main controller TC IRQ callback
void dmaTCCallback(void) {
    led2 = 1;
}

// Main controller ERR IRQ callback
void dmaERRCallback(void) {
    error("Oh no! My Mbed exploded! :( Only kidding, find the problem");
}

// Configuration callback on TC
void TC0_callback(void) {
    MODDMA_Config *config = dma.getConfig();
    dma.haltAndWaitChannelComplete( (MODDMA::CHANNELS)config->channelNum());
    dma.Disable( (MODDMA::CHANNELS)config->channelNum() );
    
    // Configurations have two IRQ callbacks for TC and Err so you 
    // know which you are processing. However, if you want to use 
    // a single callback function you can tell what type of IRQ 
    // is being processed thus:-
    if (dma.irqType() == MODDMA::TcIrq)  {
        led3 = 1;
        dma.clearTcIrq();
    }
    if (dma.irqType() == MODDMA::ErrIrq) {
        led4 = 1;
        dma.clearErrIrq();
    }
}

// Configuration cakllback on Error
void ERR0_callback(void) {
    error("Oh no! My Mbed exploded! :( Only kidding, find the problem");
}

