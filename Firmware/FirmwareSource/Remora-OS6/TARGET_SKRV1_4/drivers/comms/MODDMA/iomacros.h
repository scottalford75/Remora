/*
    Copyright (c) 2011 Andy Kirkham
 
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:
 
    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.
 
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/

#ifndef IOMACROS_H
#define IOMACROS_H

#ifndef __LPC17xx_H__
#include "LPC17xx.h"
#endif

#define PIN_PULLUP      0UL
#define PIN_REPEAT      1UL
#define PIN_NONE        2UL
#define PIN_PULLDOWN    3UL

/* p5 is P0.9 */
#define p5_SEL_MASK     ~(3UL << 18)
#define p5_SET_MASK     (1UL << 9)
#define p5_CLR_MASK     ~(p5_SET_MASK)
#define p5_AS_OUTPUT    LPC_PINCON->PINSEL0&=p5_SEL_MASK;LPC_GPIO0->FIODIR|=p5_SET_MASK
#define p5_AS_INPUT     LPC_GPIO0->FIOMASK &= p5_CLR_MASK; 
#define p5_SET          LPC_GPIO0->FIOSET = p5_SET_MASK
#define p5_CLR          LPC_GPIO0->FIOCLR = p5_SET_MASK
#define p5_IS_SET       (bool)(LPC_GPIO0->FIOPIN & p5_SET_MASK)
#define p5_IS_CLR       !(p5_IS_SET)
#define p5_MODE(x)      LPC_PINCON->PINMODE0&=p5_SEL_MASK;LPC_PINCON->PINMODE0|=((x&0x3)<<18)

/* p6 is P0.8 */
#define p6_SEL_MASK     ~(3UL << 16)
#define p6_SET_MASK     (1UL <<  8)
#define p6_CLR_MASK     ~(p6_SET_MASK)
#define p6_AS_OUTPUT    LPC_PINCON->PINSEL0&=p6_SEL_MASK;LPC_GPIO0->FIODIR|=p6-SET_MASK
#define p6_AS_INPUT     LPC_GPIO0->FIOMASK &= p6_CLR_MASK; 
#define p6_SET          LPC_GPIO0->FIOSET = p6_SET_MASK
#define p6_CLR          LPC_GPIO0->FIOCLR = p6_SET_MASK
#define p6_IS_SET       (bool)(LPC_GPIO0->FIOPIN & p6_SET_MASK)
#define p6_IS_CLR       !(p6_IS_SET)
#define p6_MODE(x)      LPC_PINCON->PINMODE0&=p6_SEL_MASK;LPC_PINCON->PINMODE0|=((x&0x3)<<16)    

/* p7 is P0.7 */
#define p7_SEL_MASK     ~(3UL << 14)
#define p7_SET_MASK     (1UL <<  7)
#define p7_CLR_MASK     ~(p7_SET_MASK)
#define p7_AS_OUTPUT    LPC_PINCON->PINSEL0&=p7_SEL_MASK;LPC_GPIO0->FIODIR|=p7_SET_MASK
#define p7_AS_INPUT     LPC_GPIO0->FIOMASK &= p7_CLR_MASK; 
#define p7_SET          LPC_GPIO0->FIOSET = p7_SET_MASK
#define p7_CLR          LPC_GPIO0->FIOCLR = p7_SET_MASK
#define p7_IS_SET       (bool)(LPC_GPIO0->FIOPIN & p7_SET_MASK)
#define p7_IS_CLR       !(p7_IS_SET)
#define p7_MODE(x)      LPC_PINCON->PINMODE0&=p7_SEL_MASK;LPC_PINCON->PINMODE0|=((x&0x3)<<14)    

