#include "mbed.h"
#include "qei.h"
#include "interrupt.h"
#include "qeiInterrupt.h"


QEI::QEI(volatile float &ptrEncoderCount) :
	ptrEncoderCount(&ptrEncoderCount)
{
    this->hasIndex = false;

    this->dirinv = 0;
    this->sigmode = 0;  // quadrature inputs
    this->capmode = 1;  // count channels A and B (4x mode)
    this->invinx = 0;

    this->configQEI();
}

QEI::QEI(volatile float &ptrEncoderCount, volatile uint8_t &ptrData, int bitNumber) :
	ptrEncoderCount(&ptrEncoderCount),
    ptrData(&ptrData),
    bitNumber(bitNumber)
{
    this->hasIndex = true;
    this->indexDetected = false;
    this->indexPulse = 100;                             
	this->count = 0;								    
    this->indexCount = 0;
    this->oldIndexCount = 0;
    this->pulseCount = 0;                               
    this->mask = 1 << this->bitNumber;

    this->dirinv = 0;
    this->sigmode = 0;                                  // quadrature inputs
    this->capmode = 1;                                  // count channels A and B (4x mode)
    this->invinx = 0;

    this->irq = QEI_IRQn;


    this->configQEI();

    interruptPtr = new qeiInterrupt(this->irq, this);	// Instantiate a new Timer Interrupt object and pass "this" pointer

    NVIC_EnableIRQ(this->irq);
}


void QEI::interruptHandler()
{
    this->indexDetected = true;
    this->indexCount = this->getPosition();
}


uint32_t QEI::getPosition()
{
    return (LPC_QEI->QEIPOS);
}


void QEI::update()
{
    this->count = getPosition();

    if (this->hasIndex)                                     // we have an index pin
    {
        // handle index, index pulse and pulse count
        if (this->indexDetected && (this->pulseCount == 0))    // index interrupt occured: rising edge on index pulse
        {
            *(this->ptrEncoderCount) = this->indexCount;
            this->pulseCount = this->indexPulse;        
            *(this->ptrData) |= this->mask;                 // set bit in data source high
        }
        else if (this->pulseCount > 0)                      // maintain both index output and encoder count for the latch period
        {
            this->indexDetected = false;
            this->pulseCount--;                             // decrement the counter
        }
        else
        {
            *(this->ptrData) &= ~this->mask;                // set bit in data source low
            *(this->ptrEncoderCount) = this->count;         // update encoder count
        }
    }
    else
    {
        *(this->ptrEncoderCount) = this->count;             // update encoder count
    }
}


void QEI::configQEI()
{
    printf("  Configuring hardware QEI module\n");

    /* Set up clock and power for QEI module */
    LPC_SC->PCONP |= PCONP_QEI_ENABLE;

    /* The clock for theQEI module is set to FCCLK  */
    LPC_SC->PCLKSEL1 = LPC_SC->PCLKSEL1 & ~(3UL<<0) | ((PCLKSEL_CCLK_DIV_1 & 3)<<0); 

    /* Assign the pins. They are hard-coded, not user-selected. */
    // MCI0 (PhA)
    LPC_PINCON->PINSEL3 = (LPC_PINCON->PINSEL3 & PINSEL3_MCI0_MASK) | PINSEL3_MCI0 ;
    LPC_PINCON->PINMODE3 = (LPC_PINCON->PINMODE3 & PINMODE3_MCI0_MASK) | PINMODE3_MCI0;

    // MCI1 (PhB)
    LPC_PINCON->PINSEL3 = (LPC_PINCON->PINSEL3 & PINSEL3_MCI1_MASK) | PINSEL3_MCI1 ;
    LPC_PINCON->PINMODE3 = (LPC_PINCON->PINMODE3 & PINMODE3_MCI1_MASK) | PINMODE3_MCI1;

    // MCI2 (Index)
    if (hasIndex)
    {
        LPC_PINCON->PINSEL3 = (LPC_PINCON->PINSEL3 & PINSEL3_MCI2_MASK) | PINSEL3_MCI2 ;
        LPC_PINCON->PINMODE3 = (LPC_PINCON->PINMODE3 & PINMODE3_MCI2_MASK) | PINMODE3_MCI2;
    }

    // Initialize all remaining values in QEI peripheral
    LPC_QEI->QEICON = QEI_CON_RESP | QEI_CON_RESV | QEI_CON_RESI;
    LPC_QEI->QEIMAXPOS = 0xFFFFFFFF;                          // Default value
    LPC_QEI->CMPOS0 = 0x00;
    LPC_QEI->CMPOS1 = 0x00;
    LPC_QEI->CMPOS2 = 0x00;
    LPC_QEI->INXCMP = 0x00;
    LPC_QEI->QEILOAD = 0x00;
    LPC_QEI->VELCOMP = 0x00;
    LPC_QEI->FILTER = 200000;       // Default for mechanical switches.

    // Set QEI configuration value corresponding to the call parameters
    LPC_QEI->QEICONF = (
        ((dirinv << 0) & 1) | \
        ((sigmode << 1) & 2) | \
        ((capmode << 2) & 4) | \
        ((invinx <<3) & 8) );
       
    // Mask all int sources   
    LPC_QEI->QEIIEC = QEI_IECLR_BITMASK;    // Set the "clear" bits for all sources in the IE clear register              

    // Clear any pending ints    
    LPC_QEI->QEICLR = QEI_INTCLR_BITMASK;   // Set the "clear" bits for for all sources in the Interrupt clear register

    // Enable specified interrupt on QEI peropheral
    LPC_QEI->QEIIES = QEI_INTSTAT_INX_Int;

    // set digital filter
    LPC_QEI->FILTER = 480UL;
    
    // set max position
    LPC_QEI->QEIMAXPOS = 0xFFFFFFFF;
}

