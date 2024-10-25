#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gpiochip_rp1.h"

// RP1 bar base address is at    0x1f00000000
// device tree reports gpiochip  0x1f000d0000
// therefore need to add  0x0d0000 to the following offsets when using bar base address

#define RP1_IO_BANK0_OFFSET      0x000d0000     // 0x00000000
#define RP1_IO_BANK1_OFFSET      0x000d4000     // 0x00004000
#define RP1_IO_BANK2_OFFSET      0x000d8000     // 0x00008000
#define RP1_SYS_RIO_BANK0_OFFSET 0x000e0000     // 0x00010000
#define RP1_SYS_RIO_BANK1_OFFSET 0x000e4000     // 0x00014000
#define RP1_SYS_RIO_BANK2_OFFSET 0x000e8000     // 0x00018000
#define RP1_PADS_BANK0_OFFSET    0x000f0000     // 0x00020000
#define RP1_PADS_BANK1_OFFSET    0x000f4000     // 0x00024000
#define RP1_PADS_BANK2_OFFSET    0x000f8000     // 0x00028000

#define RP1_RW_OFFSET  0x0000
#define RP1_XOR_OFFSET 0x1000
#define RP1_SET_OFFSET 0x2000
#define RP1_CLR_OFFSET 0x3000

#define RP1_GPIO_CTRL_FSEL_LSB     0
#define RP1_GPIO_CTRL_FSEL_MASK    (0x1f << RP1_GPIO_CTRL_FSEL_LSB)
#define RP1_GPIO_CTRL_OUTOVER_LSB  12
#define RP1_GPIO_CTRL_OUTOVER_MASK (0x03 << RP1_GPIO_CTRL_OUTOVER_LSB)
#define RP1_GPIO_CTRL_OEOVER_LSB   14
#define RP1_GPIO_CTRL_OEOVER_MASK  (0x03 << RP1_GPIO_CTRL_OEOVER_LSB)

#define RP1_PADS_OD_SET       (1 << 7)
#define RP1_PADS_IE_SET       (1 << 6)
#define RP1_PADS_PUE_SET      (1 << 3)
#define RP1_PADS_PDE_SET      (1 << 2)

#define RP1_GPIO_IO_REG_STATUS_OFFSET(offset) (((offset * 2) + 0) * sizeof(uint32_t))
#define RP1_GPIO_IO_REG_CTRL_OFFSET(offset)   (((offset * 2) + 1) * sizeof(uint32_t))
#define RP1_GPIO_PADS_REG_OFFSET(offset)      (sizeof(uint32_t) + (offset * sizeof(uint32_t)))

#define RP1_GPIO_SYS_RIO_REG_OUT_OFFSET        0x0
#define RP1_GPIO_SYS_RIO_REG_OE_OFFSET         0x4
#define RP1_GPIO_SYS_RIO_REG_SYNC_IN_OFFSET    0x8

#define rp1_gpio_write32(base, peri_offset, reg_offset, value) \
    base[(peri_offset + reg_offset)/4] = value

#define rp1_gpio_read32(base, peri_offset, reg_offset) \
    base[(peri_offset + reg_offset)/4]

typedef struct
{
   uint32_t io[3];
   uint32_t pads[3];
   uint32_t sys_rio[3];
} GPIO_STATE_T;

typedef enum
{
    RP1_FSEL_ALT0       = 0x0,
    RP1_FSEL_ALT1       = 0x1,
    RP1_FSEL_ALT2       = 0x2,
    RP1_FSEL_ALT3       = 0x3,
    RP1_FSEL_ALT4       = 0x4,
    RP1_FSEL_ALT5       = 0x5,
    RP1_FSEL_ALT6       = 0x6,
    RP1_FSEL_ALT7       = 0x7,
    RP1_FSEL_ALT8       = 0x8,
    RP1_FSEL_COUNT,
    RP1_FSEL_SYS_RIO    = RP1_FSEL_ALT5,
    RP1_FSEL_NULL       = 0x1f
} RP1_FSEL_T;

static const GPIO_STATE_T gpio_state = {
    .io = {RP1_IO_BANK0_OFFSET, RP1_IO_BANK1_OFFSET, RP1_IO_BANK2_OFFSET},
    .pads = {RP1_PADS_BANK0_OFFSET, RP1_PADS_BANK1_OFFSET, RP1_PADS_BANK2_OFFSET},
    .sys_rio = {RP1_SYS_RIO_BANK0_OFFSET, RP1_SYS_RIO_BANK1_OFFSET, RP1_SYS_RIO_BANK2_OFFSET},
};