/* p8 is P0.6 */
#define p8_SEL_MASK     ~(3UL << 12)
#define p8_SET_MASK     (1UL <<  6)
#define p8_CLR_MASK     ~(p8_SET_MASK)
#define p8_AS_OUTPUT    LPC_PINCON->PINSEL0&=p8_SEL_MASK;LPC_GPIO0->FIODIR|=p8_SET_MASK
#define p8_AS_INPUT     LPC_GPIO0->FIOMASK &= p8_CLR_MASK; 
#define p8_SET          LPC_GPIO0->FIOSET = p8_SET_MASK
#define p8_CLR          LPC_GPIO0->FIOCLR = p8_SET_MASK
#define p8_IS_SET       (bool)(LPC_GPIO0->FIOPIN & p8_SET_MASK)
#define p8_IS_CLR       !(p8_IS_SET)
#define p8_MODE(x)      LPC_PINCON->PINMODE0&=p8_SEL_MASK;LPC_PINCON->PINMODE0|=((x&0x3)<<12)    

/* p9 is P0.0 */
#define p9_SEL_MASK     ~(3UL <<  0)
#define p9_SET_MASK     (1UL <<  0)
#define p9_CLR_MASK     ~(p9_SET_MASK)
#define p9_AS_OUTPUT    LPC_PINCON->PINSEL0&=p9_SEL_MASK;LPC_GPIO0->FIODIR|=p9_SET_MASK
#define p9_AS_INPUT     LPC_GPIO0->FIOMASK &= p9_CLR_MASK; 
#define p9_SET          LPC_GPIO0->FIOSET = p9_SET_MASK
#define p9_CLR          LPC_GPIO0->FIOCLR = p9_SET_MASK
#define p9_IS_SET       (bool)(LPC_GPIO0->FIOPIN & p9_SET_MASK)
#define p9_IS_CLR       !(p9_IS_SET)
#define p9_MODE(x)      LPC_PINCON->PINMODE0&=p9_SEL_MASK;LPC_PINCON->PINMODE0|=((x&0x3)<<0)    

/* p10 is P0.1 */
#define p10_SEL_MASK    ~(3UL <<  2)
#define p10_SET_MASK    (1UL <<  1)
#define p10_CLR_MASK    ~(p10_SET_MASK)
#define p10_AS_OUTPUT   LPC_PINCON->PINSEL0&=p10_SEL_MASK;LPC_GPIO0->FIODIR|=p10_SET_MASK
#define p10_AS_INPUT    LPC_GPIO0->FIOMASK &= p10_CLR_MASK; 
#define p10_SET         LPC_GPIO0->FIOSET = p10_SET_MASK
#define p10_CLR         LPC_GPIO0->FIOCLR = p10_SET_MASK
#define p10_IS_SET      (bool)(LPC_GPIO0->FIOPIN & p10_SET_MASK)
#define p10_IS_CLR      !(p10_IS_SET)
#define p10_MODE(x)     LPC_PINCON->PINMODE0&=p10_SEL_MASK;LPC_PINCON->PINMODE0|=((x&0x3)<<2)

/* p11 is P0.18 */
#define p11_SEL_MASK    ~(3UL << 4)
#define p11_SET_MASK    (1UL <<  18)
#define p11_CLR_MASK    ~(p11_SET_MASK)
#define p11_AS_OUTPUT   LPC_PINCON->PINSEL1&=p11_SEL_MASK;LPC_GPIO0->FIODIR|=p11_SET_MASK
#define p11_AS_INPUT    LPC_GPIO0->FIOMASK &= p11_CLR_MASK; 
#define p11_SET         LPC_GPIO0->FIOSET = p11_SET_MASK
#define p11_CLR         LPC_GPIO0->FIOCLR = p11_SET_MASK
#define p11_IS_SET      (bool)(LPC_GPIO0->FIOPIN & p11_SET_MASK)
#define p11_IS_CLR      !(p11_IS_SET)
#define p11_MODE(x)     LPC_PINCON->PINMODE1&=p11_SEL_MASK;LPC_PINCON->PINMODE1|=((x&0x3)<<4)

