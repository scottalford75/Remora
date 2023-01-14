#include "interrupt.h"

//void TIM3_IRQHandler()
//{
//  if(TIM3->SR & TIM_SR_UIF) // if UIF flag is set
//  {
//    TIM3->SR &= ~TIM_SR_UIF; // clear UIF flag
    
//    Interrupt::TIM3_Wrapper();
//  }
//}


void TIM9_IRQHandler()
{
  if(TIM9->SR & TIM_SR_UIF) // if UIF flag is set
  {
    TIM9->SR &= ~TIM_SR_UIF; // clear UIF flag
    
    Interrupt::TIM9_Wrapper();
  }
}

void TIM10_IRQHandler()
{
  if(TIM10->SR & TIM_SR_UIF) // if UIF flag is set
  {
    TIM10->SR &= ~TIM_SR_UIF; // clear UIF flag
    
    Interrupt::TIM10_Wrapper();
  }
}

void TIM11_IRQHandler()
{
  if(TIM11->SR & TIM_SR_UIF) // if UIF flag is set
  {
    TIM11->SR &= ~TIM_SR_UIF; // clear UIF flag
    
    Interrupt::TIM11_Wrapper();
  }
}