#include "interrupt.h"

//void TIM3_IRQHandler()
//{
//  if(TIM3->SR & TIM_SR_UIF) // if UIF flag is set
//  {
//    TIM3->SR &= ~TIM_SR_UIF; // clear UIF flag
    
//    Interrupt::TIM3_Wrapper();
//  }
//}


void TIM3_IRQHandler()
{
  if(TIM3->SR & TIM_SR_UIF) // if UIF flag is set
  {
    TIM3->SR &= ~TIM_SR_UIF; // clear UIF flag
    
    Interrupt::TIM3_Wrapper();
  }
}

void TIM4_IRQHandler()
{
  if(TIM4->SR & TIM_SR_UIF) // if UIF flag is set
  {
    TIM4->SR &= ~TIM_SR_UIF; // clear UIF flag
    
    Interrupt::TIM4_Wrapper();
  }
}

void TIM5_IRQHandler()
{
  if(TIM5->SR & TIM_SR_UIF) // if UIF flag is set
  {
    TIM5->SR &= ~TIM_SR_UIF; // clear UIF flag
    
    Interrupt::TIM5_Wrapper();
  }
}