/* p12 is P0.17 */
#define p12_SEL_MASK    ~(3UL << 2)
#define p12_SET_MASK    (1UL << 17)
#define p12_CLR_MASK    ~(p12_SET_MASK)
#define p12_AS_OUTPUT   LPC_PINCON->PINSEL1&=p12_SEL_MASK;LPC_GPIO0->FIODIR|=p12_SET_MASK
#define p12_AS_INPUT    LPC_GPIO0->FIOMASK &= p12_CLR_MASK; 
#define p12_SET         LPC_GPIO0->FIOSET = p12_SET_MASK
#define p12_CLR         LPC_GPIO0->FIOCLR = p12_SET_MASK
#define p12_IS_SET      (bool)(LPC_GPIO0->FIOPIN & p12_SET_MASK)
#define p12_IS_CLR      !(p12_IS_SET)
#define p12_MODE(x)     LPC_PINCON->PINMODE1&=p12_SEL_MASK;LPC_PINCON->PINMODE1|=((x&0x3)<<2)

/* p13 is P0.15 */
#define p13_SEL_MASK    ~(3UL << 30)
#define p13_SET_MASK    (1UL << 15)
#define p13_CLR_MASK    ~(p13_SET_MASK)
#define p13_AS_OUTPUT   LPC_PINCON->PINSEL0&=p13_SEL_MASK;LPC_GPIO0->FIODIR|=p13_SET_MASK
#define p13_AS_INPUT    LPC_GPIO0->FIOMASK &= p13_CLR_MASK; 
#define p13_SET         LPC_GPIO0->FIOSET = p13_SET_MASK
#define p13_CLR         LPC_GPIO0->FIOCLR = p13_SET_MASK
#define p13_IS_SET      (bool)(LPC_GPIO0->FIOPIN & p13_SET_MASK)
#define p13_IS_CLR      !(p13_IS_SET)
#define p13_MODE(x)     LPC_PINCON->PINMODE0&=p13_SEL_MASK;LPC_PINCON->PINMODE0|=((x&0x3)<<30)

/* p14 is P0.16 */
#define p14_SEL_MASK    ~(3UL << 0)
#define p14_SET_MASK    (1UL << 16)
#define p14_CLR_MASK    ~(p14_SET_MASK)
#define p14_AS_OUTPUT   LPC_PINCON->PINSEL1&=p14_SEL_MASK;LPC_GPIO0->FIODIR|=p14_SET_MASK
#define p14_AS_INPUT    LPC_GPIO0->FIOMASK &= p14_CLR_MASK; 
#define p14_SET         LPC_GPIO0->FIOSET = p14_SET_MASK
#define p14_CLR         LPC_GPIO0->FIOCLR = p14_SET_MASK
#define p14_IS_SET      (bool)(LPC_GPIO0->FIOPIN & p14_SET_MASK)
#define p14_IS_CLR      !(p14_IS_SET)
#define p14_MODE(x)     LPC_PINCON->PINMODE1&=p14_SEL_MASK;LPC_PINCON->PINMODE1|=((x&0x3)<<0)

/* p15 is P0.23 */
#define p15_SEL_MASK    ~(3UL << 14)
#define p15_SET_MASK    (1UL << 23)
#define p15_CLR_MASK    ~(p15_SET_MASK)
#define p15_AS_OUTPUT   LPC_PINCON->PINSEL1&=p15_SEL_MASK;LPC_GPIO0->FIODIR|=p15_SET_MASK
#define p15_AS_INPUT    LPC_GPIO0->FIOMASK &= p15_CLR_MASK; 
#define p15_SET         LPC_GPIO0->FIOSET = p15_SET_MASK
#define p15_CLR         LPC_GPIO0->FIOCLR = p15_SET_MASK
#define p15_IS_SET      (bool)(LPC_GPIO0->FIOPIN & p15_SET_MASK)
#define p15_IS_CLR      !(p15_IS_SET)
#define p15_MODE(x)     LPC_PINCON->PINMODE1&=p15_SEL_MASK;LPC_PINCON->PINMODE1|=((x&0x3)<<14)

