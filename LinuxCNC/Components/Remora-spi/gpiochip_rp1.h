#ifndef GPIOCHIP_RP1_H
#define GPIOCHIP_RP1_H

#define UNUSED(x) (void)(x)

#include <stdint.h>

#define RP1_NUM_GPIOS   54
#define RP1_FSEL_NUM  9


static const char *rp1_gpio_fsel_names[RP1_NUM_GPIOS][RP1_FSEL_NUM] =
{
    { "SPI0_SIO3" , "DPI_PCLK"     , "TXD1"         , "SDA0"         , 0              , "SYS_RIO00" , "PROC_RIO00" , "PIO0"       , "SPI2_CS0" , },
    { "SPI0_SIO2" , "DPI_DE"       , "RXD1"         , "SCL0"         , 0              , "SYS_RIO01" , "PROC_RIO01" , "PIO1"       , "SPI2_SIO1", },
    { "SPI0_CS3"  , "DPI_VSYNC"    , "CTS1"         , "SDA1"         , "IR_RX0"       , "SYS_RIO02" , "PROC_RIO02" , "PIO2"       , "SPI2_SIO0", },
    { "SPI0_CS2"  , "DPI_HSYNC"    , "RTS1"         , "SCL1"         , "IR_TX0"       , "SYS_RIO03" , "PROC_RIO03" , "PIO3"       , "SPI2_SCLK", },
    { "GPCLK0"    , "DPI_D0"       , "TXD2"         , "SDA2"         , "RI0"          , "SYS_RIO04" , "PROC_RIO04" , "PIO4"       , "SPI3_CS0" , },
    { "GPCLK1"    , "DPI_D1"       , "RXD2"         , "SCL2"         , "DTR0"         , "SYS_RIO05" , "PROC_RIO05" , "PIO5"       , "SPI3_SIO1", },
    { "GPCLK2"    , "DPI_D2"       , "CTS2"         , "SDA3"         , "DCD0"         , "SYS_RIO06" , "PROC_RIO06" , "PIO6"       , "SPI3_SIO0", },
    { "SPI0_CS1"  , "DPI_D3"       , "RTS2"         , "SCL3"         , "DSR0"         , "SYS_RIO07" , "PROC_RIO07" , "PIO7"       , "SPI3_SCLK", },
    { "SPI0_CS0"  , "DPI_D4"       , "TXD3"         , "SDA0"         , 0              , "SYS_RIO08" , "PROC_RIO08" , "PIO8"       , "SPI4_CS0" , },
    { "SPI0_SIO1" , "DPI_D5"       , "RXD3"         , "SCL0"         , 0              , "SYS_RIO09" , "PROC_RIO09" , "PIO9"       , "SPI4_SIO0", },
    { "SPI0_SIO0" , "DPI_D6"       , "CTS3"         , "SDA1"         , 0              , "SYS_RIO010", "PROC_RIO010", "PIO10"      , "SPI4_SIO1", },
    { "SPI0_SCLK" , "DPI_D7"       , "RTS3"         , "SCL1"         , 0              , "SYS_RIO011", "PROC_RIO011", "PIO11"      , "SPI4_SCLK", },
    { "PWM0_CHAN0", "DPI_D8"       , "TXD4"         , "SDA2"         , "AAUD_LEFT"    , "SYS_RIO012", "PROC_RIO012", "PIO12"      , "SPI5_CS0" , },
    { "PWM0_CHAN1", "DPI_D9"       , "RXD4"         , "SCL2"         , "AAUD_RIGHT"   , "SYS_RIO013", "PROC_RIO013", "PIO13"      , "SPI5_SIO1", },
    { "PWM0_CHAN2", "DPI_D10"      , "CTS4"         , "SDA3"         , "TXD0"         , "SYS_RIO014", "PROC_RIO014", "PIO14"      , "SPI5_SIO0", },
    { "PWM0_CHAN3", "DPI_D11"      , "RTS4"         , "SCL3"         , "RXD0"         , "SYS_RIO015", "PROC_RIO015", "PIO15"      , "SPI5_SCLK", },
    { "SPI1_CS2"  , "DPI_D12"      , "DSI0_TE_EXT"  , 0              , "CTS0"         , "SYS_RIO016", "PROC_RIO016", "PIO16"      , },
    { "SPI1_CS1"  , "DPI_D13"      , "DSI1_TE_EXT"  , 0              , "RTS0"         , "SYS_RIO017", "PROC_RIO017", "PIO17"      , },
    { "SPI1_CS0"  , "DPI_D14"      , "I2S0_SCLK"    , "PWM0_CHAN2"   , "I2S1_SCLK"    , "SYS_RIO018", "PROC_RIO018", "PIO18"      , "GPCLK1",   },
    { "SPI1_SIO1" , "DPI_D15"      , "I2S0_WS"      , "PWM0_CHAN3"   , "I2S1_WS"      , "SYS_RIO019", "PROC_RIO019", "PIO19"      , },
    { "SPI1_SIO0" , "DPI_D16"      , "I2S0_SDI0"    , "GPCLK0"       , "I2S1_SDI0"    , "SYS_RIO020", "PROC_RIO020", "PIO20"      , },
    { "SPI1_SCLK" , "DPI_D17"      , "I2S0_SDO0"    , "GPCLK1"       , "I2S1_SDO0"    , "SYS_RIO021", "PROC_RIO021", "PIO21"      , },
    { "SD0_CLK"   , "DPI_D18"      , "I2S0_SDI1"    , "SDA3"         , "I2S1_SDI1"    , "SYS_RIO022", "PROC_RIO022", "PIO22"      , },
    { "SD0_CMD"   , "DPI_D19"      , "I2S0_SDO1"    , "SCL3"         , "I2S1_SDO1"    , "SYS_RIO023", "PROC_RIO023", "PIO23"      , },
    { "SD0_DAT0"  , "DPI_D20"      , "I2S0_SDI2"    , 0              , "I2S1_SDI2"    , "SYS_RIO024", "PROC_RIO024", "PIO24"      , "SPI2_CS1" , },
    { "SD0_DAT1"  , "DPI_D21"      , "I2S0_SDO2"    , "MIC_CLK"      , "I2S1_SDO2"    , "SYS_RIO025", "PROC_RIO025", "PIO25"      , "SPI3_CS1" , },
    { "SD0_DAT2"  , "DPI_D22"      , "I2S0_SDI3"    , "MIC_DAT0"     , "I2S1_SDI3"    , "SYS_RIO026", "PROC_RIO026", "PIO26"      , "SPI5_CS1" , },
    { "SD0_DAT3"  , "DPI_D23"      , "I2S0_SDO3"    , "MIC_DAT1"     , "I2S1_SDO3"    , "SYS_RIO027", "PROC_RIO027", "PIO27"      , "SPI1_CS1" , },
    { "SD1_CLK"   , "SDA4"         , "I2S2_SCLK"    , "SPI6_MISO"    , "VBUS_EN0"     , "SYS_RIO10" , "PROC_RIO10" , },
    { "SD1_CMD"   , "SCL4"         , "I2S2_WS"      , "SPI6_SIO0"    , "VBUS_OC0"     , "SYS_RIO11" , "PROC_RIO11" , },
    { "SD1_DAT0"  , "SDA5"         , "I2S2_SDI0"    , "SPI6_SCLK"    , "TXD5"         , "SYS_RIO12" , "PROC_RIO12" , },
    { "SD1_DAT1"  , "SCL5"         , "I2S2_SDO0"    , "SPI6_CS0"     , "RXD5"         , "SYS_RIO13" , "PROC_RIO13" , },
    { "SD1_DAT2"  , "GPCLK3"       , "I2S2_SDI1"    , "SPI6_CS1"     , "CTS5"         , "SYS_RIO14" , "PROC_RIO14" , },
    { "SD1_DAT3"  , "GPCLK4"       , "I2S2_SDO1"    , "SPI6_CS2"     , "RTS5"         , "SYS_RIO15" , "PROC_RIO15" , },
    { "PWM1_CHAN2", "GPCLK3"       , "VBUS_EN0"     , "SDA4"         , "MIC_CLK"      , "SYS_RIO20" , "PROC_RIO20" , },
    { "SPI8_CS1"  , "PWM1_CHAN0"   , "VBUS_OC0"     , "SCL4"         , "MIC_DAT0"     , "SYS_RIO21" , "PROC_RIO21" , },
    { "SPI8_CS0"  , "TXD5"         , "PCIE_CLKREQ_N", "SDA5"         , "MIC_DAT1"     , "SYS_RIO22" , "PROC_RIO22" , },
    { "SPI8_SIO1" , "RXD5"         , "MIC_CLK"      , "SCL5"         , "PCIE_CLKREQ_N", "SYS_RIO23" , "PROC_RIO23" , },
    { "SPI8_SIO0" , "RTS5"         , "MIC_DAT0"     , "SDA6"         , "AAUD_LEFT"    , "SYS_RIO24" , "PROC_RIO24" , "DSI0_TE_EXT", },
    { "SPI8_SCLK" , "CTS5"         , "MIC_DAT1"     , "SCL6"         , "AAUD_RIGHT"   , "SYS_RIO25" , "PROC_RIO25" , "DSI1_TE_EXT", },
    { "PWM1_CHAN1", "TXD5"         , "SDA4"         , "SPI6_SIO1"    , "AAUD_LEFT"    , "SYS_RIO26" , "PROC_RIO26" , },
    { "PWM1_CHAN2", "RXD5"         , "SCL4"         , "SPI6_SIO0"    , "AAUD_RIGHT"   , "SYS_RIO27" , "PROC_RIO27" , },
    { "GPCLK5"    , "RTS5"         , "VBUS_EN1"     , "SPI6_SCLK"    , "I2S2_SCLK"    , "SYS_RIO28" , "PROC_RIO28" , },
    { "GPCLK4"    , "CTS5"         , "VBUS_OC1"     , "SPI6_CS0"     , "I2S2_WS"      , "SYS_RIO29" , "PROC_RIO29" , },
    { "GPCLK5"    , "SDA5"         , "PWM1_CHAN0"   , "SPI6_CS1"     , "I2S2_SDI0"    , "SYS_RIO210", "PROC_RIO210", },
    { "PWM1_CHAN3", "SCL5"         , "SPI7_CS0"     , "SPI6_CS2"     , "I2S2_SDO0"    , "SYS_RIO211", "PROC_RIO211", },
    { "GPCLK3"    , "SDA4"         , "SPI7_SIO0"    , "MIC_CLK"      , "I2S2_SDI1"    , "SYS_RIO212", "PROC_RIO212", "DSI0_TE_EXT", },
    { "GPCLK5"    , "SCL4"         , "SPI7_SIO1"    , "MIC_DAT0"     , "I2S2_SDO1"    , "SYS_RIO213", "PROC_RIO213", "DSI1_TE_EXT", },
    { "PWM1_CHAN0", "PCIE_CLKREQ_N", "SPI7_SCLK"    , "MIC_DAT1"     , "TXD5"         , "SYS_RIO214", "PROC_RIO214", },
    { "SPI8_SCLK" , "SPI7_SCLK"    , "SDA5"         , "AAUD_LEFT"    , "RXD5"         , "SYS_RIO215", "PROC_RIO215", },
    { "SPI8_SIO1" , "SPI7_SIO0"    , "SCL5"         , "AAUD_RIGHT"   , "VBUS_EN2"     , "SYS_RIO216", "PROC_RIO216", },
    { "SPI8_SIO0" , "SPI7_SIO1"    , "SDA6"         , "AAUD_LEFT"    , "VBUS_OC2"     , "SYS_RIO217", "PROC_RIO217", },
    { "SPI8_CS0"  , 0              , "SCL6"         , "AAUD_RIGHT"   , "VBUS_EN3"     , "SYS_RIO218", "PROC_RIO218", },
    { "SPI8_CS1"  , "SPI7_CS0"     , 0              , "PCIE_CLKREQ_N", "VBUS_OC3"     , "SYS_RIO219", "PROC_RIO219", },
};


