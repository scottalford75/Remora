#ifndef INTERRUPT_H
#define INTERRUPT_H

// Base class for all interrupt derived classes

#define PERIPH_COUNT_IRQn	60				// Total number of device interrupt sources

class Interrupt
{
	protected:

		static Interrupt* ISRVectorTable[PERIPH_COUNT_IRQn];

	public:

		Interrupt(void);

		//static Interrupt* ISRVectorTable[PERIPH_COUNT_IRQn];

		static void Register(int interruptNumber, Interrupt* intThisPtr);

		// wrapper functions to ISR_Handler()
		static void TIM1_Wrapper();
        static void TIM2_Wrapper();
        static void TIM3_Wrapper();

		virtual void ISR_Handler(void) = 0;

};

#endif

/******  STM32 specific Interrupt Numbers *********************************************************************
  WWDG_IRQn                   = 0,      !< Window WatchDog Interrupt                            
  PVD_IRQn                    = 1,      !< PVD through EXTI Line detection Interrupt            
  TAMPER_IRQn                 = 2,      !< Tamper Interrupt                                     
  RTC_IRQn                    = 3,      !< RTC global Interrupt                                 
  FLASH_IRQn                  = 4,      !< FLASH global Interrupt                               
  RCC_IRQn                    = 5,      !< RCC global Interrupt                                 
  EXTI0_IRQn                  = 6,      !< EXTI Line0 Interrupt                                 
  EXTI1_IRQn                  = 7,      !< EXTI Line1 Interrupt                                 
  EXTI2_IRQn                  = 8,      !< EXTI Line2 Interrupt                                 
  EXTI3_IRQn                  = 9,      !< EXTI Line3 Interrupt                                 
  EXTI4_IRQn                  = 10,     !< EXTI Line4 Interrupt                                 
  DMA1_Channel1_IRQn          = 11,     !< DMA1 Channel 1 global Interrupt                      
  DMA1_Channel2_IRQn          = 12,     !< DMA1 Channel 2 global Interrupt                      
  DMA1_Channel3_IRQn          = 13,     !< DMA1 Channel 3 global Interrupt                      
  DMA1_Channel4_IRQn          = 14,     !< DMA1 Channel 4 global Interrupt                      
  DMA1_Channel5_IRQn          = 15,     !< DMA1 Channel 5 global Interrupt                      
  DMA1_Channel6_IRQn          = 16,     !< DMA1 Channel 6 global Interrupt                      
  DMA1_Channel7_IRQn          = 17,     !< DMA1 Channel 7 global Interrupt                      
  ADC1_2_IRQn                 = 18,     !< ADC1 and ADC2 global Interrupt                       
  USB_HP_CAN1_TX_IRQn         = 19,     !< USB Device High Priority or CAN1 TX Interrupts       
  USB_LP_CAN1_RX0_IRQn        = 20,     !< USB Device Low Priority or CAN1 RX0 Interrupts       
  CAN1_RX1_IRQn               = 21,     !< CAN1 RX1 Interrupt                                   
  CAN1_SCE_IRQn               = 22,     !< CAN1 SCE Interrupt                                   
  EXTI9_5_IRQn                = 23,     !< External Line[9:5] Interrupts                        
  TIM1_BRK_IRQn               = 24,     !< TIM1 Break Interrupt                                 
  TIM1_UP_IRQn                = 25,     !< TIM1 Update Interrupt                                
  TIM1_TRG_COM_IRQn           = 26,     !< TIM1 Trigger and Commutation Interrupt               
  TIM1_CC_IRQn                = 27,     !< TIM1 Capture Compare Interrupt                       
  TIM2_IRQn                   = 28,     !< TIM2 global Interrupt                                
  TIM3_IRQn                   = 29,     !< TIM3 global Interrupt                                
  TIM4_IRQn                   = 30,     !< TIM4 global Interrupt                                
  I2C1_EV_IRQn                = 31,     !< I2C1 Event Interrupt                                 
  I2C1_ER_IRQn                = 32,     !< I2C1 Error Interrupt                                 
  I2C2_EV_IRQn                = 33,     !< I2C2 Event Interrupt                                 
  I2C2_ER_IRQn                = 34,     !< I2C2 Error Interrupt                                 
  SPI1_IRQn                   = 35,     !< SPI1 global Interrupt                                
  SPI2_IRQn                   = 36,     !< SPI2 global Interrupt                                
  USART1_IRQn                 = 37,     !< USART1 global Interrupt                              
  USART2_IRQn                 = 38,     !< USART2 global Interrupt                              
  USART3_IRQn                 = 39,     !< USART3 global Interrupt                              
  EXTI15_10_IRQn              = 40,     !< External Line[15:10] Interrupts                      
  RTC_Alarm_IRQn              = 41,     !< RTC Alarm through EXTI Line Interrupt                
  USBWakeUp_IRQn              = 42,     !< USB Device WakeUp from suspend through EXTI Line Interrupt 
  TIM8_BRK_IRQn               = 43,     !< TIM8 Break Interrupt                                 
  TIM8_UP_IRQn                = 44,     !< TIM8 Update Interrupt                                
  TIM8_TRG_COM_IRQn           = 45,     !< TIM8 Trigger and Commutation Interrupt               
  TIM8_CC_IRQn                = 46,     !< TIM8 Capture Compare Interrupt                       
  ADC3_IRQn                   = 47,     !< ADC3 global Interrupt                                
  FSMC_IRQn                   = 48,     !< FSMC global Interrupt                                
  SDIO_IRQn                   = 49,     !< SDIO global Interrupt                                
  TIM5_IRQn                   = 50,     !< TIM5 global Interrupt                                
  SPI3_IRQn                   = 51,     !< SPI3 global Interrupt                                
  UART4_IRQn                  = 52,     !< UART4 global Interrupt                               
  UART5_IRQn                  = 53,     !< UART5 global Interrupt                               
  TIM6_IRQn                   = 54,     !< TIM6 global Interrupt                                
  TIM7_IRQn                   = 55,     !< TIM7 global Interrupt                                
  DMA2_Channel1_IRQn          = 56,     !< DMA2 Channel 1 global Interrupt                      
  DMA2_Channel2_IRQn          = 57,     !< DMA2 Channel 2 global Interrupt                      
  DMA2_Channel3_IRQn          = 58,     !< DMA2 Channel 3 global Interrupt                      
  DMA2_Channel4_5_IRQn        = 59,     !< DMA2 Channel 4 and Channel 5 global Interrupt                                                   
*/