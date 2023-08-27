#include "mbed.h"

#include "qeiDriver.h"
#include "interrupt.h"
#include "qeiInterrupt.h"

QEIdriver::QEIdriver()
{
    this->hasIndex = false;

    this->dirinv = 0;
    this->sigmode = 0;  // quadrature inputs
    this->capmode = 1;  // count channels A and B (4x mode)
    this->invinx = 0;

    this->init();
}


QEIdriver::QEIdriver(bool hasIndex) :
    hasIndex(hasIndex)
{
    this->hasIndex = true;

    this->dirinv = 0;
    this->sigmode = 0;  // quadrature inputs
    this->capmode = 1;  // count channels A and B (4x mode)
    this->invinx = 0;

    this->irq = QEI_IRQn;

    this->init();

    interruptPtr = new qeiInterrupt(this->irq, this);	// Instantiate a new Timer Interrupt object and pass "this" pointer

    NVIC_EnableIRQ(this->irq);
}


void QEIdriver::interruptHandler()
{
    this->indexDetected = true;
    this->indexCount = this->get();
}


uint32_t QEIdriver::get()
{
    return (LPC_QEI->QEIPOS);
}


void QEIdriver::init()
{
    printf("  Initialising hardware QEI module\n");

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