static const int rp1_bank_base[] = {0, 28, 34};


static void rp1_gpio_get_bank(int num, int *bank, int *offset)
{
    *bank = *offset = 0;
    if (num >= RP1_NUM_GPIOS)
    {
        assert(0);
        return;
    }

    if (num < rp1_bank_base[1])
        *bank = 0;
    else if (num < rp1_bank_base[2])
        *bank = 1;
    else
        *bank = 2;

   *offset = num - rp1_bank_base[*bank];
}

static uint32_t rp1_gpio_ctrl_read(volatile uint32_t *base, int bank, int offset)
{
    return rp1_gpio_read32(base, gpio_state.io[bank], RP1_GPIO_IO_REG_CTRL_OFFSET(offset));
}

static void rp1_gpio_ctrl_write(volatile uint32_t *base, int bank, int offset,
                                uint32_t value)
{
    rp1_gpio_write32(base, gpio_state.io[bank], RP1_GPIO_IO_REG_CTRL_OFFSET(offset), value);
}

static uint32_t rp1_gpio_pads_read(volatile uint32_t *base, int bank, int offset)
{
    return rp1_gpio_read32(base, gpio_state.pads[bank], RP1_GPIO_PADS_REG_OFFSET(offset));
}

static void rp1_gpio_pads_write(volatile uint32_t *base, int bank, int offset,
                                uint32_t value)
{
    rp1_gpio_write32(base, gpio_state.pads[bank], RP1_GPIO_PADS_REG_OFFSET(offset), value);
}

static uint32_t rp1_gpio_sys_rio_out_read(volatile uint32_t *base, int bank,
                                          int offset)
{
    UNUSED(offset);
    return rp1_gpio_read32(base, gpio_state.sys_rio[bank], RP1_GPIO_SYS_RIO_REG_OUT_OFFSET);
}

static uint32_t rp1_gpio_sys_rio_sync_in_read(volatile uint32_t *base, int bank,
                                              int offset)
{
    UNUSED(offset);
    return rp1_gpio_read32(base, gpio_state.sys_rio[bank],
                           RP1_GPIO_SYS_RIO_REG_SYNC_IN_OFFSET);
}

static void rp1_gpio_sys_rio_out_set(volatile uint32_t *base, int bank, int offset)
{
    rp1_gpio_write32(base, gpio_state.sys_rio[bank],
                     RP1_GPIO_SYS_RIO_REG_OUT_OFFSET + RP1_SET_OFFSET, 1U << offset);
}

static void rp1_gpio_sys_rio_out_clr(volatile uint32_t *base, int bank, int offset)
{
    rp1_gpio_write32(base, gpio_state.sys_rio[bank],
                     RP1_GPIO_SYS_RIO_REG_OUT_OFFSET + RP1_CLR_OFFSET, 1U << offset);
}

static uint32_t rp1_gpio_sys_rio_oe_read(volatile uint32_t *base, int bank)
{
    return rp1_gpio_read32(base, gpio_state.sys_rio[bank],
                           RP1_GPIO_SYS_RIO_REG_OE_OFFSET);
}

static void rp1_gpio_sys_rio_oe_clr(volatile uint32_t *base, int bank, int offset)
{
    rp1_gpio_write32(base, gpio_state.sys_rio[bank],
                     RP1_GPIO_SYS_RIO_REG_OE_OFFSET + RP1_CLR_OFFSET,
                     1U << offset);
}

static void rp1_gpio_sys_rio_oe_set(volatile uint32_t *base, int bank, int offset)
{
    rp1_gpio_write32(base, gpio_state.sys_rio[bank],
                     RP1_GPIO_SYS_RIO_REG_OE_OFFSET + RP1_SET_OFFSET,
                     1U << offset);
}

static void rp1_gpio_set_dir(void *priv, uint32_t gpio, GPIO_DIR_T dir)
{
    volatile uint32_t *base = priv;
    int bank, offset;

    rp1_gpio_get_bank(gpio, &bank, &offset);

    if (dir == DIR_INPUT)
        rp1_gpio_sys_rio_oe_clr(base, bank, offset);
    else if (dir == DIR_OUTPUT)
        rp1_gpio_sys_rio_oe_set(base, bank, offset);
    else
        assert(0);
}

static GPIO_DIR_T rp1_gpio_get_dir(void *priv, unsigned gpio)
{
    volatile uint32_t *base = priv;
    int bank, offset;
    GPIO_DIR_T dir;
    uint32_t reg;

    rp1_gpio_get_bank(gpio, &bank, &offset);
    reg = rp1_gpio_sys_rio_oe_read(base, bank);

    dir = (reg & (1U << offset)) ? DIR_OUTPUT : DIR_INPUT;

    return dir;
}