typedef enum
{
    GPIO_FSEL_FUNC0,
    GPIO_FSEL_FUNC1,
    GPIO_FSEL_FUNC2,
    GPIO_FSEL_FUNC3,
    GPIO_FSEL_FUNC4,
    GPIO_FSEL_FUNC5,
    GPIO_FSEL_FUNC6,
    GPIO_FSEL_FUNC7,
    GPIO_FSEL_FUNC8,
    /* ... */
    GPIO_FSEL_INPUT = 0x10,
    GPIO_FSEL_OUTPUT,
    GPIO_FSEL_GPIO, /* Preserves direction if possible, else input */
    GPIO_FSEL_NONE, /* If possible, else input */
    GPIO_FSEL_MAX
} GPIO_FSEL_T;

typedef enum
{
    PULL_NONE,
    PULL_DOWN,
    PULL_UP,
    PULL_MAX
} GPIO_PULL_T;

typedef enum
{
    DIR_INPUT,
    DIR_OUTPUT,
    DIR_MAX,
} GPIO_DIR_T;

typedef enum
{
    DRIVE_LOW,
    DRIVE_HIGH,
    DRIVE_MAX
} GPIO_DRIVE_T;

typedef struct
{
    int gpio_num;
    int fsel_num;
} rp1_gpio_fsel_result;

