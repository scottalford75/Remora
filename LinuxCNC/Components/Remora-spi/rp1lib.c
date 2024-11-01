
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>


#include "rp1lib.h"
#include "gpiochip_rp1.h"
#include "spi-dw.h"


const uint32_t spi_bases[] = {
    RP1_SPI0_BASE,
    RP1_SPI1_BASE,
    RP1_SPI2_BASE,
    RP1_SPI3_BASE,
    RP1_SPI4_BASE,
    RP1_SPI5_BASE,
    RP1_SPI6_BASE,
    RP1_SPI7_BASE,
    RP1_SPI8_BASE
};


GPIO_CHIP_T     *chip;
RP1_DEVICE_T    *inst;
static unsigned num_gpios;
char mosi_pin[15], miso_pin[15], sclk_pin[15], cs_pin[15];


rp1_gpio_fsel_result rp1_get_gpio_fsel_from_name(const char *pin_name)
{
    rp1_gpio_fsel_result result = { -1, -1 }; // Initialise with invalid values

    // Search across all rows and columns in rp1_gpio_fsel_names in gpiochip_rp1.h
        for (int i = 0; i < RP1_NUM_GPIOS; i++) {
        for (int j = 0; j < RP1_FSEL_NUM; j++) {
            if (rp1_gpio_fsel_names[i][j] == NULL) continue;

            if (strcmp(rp1_gpio_fsel_names[i][j], pin_name) == 0) {
                result.gpio_num = i;
                result.fsel_num = j;
                return result;
            }
        }
    }

    return result; // Return with -1 values if not found
}


void rp1spi_transfer(uint8_t spi_num, const void *txbuf, void *rxbuf, uint8_t len)
{
    struct dw_spi *dws;
    struct spi_device *spi;

    const uint8_t *tx = txbuf;
    uint8_t *rx = rxbuf;

    dws = &inst->spi[spi_num];
    spi = &inst->spi_dev[spi_num];

    dws->tx = (void *)tx;
    dws->tx_len = len;
    dws->rx = rx;
    dws->rx_len = len;

    // manage the chip select
    dw_writel(dws, DW_SPI_SER, 1 << spi->chip_select);

    // transfer
    dw_spi_poll_transfer(dws);

    // manage the chip select
    dw_writel(dws, DW_SPI_SER, 0);
}


int rp1spi_init(uint8_t spi_num, uint8_t cs_num, uint8_t mode, uint32_t freq)
{
    DEBUG_PRINT("rp1spi_init(), SPI%d\n", spi_num);

    struct dw_spi       *dws;
    struct dw_spi_cfg   *cfg;
    struct spi_device   *dev;
    rp1_gpio_fsel_result res;

    dws = &inst->spi[spi_num];
    cfg = &inst->spi_cfg[spi_num];
    dev = &inst->spi_dev[spi_num];

    dws->max_freq = RP1_SPI_SPEED;

    dev->max_speed_hz = dws->max_freq;
    dev->chip_select = cs_num;
    dev->mode = mode;
    dev->bits_per_word = 8; // hard coded to 8 bits per word

    cfg->tmode = DW_SPI_CTRLR0_TMOD_TR;  // transmit and receive
    cfg->dfs = dev->bits_per_word;
    cfg->freq = freq;

    // SPI bases address
    dws->regs = (volatile uint32_t *)((uintptr_t)inst->priv + spi_bases[spi_num]);

    DEBUG_PRINT("SPI%d Base address: %11lx, mapped at address: %p\n", spi_num, spi_bases[spi_num], dws->regs);

    // Basic HW init
    dw_spi_hw_init(dws);

    // Configure the gpio pins
    // SPI pin names
    snprintf(mosi_pin, sizeof(mosi_pin), "SPI%d_SIO0", spi_num);
    snprintf(miso_pin, sizeof(miso_pin), "SPI%d_SIO1", spi_num);
    snprintf(sclk_pin, sizeof(sclk_pin), "SPI%d_SCLK", spi_num);
    snprintf(cs_pin, sizeof(cs_pin), "SPI%d_CS%d", spi_num, cs_num);

    // Search for MOSI pin by name
    res = rp1_get_gpio_fsel_from_name(mosi_pin);
    if (res.gpio_num != -1) {
        DEBUG_PRINT("Pin: MOSI -> GPIO Number: %d, FSEL Number: %d\n", res.gpio_num, res.fsel_num);
    } else {
		return -1;
        printf("Failed to get GPIO and FSEL for pin: %s\n", mosi_pin);
    }
    gpio_set_fsel(res.gpio_num, res.fsel_num);
    gpio_set_pull(res.gpio_num, PULL_NONE);

    // Search for MISO pin by name
    res = rp1_get_gpio_fsel_from_name(miso_pin);
    if (res.gpio_num != -1) {
        DEBUG_PRINT("Pin: MISO -> GPIO Number: %d, FSEL Number: %d\n", res.gpio_num, res.fsel_num);
    } else {
		return -1;
        printf("Failed to get GPIO and FSEL for pin: %s\n", miso_pin);
    }
    gpio_set_fsel(res.gpio_num, res.fsel_num);
    gpio_set_pull(res.gpio_num, PULL_NONE);

    // Search for SCLK pin by name
    res = rp1_get_gpio_fsel_from_name(sclk_pin);
    if (res.gpio_num != -1) {
        DEBUG_PRINT("Pin: SCLK -> GPIO Number: %d, FSEL Number: %d\n", res.gpio_num, res.fsel_num);
    } else {
		return -1;
        printf("Failed to get GPIO and FSEL for pin: %s\n", sclk_pin);
    }
    gpio_set_fsel(res.gpio_num, res.fsel_num);
    gpio_set_pull(res.gpio_num, PULL_NONE);

    // Search for CS pin by name
    res = rp1_get_gpio_fsel_from_name(cs_pin);
    if (res.gpio_num != -1) {
        DEBUG_PRINT("Pin: CS   -> GPIO Number: %d, FSEL Number: %d\n", res.gpio_num, res.fsel_num);
    } else {
		return -1;
        printf("Failed to get GPIO and FSEL for pin: %s\n", cs_pin);
    }
    gpio_set_fsel(res.gpio_num, res.fsel_num);
    gpio_set_pull(res.gpio_num, PULL_NONE);

     // Disable controller before writing control registers
    dw_spi_enable_chip(dws, 0);

    // Configre SPI
    dws->n_bytes = 1;
    dev->cr0 = dw_spi_prepare_cr0(dws, dev);
    dw_spi_update_config(dws, dev, cfg);

    uint32_t baudr = dw_readl(dws, DW_SPI_BAUDR);
    DEBUG_PRINT("clk_div = %d\n", baudr);

    baudr = dws->max_freq / dw_readl(dws, DW_SPI_BAUDR);
    DEBUG_PRINT("BAUDR = %d hz\n", baudr);

    // Enable controller after writing control registers
    dw_spi_enable_chip(dws, 1);

    return 1;
}