static GPIO_FSEL_T rp1_gpio_get_fsel(void *priv, unsigned gpio)
{
    volatile uint32_t *base = priv;
    int bank, offset;
    uint32_t reg;
    GPIO_FSEL_T fsel;
    RP1_FSEL_T rsel;

    rp1_gpio_get_bank(gpio, &bank, &offset);
    reg = rp1_gpio_ctrl_read(base, bank, offset);

    rsel = ((reg & RP1_GPIO_CTRL_FSEL_MASK) >> RP1_GPIO_CTRL_FSEL_LSB);
    if (rsel == RP1_FSEL_SYS_RIO)
        fsel = GPIO_FSEL_GPIO;
    else if (rsel == RP1_FSEL_NULL)
        fsel = GPIO_FSEL_NONE;
    else if (rsel < RP1_FSEL_COUNT)
        fsel = (GPIO_FSEL_T)rsel;
    else
        fsel = GPIO_FSEL_MAX;

    return fsel;
}

static void rp1_gpio_set_fsel(void *priv, unsigned gpio, const GPIO_FSEL_T func)
{
    volatile uint32_t *base = priv;
    int bank, offset;
    uint32_t ctrl_reg;
    uint32_t pad_reg;
    uint32_t old_pad_reg;
    RP1_FSEL_T rsel;

    if (func < (GPIO_FSEL_T)RP1_FSEL_COUNT)
        rsel = (RP1_FSEL_T)func;
    else if (func == GPIO_FSEL_INPUT ||
             func == GPIO_FSEL_OUTPUT ||
             func == GPIO_FSEL_GPIO)
        rsel = RP1_FSEL_SYS_RIO;
    else if (func == GPIO_FSEL_NONE)
        rsel = RP1_FSEL_NULL;
    else
        return;

    rp1_gpio_get_bank(gpio, &bank, &offset);
    if (func == GPIO_FSEL_INPUT)
        rp1_gpio_set_dir(priv, gpio, DIR_INPUT);
    else if (func == GPIO_FSEL_OUTPUT)
        rp1_gpio_set_dir(priv, gpio, DIR_OUTPUT);

    ctrl_reg = rp1_gpio_ctrl_read(base, bank, offset) & ~RP1_GPIO_CTRL_FSEL_MASK;
    ctrl_reg |= rsel << RP1_GPIO_CTRL_FSEL_LSB;
    rp1_gpio_ctrl_write(base, bank, offset, ctrl_reg);

    pad_reg = rp1_gpio_pads_read(base, bank, offset);
    old_pad_reg = pad_reg;
    if (rsel == RP1_FSEL_NULL)
    {
        // Disable input
        pad_reg &= ~RP1_PADS_IE_SET;
    }
    else
    {
        // Enable input
        pad_reg |= RP1_PADS_IE_SET;
    }

    if (rsel != RP1_FSEL_NULL)
    {
        // Enable peripheral func output
        pad_reg &= ~RP1_PADS_OD_SET;
    }
    else
    {
        // Disable peripheral func output
        pad_reg |= RP1_PADS_OD_SET;
    }

    if (pad_reg != old_pad_reg)
        rp1_gpio_pads_write(base, bank, offset, pad_reg);
}

static int rp1_gpio_get_level(void *priv, unsigned gpio)
{
    volatile uint32_t *base = priv;
    int bank, offset;
    uint32_t pad_reg;
    uint32_t reg;
    int level;

    rp1_gpio_get_bank(gpio, &bank, &offset);
    pad_reg = rp1_gpio_pads_read(base, bank, offset);
    if (!(pad_reg & RP1_PADS_IE_SET))
	return -1;
    reg = rp1_gpio_sys_rio_sync_in_read(base, bank, offset);
    level = (reg & (1U << offset)) ? 1 : 0;

    return level;
}

static void rp1_gpio_set_drive(void *priv, unsigned gpio, GPIO_DRIVE_T drv)
{
    volatile uint32_t *base = priv;
    int bank, offset;

    rp1_gpio_get_bank(gpio, &bank, &offset);
    if (drv == DRIVE_HIGH)
        rp1_gpio_sys_rio_out_set(base, bank, offset);
    else if (drv == DRIVE_LOW)
        rp1_gpio_sys_rio_out_clr(base, bank, offset);
}