typedef struct GPIO_CHIP_INTERFACE_ GPIO_CHIP_INTERFACE_T;

typedef struct GPIO_CHIP_
{
    const char *name;
    const char *compatible;
    const GPIO_CHIP_INTERFACE_T *interface;
    int size;
    uintptr_t data;
} GPIO_CHIP_T;

struct GPIO_CHIP_INTERFACE_
{
    void * (*gpio_create_instance)(const GPIO_CHIP_T *chip, const char *dtnode);
    int (*gpio_count)(void *priv);
    void * (*gpio_probe_instance)(void *priv, volatile uint32_t *base);
    GPIO_FSEL_T (*gpio_get_fsel)(void *priv, uint32_t gpio);
    void (*gpio_set_fsel)(void *priv, uint32_t gpio, const GPIO_FSEL_T func);
    void (*gpio_set_drive)(void *priv, uint32_t gpio, GPIO_DRIVE_T drv);
    void (*gpio_set_dir)(void *priv, uint32_t gpio, GPIO_DIR_T dir);
    GPIO_DIR_T (*gpio_get_dir)(void *priv, uint32_t gpio);
    int (*gpio_get_level)(void *priv, uint32_t gpio);  /* The actual level observed */
    GPIO_DRIVE_T (*gpio_get_drive)(void *priv, uint32_t gpio);  /* What it is being driven as */
    GPIO_PULL_T (*gpio_get_pull)(void *priv, uint32_t gpio);
    void (*gpio_set_pull)(void *priv, uint32_t gpio, GPIO_PULL_T pull);
    const char * (*gpio_get_name)(void *priv, uint32_t gpio);
    const char * (*gpio_get_fsel_name)(void *priv, uint32_t gpio, GPIO_FSEL_T fsel);
};

// Declare rp1_chip as an external variable
extern GPIO_CHIP_T rp1_chip;

#endif // GPIOCHIP_RP1_H
