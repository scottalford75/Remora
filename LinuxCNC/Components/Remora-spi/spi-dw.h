#ifndef SPI_DW_H
#define SPI_DW_H

#include <stdio.h>
#include <stdint.h>

#define DEBUG 1 // Define DEBUG to enable debugging output

#if DEBUG
    #define DEBUG_PRINT(fmt, ...) \
        do { fprintf(stderr, "DEBUG: " fmt, ##__VA_ARGS__); } while (0)
#else
    #define DEBUG_PRINT(fmt, ...) \
        do { } while (0)
#endif

#define MIN(a,b) (a < b ? a:b)
#define MIN3(a,b,c) (MIN(MIN(a,b),c))
#define __bf_shf(x) (__builtin_ffsll(x)-1)
#define BIT(a)           (0x1U << (a))
#define GENMASK(a, b)    (((unsigned) -1 >> (31 - (b))) & ~((1U << (a)) - 1))
#define FIELD_PREP(_mask, _val)						\
	({								\
		((typeof(_mask))(_val) << __bf_shf(_mask)) & (_mask);	\
	})
#define DIV_ROUND_UP(n,d) (((n) + (d) -1) / (d))

#define	SPI_CPHA	0x01			/* clock phase */
#define	SPI_CPOL	0x02			/* clock polarity */
#define	SPI_MODE_0	(0|0)			/* (original MicroWire) */
#define	SPI_MODE_1	(0|SPI_CPHA)
#define	SPI_MODE_2	(SPI_CPOL|0)
#define	SPI_MODE_3	(SPI_CPOL|SPI_CPHA)
#define	SPI_CS_HIGH	0x04			/* chipselect active high? */
#define	SPI_LSB_FIRST	0x08		/* per-word bits-on-wire */
#define	SPI_3WIRE	0x10			/* SI/SO signals shared */
#define	SPI_LOOP	0x20			/* loopback mode */
#define	SPI_NO_CS	0x40			/* 1 dev/bus, no chipselect */
#define	SPI_READY	0x80			/* slave pulls low to pause */

/* Synopsys DW SSI IP-core virtual IDs */
#define DW_PSSI_ID 0
#define DW_HSSI_ID 1

/* Synopsys DW SSI component versions (FourCC sequence) */
#define DW_HSSI_102A			0x3130322a

/* DW SSI IP-core ID and version check helpers */
#define dw_spi_ip_is(_dws, _ip) ((_dws)->ip == DW_##_ip##_ID)

#define __dw_spi_ver_cmp(_dws, _ip, _ver, _op)                                 \
  (dw_spi_ip_is(_dws, _ip) && (_dws)->ver _op DW_##_ip##_##_ver)

#define dw_spi_ver_is(_dws, _ip, _ver) __dw_spi_ver_cmp(_dws, _ip, _ver, ==)

#define dw_spi_ver_is_ge(_dws, _ip, _ver) __dw_spi_ver_cmp(_dws, _ip, _ver, >=)

/* DW SPI controller capabilities */
#define DW_SPI_CAP_CS_OVERRIDE  BIT(0)
#define DW_SPI_CAP_DFS32        BIT(1)

/* Register offsets (Generic for both DWC APB SSI and DWC SSI IP-cores) */
#define DW_SPI_CTRLR0 0x00
#define DW_SPI_CTRLR1 0x04
#define DW_SPI_SSIENR 0x08
#define DW_SPI_MWCR 0x0c

#define DW_SPI_SER 0x10
#define DW_SPI_SER_CS0 BIT(0)
#define DW_SPI_SER_CS1 BIT(1)
#define DW_SPI_SER_CS2 BIT(2)
#define DW_SPI_SER_CS3 BIT(3)

#define DW_SPI_BAUDR 0x14
#define DW_SPI_TXFTLR 0x18
#define DW_SPI_RXFTLR 0x1c
#define DW_SPI_TXFLR 0x20
#define DW_SPI_RXFLR 0x24
#define DW_SPI_SR 0x28
#define DW_SPI_IMR 0x2c
#define DW_SPI_ISR 0x30
#define DW_SPI_RISR 0x34
#define DW_SPI_TXOICR 0x38
#define DW_SPI_RXOICR 0x3c
#define DW_SPI_RXUICR 0x40
#define DW_SPI_MSTICR 0x44
#define DW_SPI_ICR 0x48
#define DW_SPI_DMACR 0x4c
#define DW_SPI_DMATDLR 0x50
#define DW_SPI_DMARDLR 0x54
#define DW_SPI_IDR 0x58
#define DW_SPI_VERSION 0x5c
#define DW_SPI_DR 0x60
#define DW_SPI_RX_SAMPLE_DLY 0xf0
#define DW_SPI_CS_OVERRIDE 0xf4

/* Bit fields in CTRLR0 (DWC APB SSI) */
#define DW_PSSI_CTRLR0_DFS_MASK			    GENMASK(3, 0)
#define DW_PSSI_CTRLR0_DFS32_MASK		    GENMASK(20, 16)

#define DW_PSSI_CTRLR0_FRF_MASK			    GENMASK(5, 4)
#define DW_SPI_CTRLR0_FRF_MOTO_SPI		  0x0
#define DW_SPI_CTRLR0_FRF_TI_SSP		    0x1
#define DW_SPI_CTRLR0_FRF_NS_MICROWIRE	0x2
#define DW_SPI_CTRLR0_FRF_RESV			    0x3

#define DW_PSSI_CTRLR0_MODE_MASK		    GENMASK(7, 6)
#define DW_PSSI_CTRLR0_SCPHA			      BIT(6)
#define DW_PSSI_CTRLR0_SCPOL			      BIT(7)

#define DW_PSSI_CTRLR0_TMOD_MASK		    GENMASK(9, 8)
#define DW_SPI_CTRLR0_TMOD_TR			      0x0	/* xmit & recv */
#define DW_SPI_CTRLR0_TMOD_TO			      0x1	/* xmit only */
#define DW_SPI_CTRLR0_TMOD_RO			      0x2	/* recv only */
#define DW_SPI_CTRLR0_TMOD_EPROMREAD		0x3	/* eeprom read mode */

#define DW_PSSI_CTRLR0_SLV_OE			      BIT(10)
#define DW_PSSI_CTRLR0_SRL			        BIT(11)
#define DW_PSSI_CTRLR0_CFS			        BIT(12)

/* Bit fields in CTRLR0 (DWC SSI with AHB interface) */
#define DW_HSSI_CTRLR0_DFS_MASK			GENMASK(4, 0)
#define DW_HSSI_CTRLR0_FRF_MASK			GENMASK(7, 6)
#define DW_HSSI_CTRLR0_SCPHA			BIT(8)
#define DW_HSSI_CTRLR0_SCPOL			BIT(9)
#define DW_HSSI_CTRLR0_TMOD_MASK		GENMASK(11, 10)
#define DW_HSSI_CTRLR0_SRL			BIT(13)
#define DW_HSSI_CTRLR0_MST			BIT(31)

#define BITS_PER_BYTE 8
#define DW_SPI_GET_BYTE(_val, _idx) \
	((_val) >> (BITS_PER_BYTE * (_idx)) & 0xff)

struct spi_device {
  uint32_t  cr0;
  uint32_t  max_speed_hz;
  uint8_t	chip_select;
  uint8_t	mode;
  uint8_t	bits_per_word;
};


/* Slave spi_transfer/spi_mem_op related */
struct dw_spi_cfg {
	uint8_t tmode;
	uint8_t dfs;
	uint32_t ndf;
	uint32_t freq;
};

struct dw_spi {
  uint32_t       ip;   /* Synopsys DW SSI IP-core ID */
  uint32_t       ver;  /* Synopsys component version */
  uint32_t       caps; /* DW SPI capabilities */

  void           *regs;
  unsigned long  paddr;

  uint32_t       fifo_len;       /* depth of the FIFO buffer */
  unsigned int   dfs_offset;    /* CTRLR0 DFS field offset */
  uint32_t       max_freq;      /* max bus freq supported */

  void           *tx;
  unsigned int   tx_len;
  void           *rx;
  unsigned int   rx_len;

  uint8_t        n_bytes; /* current is a 1/2 bytes op */
  uint32_t       current_freq; /* frequency in hz */
};


// Forward declarations
void dw_spi_hw_init(struct dw_spi *dws);
uint32_t dw_spi_prepare_cr0(struct dw_spi *dws, struct spi_device *spi);
void dw_spi_update_config(struct dw_spi *dws, struct spi_device *spi, struct dw_spi_cfg *cfg);
int dw_spi_poll_transfer(struct dw_spi *dws);


// Read a 32-bit value from the SPI device's registers
static inline uint32_t dw_readl(struct dw_spi *dws, uint32_t offset) {
  return *(volatile uint32_t *)(dws->regs + offset);
}

// Write a 32-bit value to the SPI device's registers
static inline void dw_writel(struct dw_spi *dws, uint32_t offset, uint32_t val) {
  *(volatile uint32_t *)(dws->regs + offset) = val;
}

// enable spi
static inline void dw_spi_enable_chip(struct dw_spi *dws, int enable) {
    dw_writel(dws, DW_SPI_SSIENR, (enable ? 1 : 0));
}

// Set the SPI clock divisor
static inline void dw_spi_set_clk(struct dw_spi *dws, uint16_t div) {
  dw_writel(dws, DW_SPI_BAUDR, div);
}

// Disable interrupts
static inline void dw_spi_mask_intr(struct dw_spi *dws, uint32_t mask) {
  uint32_t new_mask;

  new_mask = dw_readl(dws, DW_SPI_IMR) & ~mask;
  dw_writel(dws, DW_SPI_IMR, new_mask);
}

// Enable interrupts
static inline void dw_spi_umask_intr(struct dw_spi *dws, uint32_t mask) {
  uint32_t new_mask;

  new_mask = dw_readl(dws, DW_SPI_IMR) | mask;
  dw_writel(dws, DW_SPI_IMR, new_mask);
}


/*
 * This disables the SPI controller, interrupts, clears the interrupts status
 * and CS, then re-enables the controller back. Transmit and receive FIFO
 * buffers are cleared when the device is disabled.
 */


static inline void dw_spi_reset_chip(struct dw_spi *dws) {
    dw_spi_enable_chip(dws, 0);
    dw_spi_mask_intr(dws, 0xff);
    dw_readl(dws, DW_SPI_ICR);
    dw_writel(dws, DW_SPI_SER, 0);
    dw_spi_enable_chip(dws, 1);
}

// Shutdown the SPI chip
static inline void dw_spi_shutdown_chip(struct dw_spi *dws) {
  dw_spi_enable_chip(dws, 0);
  dw_spi_set_clk(dws, 0);
}





#endif // SPI_DW_H