/* p16 is P0.24 */
#define p16_SEL_MASK    ~(3UL << 16)
#define p16_SET_MASK    (1UL <<  24)
#define p16_CLR_MASK    ~(p16_SET_MASK)
#define p16_AS_OUTPUT   LPC_PINCON->PINSEL1&=p16_SEL_MASK;LPC_GPIO0->FIODIR|=p16_SET_MASK
#define p16_AS_INPUT    LPC_GPIO0->FIOMASK &= p16_CLR_MASK; 
#define p16_SET         LPC_GPIO0->FIOSET = p16_SET_MASK
#define p16_CLR         LPC_GPIO0->FIOCLR = p16_SET_MASK
#define p16_IS_SET      (bool)(LPC_GPIO0->FIOPIN & p16_SET_MASK)
#define p16_IS_CLR      !(p16_IS_SET)
#define p16_MODE(x)     LPC_PINCON->PINMODE1&=p16_SEL_MASK;LPC_PINCON->PINMODE1|=((x&0x3)<<16)

/* p17 is P0.25 */
#define p17_SEL_MASK    ~(3UL <<  18)
#define p17_SET_MASK    (1UL <<  25)
#define p17_CLR_MASK    ~(p17_SET_MASK)
#define p17_AS_OUTPUT   LPC_PINCON->PINSEL1&=p17_SEL_MASK;LPC_GPIO0->FIODIR|=p17_SET_MASK
#define p17_AS_INPUT    LPC_GPIO0->FIOMASK &= p17_CLR_MASK; 
#define p17_SET         LPC_GPIO0->FIOSET = p17_SET_MASK
#define p17_CLR         LPC_GPIO0->FIOCLR = p17_SET_MASK
#define p17_IS_SET      (bool)(LPC_GPIO0->FIOPIN & p17_SET_MASK)
#define p17_IS_CLR      !(p17_IS_SET)
#define p17_MODE(x)     LPC_PINCON->PINMODE1&=p17_SEL_MASK;LPC_PINCON->PINMODE1|=((x&0x3)<<18)

/* p18 is P0.26 */
#define p18_SEL_MASK    ~(3UL << 20)
#define p18_SET_MASK    (1UL << 26)
#define p18_CLR_MASK    ~(p18_SET_MASK)
#define p18_AS_OUTPUT   LPC_PINCON->PINSEL1&=p18_SEL_MASK;LPC_GPIO0->FIODIR|=p18_SET_MASK
#define p18_AS_INPUT    LPC_GPIO0->FIOMASK &= p18_CLR_MASK; 
#define p18_SET         LPC_GPIO0->FIOSET = p18_SET_MASK
#define p18_CLR         LPC_GPIO0->FIOCLR = p18_SET_MASK
#define p18_IS_SET      (bool)(LPC_GPIO0->FIOPIN & p18_SET_MASK)
#define p18_IS_CLR      !(p18_IS_SET)
#define p18_MODE(x)     LPC_PINCON->PINMODE1&=p18_SEL_MASK;LPC_PINCON->PINMODE1|=((x&0x3)<<20)

/* p19 is P1.30 */
#define p19_SEL_MASK    ~(3UL << 28)
#define p19_SET_MASK    (1UL << 30)
#define p19_AS_OUTPUT   LPC_PINCON->PINSEL3&=p19_SEL_MASK;LPC_GPIO1->FIODIR|=p19_SET_MASK
#define p19_AS_INPUT    LPC_GPIO1->FIOMASK &= p19_CLR_MASK; 
#define p19_SET         LPC_GPIO1->FIOSET = p19_SET_MASK
#define p19_CLR         LPC_GPIO1->FIOCLR = p19_SET_MASK
#define p19_IS_SET      (bool)(LPC_GPIO1->FIOPIN & p19_SET_MASK)
#define p19_IS_CLR      !(p19_IS_SET)
#define p19_MODE(x)     LPC_PINCON->PINMODE3&=p19_SEL_MASK;LPC_PINCON->PINMODE3|=((x&0x3)<<28)

