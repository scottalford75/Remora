/*
 * Demonstrates capturing the GPIO P0.4 to P0.7 "nibble" to memory 
 * using GPDMA. The transfers from port pins to memory buffer are
 * triggered using Timer1 MAT1.0 match compare.
 *
 * In this example all inputs have pullups. So with nothing connected
 * the P0.4/7 reads as 0xF. Connecting a wire from one or more of the four 
 * inputs to ground will show up in the captured buffer sequence.
 */
 
#include "mbed.h"
#include "MODDMA.h"
#include "iomacros.h" // within MODDMA library.

// How long between grabbing GPIO FIO0PIN register.
// Value is in microseconds. (500000 is half a second).
#define SAMPLE_PERIOD   500000

#define NUM_OF_SAMPLES  5

Serial pc(USBTX, USBRX);

DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);

uint32_t buffer[NUM_OF_SAMPLES];

bool dmaTransferComplete;

MODDMA dma;
MODDMA_Config *conf;

void TC0_callback(void);
void ERR0_callback(void);

int main() {
    volatile int life_counter = 0;
     
    // Macros defined in iomacros.h, saves messing with DigitalIn
    p30_AS_INPUT; p30_MODE( PIN_PULLUP ); // P0.4
    p29_AS_INPUT; p29_MODE( PIN_PULLUP ); // P0.5
    p8_AS_INPUT;  p8_MODE( PIN_PULLUP );  // P0.6
    p7_AS_INPUT;  p7_MODE( PIN_PULLUP );  // P0.7
    
    // Clear the buffer.
    memset(buffer, 0, sizeof(buffer));
    
    // Setup the serial port to print out results.
    pc.baud(115200);
    pc.printf("Starting up...\n");
    
    // Set-up timer1 as a periodic timer.
    LPC_SC->PCONP    |= (1UL << 2); // TIM1 On
    LPC_SC->PCLKSEL0 |= (3UL << 4); // CCLK/8 = 12MHz
    LPC_TIM1->PR      = 11;         // TC clocks at 1MHz.
    LPC_TIM1->MCR     = 2;          // Reset TCR to zero on match.
    LPC_TIM1->MR0     = SAMPLE_PERIOD;
    
    // Prepare the GPDMA system.
    conf = new MODDMA_Config;
    conf
     ->channelNum    ( MODDMA::Channel_0 )
     ->srcMemAddr    ( (uint32_t)&LPC_GPIO0->FIOPIN )
     ->dstMemAddr    ( (uint32_t)&buffer[0] )
     ->transferSize  ( NUM_OF_SAMPLES )
     ->transferType  ( MODDMA::g2m ) // pseudo transfer code MODDMA understands.
     ->transferWidth ( MODDMA::word )
     ->srcConn       ( MODDMA::MAT1_0 )
     ->dmacSync      ( MODDMA::MAT1_0 ) 
     ->attach_tc     ( TC0_callback )
     ->attach_err    ( ERR0_callback )
    ; // end conf.
    
    // Prepare configuration.
    if (!dma.Setup( conf )) {
        error("Doh!");
    }
    
    // Enable GPDMA to be ready for the TIM1 "ticks".       
    dma.Enable( conf );
    
    // Begin.
    LPC_TIM1->TCR = 1;
       
    while (1) { 
        if (life_counter++ > 1000000) {
            led1 = !led1; // Show some sort of life.
            life_counter = 0;
        }
        
        if (dmaTransferComplete) {
            dmaTransferComplete = false;
            for (int i = 0; i < NUM_OF_SAMPLES; i++) {
                int val = (buffer[i] >> 4) & 0xF; 
                pc.printf("Buffer index %d = 0x%x\n", i, val);
            }
            pc.printf("Done.\n");
            
            // Schedule another grab.
            if (dma.Setup( conf )) {        
                dma.Enable( conf );                
            }            
        }
    }       
}

// Configuration callback on TC
void TC0_callback(void) {
    
    // Just show sample sequence grab complete.
    led3 = !led3; 
        
    // Get configuration pointer.
    MODDMA_Config *config = dma.getConfig();
    
    // Finish the DMA cycle by shutting down the channel.
    dma.Disable( (MODDMA::CHANNELS)config->channelNum() );
    
    // Tell main() while(1) loop to print the results.
    dmaTransferComplete = true;            
    
    // Clear DMA IRQ flags.
    if (dma.irqType() == MODDMA::TcIrq) dma.clearTcIrq();    
    if (dma.irqType() == MODDMA::ErrIrq) dma.clearErrIrq();    
}

// Configuration callback on Error
void ERR0_callback(void) {
    error("Oh no! My Mbed EXPLODED! :( Only kidding, go find the problem");
}