static void rp1_gpio_set_pull(void *priv, unsigned gpio, GPIO_PULL_T pull)
{
    volatile uint32_t *base = priv;
    uint32_t reg;
    int bank, offset;

    rp1_gpio_get_bank(gpio, &bank, &offset);
    reg = rp1_gpio_pads_read(base, bank, offset);
    reg &= ~(RP1_PADS_PDE_SET | RP1_PADS_PUE_SET);
    if (pull == PULL_UP)
        reg |= RP1_PADS_PUE_SET;
    else if (pull == PULL_DOWN)
        reg |= RP1_PADS_PDE_SET;
    rp1_gpio_pads_write(base, bank, offset, reg);
}

static GPIO_PULL_T rp1_gpio_get_pull(void *priv, unsigned gpio)
{
    volatile uint32_t *base = priv;
    uint32_t reg;
    GPIO_PULL_T pull = PULL_NONE;
    int bank, offset;

    rp1_gpio_get_bank(gpio, &bank, &offset);
    reg = rp1_gpio_pads_read(base, bank, offset);
    if (reg & RP1_PADS_PUE_SET)
        pull = PULL_UP;
    else if (reg & RP1_PADS_PDE_SET)
        pull = PULL_DOWN;

    return pull;
}

static GPIO_DRIVE_T rp1_gpio_get_drive(void *priv, unsigned gpio)
{
    volatile uint32_t *base = priv;
    uint32_t reg;
    int bank, offset;

    rp1_gpio_get_bank(gpio, &bank, &offset);
    reg = rp1_gpio_sys_rio_out_read(base, bank, offset);
    return (reg & (1U << offset)) ? DRIVE_HIGH : DRIVE_LOW;
}

static const char *rp1_gpio_get_name(void *priv, unsigned gpio)
{
    static char name_buf[16];
    UNUSED(priv);

    if (gpio >= RP1_NUM_GPIOS)
        return NULL;

    sprintf(name_buf, "GPIO%d", gpio);
    return name_buf;
}

static const char *rp1_gpio_get_fsel_name(void *priv, unsigned gpio, GPIO_FSEL_T fsel)
{
    const char *name = NULL;
    UNUSED(priv);
    switch (fsel)
    {
    case GPIO_FSEL_GPIO:
        name = "gpio";
        break;
    case GPIO_FSEL_INPUT:
        name = "input";
        break;
    case GPIO_FSEL_OUTPUT:
        name = "output";
        break;
    case GPIO_FSEL_NONE:
        name = "none";
        break;
    case GPIO_FSEL_FUNC0:
    case GPIO_FSEL_FUNC1:
    case GPIO_FSEL_FUNC2:
    case GPIO_FSEL_FUNC3:
    case GPIO_FSEL_FUNC4:
    case GPIO_FSEL_FUNC5:
    case GPIO_FSEL_FUNC6:
    case GPIO_FSEL_FUNC7:
    case GPIO_FSEL_FUNC8:
        if (gpio < RP1_NUM_GPIOS)
        {
            name = rp1_gpio_fsel_names[gpio][fsel - GPIO_FSEL_FUNC0];
            if (!name)
                name = "-";
        }
        break;
    default:
        return NULL;
    }
    return name;
}

static void *rp1_gpio_create_instance(const GPIO_CHIP_T *chip,
                                      const char *dtnode)
{
    UNUSED(dtnode);
    return (void *)chip;
}

static int rp1_gpio_count(void *priv)
{
    UNUSED(priv);
    return RP1_NUM_GPIOS;
}

static void *rp1_gpio_probe_instance(void *priv, volatile uint32_t *base)
{
    UNUSED(priv);
    return (void *)base;
}

static const GPIO_CHIP_INTERFACE_T rp1_gpio_interface =
{
    .gpio_create_instance = rp1_gpio_create_instance,
    .gpio_count = rp1_gpio_count,
    .gpio_probe_instance = rp1_gpio_probe_instance,
    .gpio_get_fsel = rp1_gpio_get_fsel,
    .gpio_set_fsel = rp1_gpio_set_fsel,
    .gpio_set_drive = rp1_gpio_set_drive,
    .gpio_set_dir = rp1_gpio_set_dir,
    .gpio_get_dir = rp1_gpio_get_dir,
    .gpio_get_level = rp1_gpio_get_level,
    .gpio_get_drive = rp1_gpio_get_drive,
    .gpio_get_pull = rp1_gpio_get_pull,
    .gpio_set_pull = rp1_gpio_set_pull,
    .gpio_get_name = rp1_gpio_get_name,
    .gpio_get_fsel_name = rp1_gpio_get_fsel_name,
};

GPIO_CHIP_T rp1_chip = {"rp1", "raspberrypi,rp1-gpio", &rp1_gpio_interface, 0x30000, 0};