/* p20 is P1.31 */
#define p20_SEL_MASK    ~(3UL << 30)
#define p20_SET_MASK    (1UL << 31)
#define p20_CLR_MASK    ~(p20_SET_MASK)
#define p20_AS_OUTPUT   LPC_PINCON->PINSEL3&=p20_SEL_MASK;LPC_GPIO1->FIODIR|=p20_SET_MASK
#define p20_AS_INPUT    LPC_GPIO1->FIOMASK &= p20_CLR_MASK; 
#define p20_SET         LPC_GPIO1->FIOSET = p20_SET_MASK
#define p20_CLR         LPC_GPIO1->FIOCLR = p20_SET_MASK
#define p20_IS_SET      (bool)(LPC_GPIO1->FIOPIN & p20_SET_MASK)
#define p20_IS_CLR      !(p20_IS_SET)
#define p20_MODE(x)     LPC_PINCON->PINMODE3&=p20_SEL_MASK;LPC_PINCON->PINMODE3|=((x&0x3)<<30)

/* p21 is P2.5 */
#define p21_SEL_MASK    ~(3UL << 10)
#define p21_SET_MASK    (1UL << 5)
#define p21_CLR_MASK    ~(p21_SET_MASK)
#define p21_AS_OUTPUT   LPC_PINCON->PINSEL4&=p21_SEL_MASK;LPC_GPIO2->FIODIR|=p21_SET_MASK
#define p21_AS_INPUT    LPC_GPIO2->FIOMASK &= p21_CLR_MASK; 
#define p21_SET         LPC_GPIO2->FIOSET = p21_SET_MASK
#define p21_CLR         LPC_GPIO2->FIOCLR = p21_SET_MASK
#define p21_IS_SET      (bool)(LPC_GPIO2->FIOPIN & p21_SET_MASK)
#define p21_IS_CLR      !(p21_IS_SET)
#define p21_TOGGLE      p21_IS_SET?p21_CLR:p21_SET
#define p21_MODE(x)     LPC_PINCON->PINMODE4&=p21_SEL_MASK;LPC_PINCON->PINMODE4|=((x&0x3)<<10)

/* p22 is P2.4 */
#define p22_SEL_MASK    ~(3UL << 8)
#define p22_SET_MASK    (1UL << 4)
#define p22_CLR_MASK    ~(p22_SET_MASK)
#define p22_AS_OUTPUT   LPC_PINCON->PINSEL4&=p22_SEL_MASK;LPC_GPIO2->FIODIR|=p22_SET_MASK
#define p22_AS_INPUT    LPC_GPIO2->FIOMASK &= p22_CLR_MASK; 
#define p22_SET         LPC_GPIO2->FIOSET = p22_SET_MASK
#define p22_CLR         LPC_GPIO2->FIOCLR = p22_SET_MASK
#define p22_IS_SET      (bool)(LPC_GPIO2->FIOPIN & p22_SET_MASK)
#define p22_IS_CLR      !(p22_IS_SET)
#define p22_TOGGLE      p22_IS_SET?p22_CLR:p22_SET
#define p22_MODE(x)     LPC_PINCON->PINMODE4&=p22_SEL_MASK;LPC_PINCON->PINMODE4|=((x&0x3)<<8)

/* p23 is P2.3 */
#define p23_SEL_MASK    ~(3UL << 6)
#define p23_SET_MASK    (1UL << 3)
#define p23_CLR_MASK    ~(p23_SET_MASK)
#define p23_AS_OUTPUT   LPC_PINCON->PINSEL4&=p23_SEL_MASK;LPC_GPIO2->FIODIR|=p23_SET_MASK
#define p23_AS_INPUT    LPC_GPIO2->FIOMASK &= p23_CLR_MASK; 
#define p23_SET         LPC_GPIO2->FIOSET = p23_SET_MASK
#define p23_CLR         LPC_GPIO2->FIOCLR = p23_SET_MASK
#define p23_IS_SET      (bool)(LPC_GPIO2->FIOPIN & p23_SET_MASK)
#define p23_IS_CLR      !(p23_IS_SET)
#define p23_TOGGLE      p23_IS_SET?p23_CLR:p23_SET
#define p23_MODE(x)     LPC_PINCON->PINMODE4&=p23_SEL_MASK;LPC_PINCON->PINMODE4|=((x&0x3)<<6)