GPIO_DIR_T gpio_get_dir(unsigned gpio)
{
    return inst->chip->interface->gpio_get_dir(inst->priv, gpio);
}


void gpio_set_dir(unsigned gpio, GPIO_DIR_T dir)
{
   inst->chip->interface->gpio_set_dir(inst->priv, gpio, dir);
}


GPIO_FSEL_T gpio_get_fsel(unsigned gpio)
{
    GPIO_FSEL_T fsel = GPIO_FSEL_MAX;

    fsel = inst->chip->interface->gpio_get_fsel(inst->priv, gpio);

    if (fsel == GPIO_FSEL_GPIO)
    {
        if (gpio_get_dir(gpio) == DIR_OUTPUT)
            fsel = GPIO_FSEL_OUTPUT;
        else
            fsel = GPIO_FSEL_INPUT;
    }

    return fsel;
}


void gpio_set_fsel(unsigned gpio, const GPIO_FSEL_T func)
{
    inst->chip->interface->gpio_set_fsel(inst->priv, gpio, func);
}


void gpio_set_drive(unsigned gpio, GPIO_DRIVE_T drv)
{
    inst->chip->interface->gpio_set_drive(inst->priv, gpio, drv);
}


void gpio_set(unsigned gpio)
{
    inst->chip->interface->gpio_set_drive(inst->priv, gpio, 1);
    inst->chip->interface->gpio_set_dir(inst->priv, gpio, DIR_OUTPUT);
}


void gpio_clear(unsigned gpio)
{
    inst->chip->interface->gpio_set_drive(inst->priv, gpio, 0);
    inst->chip->interface->gpio_set_dir(inst->priv, gpio, DIR_OUTPUT);
}


int gpio_get_level(unsigned gpio)
{
    return inst->chip->interface->gpio_get_level(inst->priv, gpio);
}


GPIO_DRIVE_T gpio_get_drive(unsigned gpio)
{
    return inst->chip->interface->gpio_get_drive(inst->priv, gpio);
}


GPIO_PULL_T gpio_get_pull(unsigned gpio)
{
    return inst->chip->interface->gpio_get_pull(inst->priv, gpio);
}


void gpio_set_pull(unsigned gpio, GPIO_PULL_T pull)
{
    inst->chip->interface->gpio_set_pull(inst->priv, gpio, pull);
}


static RP1_DEVICE_T *rp1_create_instance(const GPIO_CHIP_T *chip, uint64_t phys_addr, const char *name)
{
    RP1_DEVICE_T *inst = (RP1_DEVICE_T *)calloc(1, sizeof(RP1_DEVICE_T));

    inst->chip = chip;
    inst->name = name ? name: chip->name;
    inst->phys_addr = phys_addr;
    inst->priv = NULL;
    inst->base = 0;

    inst->priv = chip->interface->gpio_create_instance(chip, NULL);
    if (!inst->priv)
        return NULL;

    return inst;
}


int rp1lib_init(void)
{
    uint64_t phys_addr = RP1_BAR1;

    DEBUG_PRINT("Initialising RP1 library: %s\n", __func__);

    // rp1_chip is declared in gpiochip_rp1.c
    chip = &rp1_chip;

    inst = rp1_create_instance(chip, phys_addr, NULL);
    if (!inst)
        return -1;

    inst->phys_addr = phys_addr;

    // map memory
    inst->mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (inst->mem_fd < 0)
        return errno;

    inst->priv = mmap(
        NULL,
        RP1_BAR1_LEN,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        inst->mem_fd,
        inst->phys_addr
        );

    DEBUG_PRINT("Base address: %11lx, size: %lx, mapped at address: %p\n", inst->phys_addr, RP1_BAR1_LEN, inst->priv);

    if (inst->priv == MAP_FAILED)
        return errno;

    return (int)num_gpios;
}


int rp1lib_deinit(void)
{
    if (inst)
    {
        //Clean up SPI instances etc

        munmap(inst->priv, RP1_BAR1_LEN);
        free(inst);
        return 0;
    }
    else return -1;
}

