#ifndef INTERRUPT_H
#define INTERRUPT_H

// Base class for all interrupt derived classes

#define PERIPH_COUNT_IRQn	32				// Total number of device interrupt sources

class Interrupt
{
	protected:

		static Interrupt* ISRVectorTable[PERIPH_COUNT_IRQn];

	public:

		Interrupt(void);

		//static Interrupt* ISRVectorTable[PERIPH_COUNT_IRQn];

		static void Register(int interruptNumber, Interrupt* intThisPtr);

		// wrapper functions to ISR_Handler()
		static void TIMER0_Wrapper();
		static void TIMER1_Wrapper();
        static void TIMER2_Wrapper();
        static void QEI_Wrapper();

		virtual void ISR_Handler(void) = 0;

};

#endif


/******  LPC17xx Specific Interrupt Numbers ******************************************************
  WDT_IRQn                      = 0,        Watchdog Timer Interrupt
  TIMER0_IRQn                   = 1,        Timer0 Interrupt
  TIMER1_IRQn                   = 2,        Timer1 Interrupt
  TIMER2_IRQn                   = 3,        Timer2 Interrupt
  TIMER3_IRQn                   = 4,        Timer3 Interrupt
  UART0_IRQn                    = 5,        UART0 Interrupt
  UART1_IRQn                    = 6,        UART1 Interrupt
  UART2_IRQn                    = 7,        UART2 Interrupt
  UART3_IRQn                    = 8,        UART3 Interrupt
  PWM1_IRQn                     = 9,        PWM1 Interrupt
  I2C0_IRQn                     = 10,       I2C0 Interrupt
  I2C1_IRQn                     = 11,       I2C1 Interrupt
  I2C2_IRQn                     = 12,       I2C2 Interrupt
  SPI_IRQn                      = 13,       SPI Interrupt
  SSP0_IRQn                     = 14,       SSP0 Interrupt
  SSP1_IRQn                     = 15,       SSP1 Interrupt
  PLL0_IRQn                     = 16,       PLL0 Lock (Main PLL) Interrupt
  RTC_IRQn                      = 17,       Real Time Clock Interrupt
  EINT0_IRQn                    = 18,       External Interrupt 0 Interrupt
  EINT1_IRQn                    = 19,       External Interrupt 1 Interrupt
  EINT2_IRQn                    = 20,       External Interrupt 2 Interrupt
  EINT3_IRQn                    = 21,       External Interrupt 3 Interrupt
  ADC_IRQn                      = 22,       A/D Converter Interrupt
  BOD_IRQn                      = 23,       Brown-Out Detect Interrupt
  USB_IRQn                      = 24,       USB Interrupt
  CAN_IRQn                      = 25,       CAN Interrupt
  DMA_IRQn                      = 26,       General Purpose DMA Interrupt
  I2S_IRQn                      = 27,       I2S Interrupt
  ENET_IRQn                     = 28,       Ethernet Interrupt
  RIT_IRQn                      = 29,       Repetitive Interrupt Timer Interrupt
  MCPWM_IRQn                    = 30,       Motor Control PWM Interrupt
  QEI_IRQn                      = 31,       Quadrature Encoder Interface Interrupt
  PLL1_IRQn                     = 32,       PLL1 Lock (USB PLL) Interrupt

PERIPH_COUNT_IRQn    = 32  < Number of peripheral IDs */