/* p24 is P2.2 */
#define p24_SEL_MASK    ~(3UL << 4)
#define p24_SET_MASK    (1UL << 2)
#define p24_CLR_MASK    ~(p24_SET_MASK)
#define p24_AS_OUTPUT   LPC_PINCON->PINSEL4&=p24_SEL_MASK;LPC_GPIO2->FIODIR|=p24_SET_MASK
#define p24_AS_INPUT    LPC_GPIO2->FIOMASK &= p24_CLR_MASK; 
#define p24_SET         LPC_GPIO2->FIOSET = p24_SET_MASK
#define p24_CLR         LPC_GPIO2->FIOCLR = p24_SET_MASK
#define p24_IS_SET      (bool)(LPC_GPIO2->FIOPIN & p24_SET_MASK)
#define p24_IS_CLR      !(p24_IS_SET)
#define p24_TOGGLE      p24_IS_SET?p24_CLR:p24_SET
#define p24_MODE(x)     LPC_PINCON->PINMODE4&=p24_SEL_MASK;LPC_PINCON->PINMODE4|=((x&0x3)<<4)

/* p25 is P2.1 */
#define p25_SEL_MASK    ~(3UL << 2)
#define p25_SET_MASK    (1UL << 1)
#define p25_CLR_MASK    ~(p25_SET_MASK)
#define p25_AS_OUTPUT   LPC_PINCON->PINSEL4&=p25_SEL_MASK;LPC_GPIO2->FIODIR|=p25_SET_MASK
#define p25_AS_INPUT    LPC_GPIO2->FIOMASK &= p25_CLR_MASK; 
#define p25_SET         LPC_GPIO2->FIOSET = p25_SET_MASK
#define p25_CLR         LPC_GPIO2->FIOCLR = p25_SET_MASK
#define p25_IS_SET      (bool)(LPC_GPIO2->FIOPIN & p25_SET_MASK)
#define p25_IS_CLR      !(p25_IS_SET)
#define p25_MODE(x)     LPC_PINCON->PINMODE4&=p25_SEL_MASK;LPC_PINCON->PINMODE4|=((x&0x3)<<2)

/* p26 is P2.0 */
#define p26_SEL_MASK    ~(3UL << 0)
#define p26_SET_MASK    (1UL << 0)
#define p26_CLR_MASK    ~(p26_SET_MASK)
#define p26_AS_OUTPUT   LPC_PINCON->PINSEL4&=p26_SEL_MASK;LPC_GPIO2->FIODIR|=p26_SET_MASK
#define p26_AS_INPUT    LPC_GPIO2->FIOMASK &= p26_CLR_MASK; 
#define p26_SET         LPC_GPIO2->FIOSET = p26_SET_MASK
#define p26_CLR         LPC_GPIO2->FIOCLR = p26_SET_MASK
#define p26_IS_SET      (bool)(LPC_GPIO2->FIOPIN & p26_SET_MASK)
#define p26_IS_CLR      !(p26_IS_SET)
#define p26_MODE(x)     LPC_PINCON->PINMODE4&=p26_SEL_MASK;LPC_PINCON->PINMODE4|=((x&0x3)<<0)

/* p27 is P0.11 */
#define p27_SEL_MASK    ~(3UL << 22)
#define p27_SET_MASK    (1UL << 11)
#define p27_CLR_MASK    ~(p27_SET_MASK)
#define p27_AS_OUTPUT   LPC_PINCON->PINSEL0&=p27_SEL_MASK;LPC_GPIO0->FIODIR|=p27_SET_MASK
#define p27_AS_INPUT    LPC_GPIO0->FIOMASK &= p27_CLR_MASK; 
#define p27_SET         LPC_GPIO0->FIOSET = p27_SET_MASK
#define p27_CLR         LPC_GPIO0->FIOCLR = p27_SET_MASK
#define p27_IS_SET      (bool)(LPC_GPIO0->FIOPIN & p27_SET_MASK)
#define p27_IS_CLR      !(p27_IS_SET)
#define p27_MODE(x)     LPC_PINCON->PINMODE0&=p27_SEL_MASK;LPC_PINCON->PINMODE0|=((x&0x3)<<22)

