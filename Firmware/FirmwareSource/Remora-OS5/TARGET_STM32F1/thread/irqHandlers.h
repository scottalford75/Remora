#include "interrupt.h"


void TIM1_IRQHandler()
{
  if(TIM1->SR & TIM_SR_UIF) // if UIF flag is set
  {
    TIM1->SR &= ~TIM_SR_UIF; // clear UIF flag
    
    Interrupt::TIM1_Wrapper();
  }
}

void TIM2_IRQHandler()
{
  if(TIM2->SR & TIM_SR_UIF) // if UIF flag is set
  {
    TIM2->SR &= ~TIM_SR_UIF; // clear UIF flag
    
    Interrupt::TIM2_Wrapper();
  }
}

void TIM3_IRQHandler()
{
  if(TIM3->SR & TIM_SR_UIF) // if UIF flag is set
  {
    TIM3->SR &= ~TIM_SR_UIF; // clear UIF flag
    
    Interrupt::TIM3_Wrapper();
  }
}