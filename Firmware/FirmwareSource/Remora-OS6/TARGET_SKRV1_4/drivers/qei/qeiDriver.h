#ifndef QEIDRIVER_H
#define QEIDRIVER_H

#include "mbed.h"

#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <string>

class qeiInterrupt; // forward declatation

class QEIdriver
{
    	friend class    qeiInterrupt;
    
    private:

        qeiInterrupt* 	interruptPtr;
        IRQn_Type 		irq;

        int             dirinv;                 // Direction invert. When = 1, complements the QEICONF register DIR bit
        int             sigmode;                // Signal mode. When = 0, PhA and PhB are quadrature inputs. When = 1, PhA is direction and PhB is clock
        int             capmode;                // Capture mode. When = 0, count PhA edges only (2X mode). Whe = 1, count PhB edges also (4X mode)
        int             invinx;                 // Invert index. When = 1, inverts the sense of the index signal

        void interruptHandler();

    public:

        bool             hasIndex;
        bool             indexDetected;
        int32_t          indexCount;

        QEIdriver();            // for channel A & B
        QEIdriver(bool);        // For channels A & B, and index

        void init(void);
        uint32_t get(void);



/* Private Macros ------------------------------------------------------------- */
/* --------------------- BIT DEFINITIONS -------------------------------------- */
/* Quadrature Encoder Interface Control Register Definition --------------------- */
/*********************************************************************//**
 * Macro defines for QEI Control register
 **********************************************************************/
#define QEI_CON_RESP        ((uint32_t)(1<<0))        /**< Reset position counter */
#define QEI_CON_RESPI        ((uint32_t)(1<<1))        /**< Reset Posistion Counter on Index */
#define QEI_CON_RESV        ((uint32_t)(1<<2))        /**< Reset Velocity */
#define QEI_CON_RESI        ((uint32_t)(1<<3))        /**< Reset Index Counter */
#define QEI_CON_BITMASK        ((uint32_t)(0x0F))        /**< QEI Control register bit-mask */

/*********************************************************************//**
 * Macro defines for QEI Configuration register
 **********************************************************************/
#define QEI_CONF_DIRINV        ((uint32_t)(1<<0))        /**< Direction Invert */
#define QEI_CONF_SIGMODE    ((uint32_t)(1<<1))        /**< Signal mode */
#define QEI_CONF_CAPMODE    ((uint32_t)(1<<2))        /**< Capture mode */
#define QEI_CONF_INVINX        ((uint32_t)(1<<3))        /**< Invert index */
#define QEI_CONF_BITMASK    ((uint32_t)(0x0F))        /**< QEI Configuration register bit-mask */

/*********************************************************************//**
 * Macro defines for QEI Status register
 **********************************************************************/
#define QEI_STAT_DIR        ((uint32_t)(1<<0))        /**< Direction bit */
#define QEI_STAT_BITMASK    ((uint32_t)(1<<0))        /**< QEI status register bit-mask */

/* Quadrature Encoder Interface Interrupt registers definitions --------------------- */
/*********************************************************************//**
 * Macro defines for QEI Interrupt Status register
 **********************************************************************/
#define QEI_INTSTAT_INX_Int            ((uint32_t)(1<<0))    /**< Indicates that an index pulse was detected */
#define QEI_INTSTAT_TIM_Int            ((uint32_t)(1<<1))    /**< Indicates that a velocity timer overflow occurred */
#define QEI_INTSTAT_VELC_Int        ((uint32_t)(1<<2))    /**< Indicates that capture velocity is less than compare velocity */
#define QEI_INTSTAT_DIR_Int            ((uint32_t)(1<<3))    /**< Indicates that a change of direction was detected */
#define QEI_INTSTAT_ERR_Int            ((uint32_t)(1<<4))    /**< Indicates that an encoder phase error was detected */
#define QEI_INTSTAT_ENCLK_Int        ((uint32_t)(1<<5))    /**< Indicates that and encoder clock pulse was detected */
#define QEI_INTSTAT_POS0_Int        ((uint32_t)(1<<6))    /**< Indicates that the position 0 compare value is equal to the
                                                        current position */
#define QEI_INTSTAT_POS1_Int        ((uint32_t)(1<<7))    /**< Indicates that the position 1compare value is equal to the
                                                        current position */
#define QEI_INTSTAT_POS2_Int        ((uint32_t)(1<<8))    /**< Indicates that the position 2 compare value is equal to the
                                                        current position */
#define QEI_INTSTAT_REV_Int            ((uint32_t)(1<<9))    /**< Indicates that the index compare value is equal to the current
                                                        index count */
#define QEI_INTSTAT_POS0REV_Int        ((uint32_t)(1<<10))    /**< Combined position 0 and revolution count interrupt. Set when
                                                        both the POS0_Int bit is set and the REV_Int is set */
#define QEI_INTSTAT_POS1REV_Int        ((uint32_t)(1<<11))    /**< Combined position 1 and revolution count interrupt. Set when
                                                        both the POS1_Int bit is set and the REV_Int is set */
#define QEI_INTSTAT_POS2REV_Int        ((uint32_t)(1<<12))    /**< Combined position 2 and revolution count interrupt. Set when
                                                        both the POS2_Int bit is set and the REV_Int is set */
#define QEI_INTSTAT_BITMASK            ((uint32_t)(0x1FFF))    /**< QEI Interrupt Status register bit-mask */

/*********************************************************************//**
 * Macro defines for QEI Interrupt Set register
 **********************************************************************/
#define QEI_INTSET_INX_Int            ((uint32_t)(1<<0))    /**< Set Bit Indicates that an index pulse was detected */
#define QEI_INTSET_TIM_Int            ((uint32_t)(1<<1))    /**< Set Bit Indicates that a velocity timer overflow occurred */
#define QEI_INTSET_VELC_Int            ((uint32_t)(1<<2))    /**< Set Bit Indicates that capture velocity is less than compare velocity */
#define QEI_INTSET_DIR_Int            ((uint32_t)(1<<3))    /**< Set Bit Indicates that a change of direction was detected */
#define QEI_INTSET_ERR_Int            ((uint32_t)(1<<4))    /**< Set Bit Indicates that an encoder phase error was detected */
#define QEI_INTSET_ENCLK_Int        ((uint32_t)(1<<5))    /**< Set Bit Indicates that and encoder clock pulse was detected */
#define QEI_INTSET_POS0_Int            ((uint32_t)(1<<6))    /**< Set Bit Indicates that the position 0 compare value is equal to the
                                                        current position */
#define QEI_INTSET_POS1_Int            ((uint32_t)(1<<7))    /**< Set Bit Indicates that the position 1compare value is equal to the
                                                        current position */
#define QEI_INTSET_POS2_Int            ((uint32_t)(1<<8))    /**< Set Bit Indicates that the position 2 compare value is equal to the
                                                        current position */
#define QEI_INTSET_REV_Int            ((uint32_t)(1<<9))    /**< Set Bit Indicates that the index compare value is equal to the current
                                                        index count */
#define QEI_INTSET_POS0REV_Int        ((uint32_t)(1<<10))    /**< Set Bit that combined position 0 and revolution count interrupt */
#define QEI_INTSET_POS1REV_Int        ((uint32_t)(1<<11))    /**< Set Bit that Combined position 1 and revolution count interrupt */
#define QEI_INTSET_POS2REV_Int        ((uint32_t)(1<<12))    /**< Set Bit that Combined position 2 and revolution count interrupt */
#define QEI_INTSET_BITMASK            ((uint32_t)(0x1FFF))    /**< QEI Interrupt Set register bit-mask */

/*********************************************************************//**
 * Macro defines for QEI Interrupt Clear register
 **********************************************************************/
#define QEI_INTCLR_INX_Int            ((uint32_t)(1<<0))    /**< Clear Bit Indicates that an index pulse was detected */
#define QEI_INTCLR_TIM_Int            ((uint32_t)(1<<1))    /**< Clear Bit Indicates that a velocity timer overflow occurred */
#define QEI_INTCLR_VELC_Int            ((uint32_t)(1<<2))    /**< Clear Bit Indicates that capture velocity is less than compare velocity */
#define QEI_INTCLR_DIR_Int            ((uint32_t)(1<<3))    /**< Clear Bit Indicates that a change of direction was detected */
#define QEI_INTCLR_ERR_Int            ((uint32_t)(1<<4))    /**< Clear Bit Indicates that an encoder phase error was detected */
#define QEI_INTCLR_ENCLK_Int        ((uint32_t)(1<<5))    /**< Clear Bit Indicates that and encoder clock pulse was detected */
#define QEI_INTCLR_POS0_Int            ((uint32_t)(1<<6))    /**< Clear Bit Indicates that the position 0 compare value is equal to the
                                                        current position */
#define QEI_INTCLR_POS1_Int            ((uint32_t)(1<<7))    /**< Clear Bit Indicates that the position 1compare value is equal to the
                                                        current position */
#define QEI_INTCLR_POS2_Int            ((uint32_t)(1<<8))    /**< Clear Bit Indicates that the position 2 compare value is equal to the
                                                        current position */
#define QEI_INTCLR_REV_Int            ((uint32_t)(1<<9))    /**< Clear Bit Indicates that the index compare value is equal to the current
                                                        index count */
#define QEI_INTCLR_POS0REV_Int        ((uint32_t)(1<<10))    /**< Clear Bit that combined position 0 and revolution count interrupt */
#define QEI_INTCLR_POS1REV_Int        ((uint32_t)(1<<11))    /**< Clear Bit that Combined position 1 and revolution count interrupt */
#define QEI_INTCLR_POS2REV_Int        ((uint32_t)(1<<12))    /**< Clear Bit that Combined position 2 and revolution count interrupt */
#define QEI_INTCLR_BITMASK            ((uint32_t)(0x1FFF))    /**< QEI Interrupt Clear register bit-mask */

/*********************************************************************//**
 * Macro defines for QEI Interrupt Enable register
 **********************************************************************/
#define QEI_INTEN_INX_Int            ((uint32_t)(1<<0))    /**< Enabled Interrupt Bit Indicates that an index pulse was detected */
#define QEI_INTEN_TIM_Int            ((uint32_t)(1<<1))    /**< Enabled Interrupt Bit Indicates that a velocity timer overflow occurred */
#define QEI_INTEN_VELC_Int            ((uint32_t)(1<<2))    /**< Enabled Interrupt Bit Indicates that capture velocity is less than compare velocity */
#define QEI_INTEN_DIR_Int            ((uint32_t)(1<<3))    /**< Enabled Interrupt Bit Indicates that a change of direction was detected */
#define QEI_INTEN_ERR_Int            ((uint32_t)(1<<4))    /**< Enabled Interrupt Bit Indicates that an encoder phase error was detected */
#define QEI_INTEN_ENCLK_Int            ((uint32_t)(1<<5))    /**< Enabled Interrupt Bit Indicates that and encoder clock pulse was detected */
#define QEI_INTEN_POS0_Int            ((uint32_t)(1<<6))    /**< Enabled Interrupt Bit Indicates that the position 0 compare value is equal to the
                                                        current position */
#define QEI_INTEN_POS1_Int            ((uint32_t)(1<<7))    /**< Enabled Interrupt Bit Indicates that the position 1compare value is equal to the
                                                        current position */
#define QEI_INTEN_POS2_Int            ((uint32_t)(1<<8))    /**< Enabled Interrupt Bit Indicates that the position 2 compare value is equal to the
                                                        current position */
#define QEI_INTEN_REV_Int            ((uint32_t)(1<<9))    /**< Enabled Interrupt Bit Indicates that the index compare value is equal to the current
                                                        index count */
#define QEI_INTEN_POS0REV_Int        ((uint32_t)(1<<10))    /**< Enabled Interrupt Bit that combined position 0 and revolution count interrupt */
#define QEI_INTEN_POS1REV_Int        ((uint32_t)(1<<11))    /**< Enabled Interrupt Bit that Combined position 1 and revolution count interrupt */
#define QEI_INTEN_POS2REV_Int        ((uint32_t)(1<<12))    /**< Enabled Interrupt Bit that Combined position 2 and revolution count interrupt */
#define QEI_INTEN_BITMASK            ((uint32_t)(0x1FFF))    /**< QEI Interrupt Enable register bit-mask */

/*********************************************************************//**
 * Macro defines for QEI Interrupt Enable Set register
 **********************************************************************/
#define QEI_IESET_INX_Int            ((uint32_t)(1<<0))    /**< Set Enable Interrupt Bit Indicates that an index pulse was detected */
#define QEI_IESET_TIM_Int            ((uint32_t)(1<<1))    /**< Set Enable Interrupt Bit Indicates that a velocity timer overflow occurred */
#define QEI_IESET_VELC_Int            ((uint32_t)(1<<2))    /**< Set Enable Interrupt Bit Indicates that capture velocity is less than compare velocity */
#define QEI_IESET_DIR_Int            ((uint32_t)(1<<3))    /**< Set Enable Interrupt Bit Indicates that a change of direction was detected */
#define QEI_IESET_ERR_Int            ((uint32_t)(1<<4))    /**< Set Enable Interrupt Bit Indicates that an encoder phase error was detected */
#define QEI_IESET_ENCLK_Int            ((uint32_t)(1<<5))    /**< Set Enable Interrupt Bit Indicates that and encoder clock pulse was detected */
#define QEI_IESET_POS0_Int            ((uint32_t)(1<<6))    /**< Set Enable Interrupt Bit Indicates that the position 0 compare value is equal to the
                                                        current position */
#define QEI_IESET_POS1_Int            ((uint32_t)(1<<7))    /**< Set Enable Interrupt Bit Indicates that the position 1compare value is equal to the
                                                        current position */
#define QEI_IESET_POS2_Int            ((uint32_t)(1<<8))    /**< Set Enable Interrupt Bit Indicates that the position 2 compare value is equal to the
                                                        current position */
#define QEI_IESET_REV_Int            ((uint32_t)(1<<9))    /**< Set Enable Interrupt Bit Indicates that the index compare value is equal to the current
                                                        index count */
#define QEI_IESET_POS0REV_Int        ((uint32_t)(1<<10))    /**< Set Enable Interrupt Bit that combined position 0 and revolution count interrupt */
#define QEI_IESET_POS1REV_Int        ((uint32_t)(1<<11))    /**< Set Enable Interrupt Bit that Combined position 1 and revolution count interrupt */
#define QEI_IESET_POS2REV_Int        ((uint32_t)(1<<12))    /**< Set Enable Interrupt Bit that Combined position 2 and revolution count interrupt */
#define QEI_IESET_BITMASK            ((uint32_t)(0x1FFF))    /**< QEI Interrupt Enable Set register bit-mask */

/*********************************************************************//**
 * Macro defines for QEI Interrupt Enable Clear register
 **********************************************************************/
#define QEI_IECLR_INX_Int            ((uint32_t)(1<<0))    /**< Clear Enabled Interrupt Bit Indicates that an index pulse was detected */
#define QEI_IECLR_TIM_Int            ((uint32_t)(1<<1))    /**< Clear Enabled Interrupt Bit Indicates that a velocity timer overflow occurred */
#define QEI_IECLR_VELC_Int            ((uint32_t)(1<<2))    /**< Clear Enabled Interrupt Bit Indicates that capture velocity is less than compare velocity */
#define QEI_IECLR_DIR_Int            ((uint32_t)(1<<3))    /**< Clear Enabled Interrupt Bit Indicates that a change of direction was detected */
#define QEI_IECLR_ERR_Int            ((uint32_t)(1<<4))    /**< Clear Enabled Interrupt Bit Indicates that an encoder phase error was detected */
#define QEI_IECLR_ENCLK_Int            ((uint32_t)(1<<5))    /**< Clear Enabled Interrupt Bit Indicates that and encoder clock pulse was detected */
#define QEI_IECLR_POS0_Int            ((uint32_t)(1<<6))    /**< Clear Enabled Interrupt Bit Indicates that the position 0 compare value is equal to the
                                                        current position */
#define QEI_IECLR_POS1_Int            ((uint32_t)(1<<7))    /**< Clear Enabled Interrupt Bit Indicates that the position 1compare value is equal to the
                                                        current position */
#define QEI_IECLR_POS2_Int            ((uint32_t)(1<<8))    /**< Clear Enabled Interrupt Bit Indicates that the position 2 compare value is equal to the
                                                        current position */
#define QEI_IECLR_REV_Int            ((uint32_t)(1<<9))    /**< Clear Enabled Interrupt Bit Indicates that the index compare value is equal to the current
                                                        index count */
#define QEI_IECLR_POS0REV_Int        ((uint32_t)(1<<10))    /**< Clear Enabled Interrupt Bit that combined position 0 and revolution count interrupt */
#define QEI_IECLR_POS1REV_Int        ((uint32_t)(1<<11))    /**< Clear Enabled Interrupt Bit that Combined position 1 and revolution count interrupt */
#define QEI_IECLR_POS2REV_Int        ((uint32_t)(1<<12))    /**< Clear Enabled Interrupt Bit that Combined position 2 and revolution count interrupt */
#define QEI_IECLR_BITMASK            ((uint32_t)(0x1FFF))    /**< QEI Interrupt Enable Clear register bit-mask */

/*********************************************************************//**
 * Macro defines for PCONP register QEI-related bits
 **********************************************************************/
#define PCONP_QEI_ENABLE             ((uint32_t)(1<<18))     /**< QEI peripheral power enable bit */
#define PCONP_QEI_DISABLE            ~((uint32_t)(1<<18))     /**< QEI peripheral power disable bit-mask */

/*********************************************************************//**
 * Macro defines for PCLKSELx register QEI-related bits
 **********************************************************************/
#define PCLKSEL_CCLK_DIV_1              1UL                 /**< Set PCLK to CCLK/1 */
#define PCLKSEL_CCLK_DIV_2              2UL                 /**< Set PCLK to CCLK/2 */
#define PCLKSEL_CCLK_DIV_4              0UL                 /**< Set PCLK to CCLK/4 */
#define PCLKSEL_CCLK_DIV_8              3UL                 /**< Set PCLK to CCLK/8 */
#define PCLKSEL1_PCLK_QEI_MASK          ((uint32_t)(3<<0))  /**< PCLK_QEI PCLK_QEI bit field mask */
/*********************************************************************//**
 * Macro defines for PINSEL3 register QEI-related bits
 **********************************************************************/
#define PINSEL3_MCI0                ((uint32_t)(1<<8))     /**< MCIO (PhA) pin select */
#define PINSEL3_MCI0_MASK          ~((uint32_t)(3<<8))     /**< MCIO (PhA) pin mask */
#define PINSEL3_MCI1                ((uint32_t)(1<<14))    /**< MCI1 (PhB) pin select */
#define PINSEL3_MCI1_MASK          ~((uint32_t)(3<<14))    /**< MCI2 (PhB) pin mask */
#define PINSEL3_MCI2                ((uint32_t)(1<<16))    /**< MCI2 (Index) pin select */
#define PINSEL3_MCI2_MASK          ~((uint32_t)(3<<16))    /**< MCI2 (Index) pin mask */

/*********************************************************************//**
 * Macro defines for PINMODE3 register QEI-related bits
 **********************************************************************/
#define PIN_PULL_UP                     0UL
#define PIN_REPEATER                    1UL
#define PIN_NORESISTOR                  2UL
#define PIN_PULL_DOWN                   3UL     

#define PINMODE3_MCI0                ((uint32_t)(PIN_NORESISTOR<<8))     /**< MCIO (PhA) resistor selection */
#define PINMODE3_GPIO1p20            ((uint32_t)(PIN_PULL_DOWN<<8))      /**< GPIO 1.20) resistor selection */
#define PINMODE3_MCI0_MASK          ~((uint32_t)(3<<8))                  /**< MCIO (PhA) resistor mask */

#define PINMODE3_MCI1                ((uint32_t)(PIN_NORESISTOR<<14))    /**< MCI1 (PhB) resistor selection */
#define PINMODE3_GPIO1p23            ((uint32_t)(PIN_PULL_DOWN<<14))      /**< GPIO 1.23) resistor selection */
#define PINMODE3_MCI1_MASK          ~((uint32_t)(3<<14))                 /**< MCI1 (PhB) resistor mask */

#define PINMODE3_MCI2                ((uint32_t)(PIN_PULL_UP<<16))       /**< MCI2 (Index) resistor selection */
#define PINMODE3_GPIO1p24            ((uint32_t)(PIN_PULL_DOWN<<16))      /**< GPIO 1.24) resistor selection */
#define PINMODE3_MCI2_MASK          ~((uint32_t)(3<<16))                 /**< MCI2 (Index) resistor mask */

};

#endif