/* p28 is P0.10 */
#define p28_SEL_MASK    ~(3UL << 20)
#define p28_SET_MASK    (1UL <<  10)
#define p28_CLR_MASK    ~(p28_SET_MASK)
#define p28_AS_OUTPUT   LPC_PINCON->PINSEL0&=p28_SEL_MASK;LPC_GPIO0->FIODIR|=p28_SET_MASK
#define p28_AS_INPUT    LPC_GPIO0->FIOMASK &= p28_CLR_MASK; 
#define p28_SET         LPC_GPIO0->FIOSET = p28_SET_MASK
#define p28_CLR         LPC_GPIO0->FIOCLR = p28_SET_MASK
#define p28_IS_SET      (bool)(LPC_GPIO0->FIOPIN & p28_SET_MASK)
#define p28_IS_CLR      !(p28_IS_SET)
#define p28_MODE(x)     LPC_PINCON->PINMODE0&=p28_SEL_MASK;LPC_PINCON->PINMODE0|=((x&0x3)<<20)

/* p29 is P0.5 */
#define p29_SEL_MASK    ~(3UL << 10)
#define p29_SET_MASK    (1UL << 5)
#define p29_CLR_MASK    ~(p29_SET_MASK)
#define p29_AS_OUTPUT   LPC_PINCON->PINSEL0&=p29_SEL_MASK;LPC_GPIO0->FIODIR|=p29_SET_MASK
#define p29_AS_INPUT    LPC_GPIO0->FIOMASK &= p29_CLR_MASK; 
#define p29_SET         LPC_GPIO0->FIOSET = p29_SET_MASK
#define p29_CLR         LPC_GPIO0->FIOCLR = p29_SET_MASK
#define p29_IS_SET      (bool)(LPC_GPIO0->FIOPIN & p29_SET_MASK)
#define p29_IS_CLR      !(p29_IS_SET)
#define p29_TOGGLE      p29_IS_SET?p29_CLR:p29_SET
#define p29_MODE(x)     LPC_PINCON->PINMODE0&=p29_SEL_MASK;LPC_PINCON->PINMODE0|=((x&0x3)<<10)

/* p30 is P0.4 */
#define p30_SEL_MASK    ~(3UL << 8)
#define p30_SET_MASK    (1UL << 4)
#define p30_CLR_MASK    ~(p30_SET_MASK)
#define p30_AS_OUTPUT   LPC_PINCON->PINSEL0&=p30_SEL_MASK;LPC_GPIO0->FIODIR|=p30_SET_MASK
#define p30_AS_INPUT    LPC_GPIO0->FIOMASK &= p30_CLR_MASK; 
#define p30_SET         LPC_GPIO0->FIOSET = p30_SET_MASK
#define p30_CLR         LPC_GPIO0->FIOCLR = p30_SET_MASK
#define p30_IS_SET      (bool)(LPC_GPIO0->FIOPIN & p30_SET_MASK)
#define p30_IS_CLR      !(p30_IS_SET)
#define p30_MODE(x)     LPC_PINCON->PINMODE0&=p30_SEL_MASK;LPC_PINCON->PINMODE0|=((x&0x3)<<8)

/* The following definitions are for the four Mbed LEDs.
    LED1 = P1.18
    LED2 = P1.20
    LED3 = P1.21
    LED4 = P1.23 */

#define P1_18_SEL_MASK  ~(3UL << 4)
#define P1_18_SET_MASK  (1UL << 18)
#define P1_18_CLR_MASK  ~(P1_18_SET_MASK)
#define P1_18_AS_OUTPUT LPC_PINCON->PINSEL3&=P1_18_SEL_MASK;LPC_GPIO1->FIODIR|=P1_18_SET_MASK
#define P1_18_AS_INPUT  LPC_GPIO1->FIOMASK &= P1_18_CLR_MASK; 
#define P1_18_SET       LPC_GPIO1->FIOSET = P1_18_SET_MASK
#define P1_18_CLR       LPC_GPIO1->FIOCLR = P1_18_SET_MASK
#define P1_18_IS_SET    (bool)(LPC_GPIO1->FIOPIN & P1_18_SET_MASK)
#define P1_18_IS_CLR    !(P1_18_IS_SET)
#define LED1_USE        P1_18_AS_OUTPUT;P1_18_AS_INPUT
#define LED1_ON         P1_18_SET
#define LED1_OFF        P1_18_CLR
#define LED1_IS_ON      P1_18_IS_SET
#define LED1_TOGGLE     P1_18_IS_SET?LED1_OFF:LED1_ON

#define P1_20_SEL_MASK  ~(3UL << 8)
#define P1_20_SET_MASK  (1UL << 20)
#define P1_20_CLR_MASK  ~(P1_20_SET_MASK)
#define P1_20_AS_OUTPUT LPC_PINCON->PINSEL3&=P1_20_SEL_MASK;LPC_GPIO1->FIODIR|=P1_20_SET_MASK
#define P1_20_AS_INPUT  LPC_GPIO1->FIOMASK &= P1_20_CLR_MASK; 
#define P1_20_SET       LPC_GPIO1->FIOSET = P1_20_SET_MASK
#define P1_20_CLR       LPC_GPIO1->FIOCLR = P1_20_SET_MASK
#define P1_20_IS_SET    (bool)(LPC_GPIO1->FIOPIN & P1_20_SET_MASK)
#define P1_20_IS_CLR    !(P1_20_IS_SET)    
#define LED2_USE        P1_20_AS_OUTPUT;P1_20_AS_INPUT
#define LED2_ON         P1_20_SET
#define LED2_OFF        P1_20_CLR
#define LED2_IS_ON      P1_20_IS_SET
#define LED2_TOGGLE     P1_20_IS_SET?LED2_OFF:LED2_ON

#define P1_21_SEL_MASK  ~(3UL << 10)
#define P1_21_SET_MASK  (1UL << 21)
#define P1_21_CLR_MASK  ~(P1_21_SET_MASK)
#define P1_21_AS_OUTPUT LPC_PINCON->PINSEL3&=P1_21_SEL_MASK;LPC_GPIO1->FIODIR|=P1_21_SET_MASK
#define P1_21_AS_INPUT  LPC_GPIO1->FIOMASK &= P1_21_CLR_MASK; 
#define P1_21_SET       LPC_GPIO1->FIOSET = P1_21_SET_MASK
#define P1_21_CLR       LPC_GPIO1->FIOCLR = P1_21_SET_MASK
#define P1_21_IS_SET    (bool)(LPC_GPIO1->FIOPIN & P1_21_SET_MASK)
#define P1_21_IS_CLR    !(P1_21_IS_SET)
#define LED3_USE        P1_21_AS_OUTPUT;P1_21_AS_INPUT
#define LED3_ON         P1_21_SET
#define LED3_OFF        P1_21_CLR
#define LED3_IS_ON      P1_21_IS_SET
#define LED3_TOGGLE     P1_21_IS_SET?LED3_OFF:LED3_ON

#define P1_23_SEL_MASK  ~(3UL << 14)
#define P1_23_SET_MASK  (1UL << 23)
#define P1_23_CLR_MASK  ~(P1_23_SET_MASK)
#define P1_23_AS_OUTPUT LPC_PINCON->PINSEL3&=P1_23_SEL_MASK;LPC_GPIO1->FIODIR|=P1_23_SET_MASK
#define P1_23_AS_INPUT  LPC_GPIO1->FIOMASK &= P1_23_CLR_MASK; 
#define P1_23_SET       LPC_GPIO1->FIOSET = P1_23_SET_MASK
#define P1_23_CLR       LPC_GPIO1->FIOCLR = P1_23_SET_MASK
#define P1_23_IS_SET    (bool)(LPC_GPIO1->FIOPIN & P1_23_SET_MASK)
#define P1_23_IS_CLR    !(P1_23_IS_SET)    
#define LED4_USE        P1_23_AS_OUTPUT;P1_23_AS_INPUT
#define LED4_ON         P1_23_SET
#define LED4_OFF        P1_23_CLR
#define LED4_IS_ON      P1_23_IS_SET
#define LED4_TOGGLE     P1_23_IS_SET?LED4_OFF:LED4_ON

#endif

