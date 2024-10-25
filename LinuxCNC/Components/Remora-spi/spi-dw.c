#include <stdint.h>

#include "spi-dw.h"

/* Return the max entries we can fill into tx fifo */
static inline uint32_t dw_spi_tx_max(struct dw_spi *dws)
{
  uint32_t tx_room, rxtx_gap;

  tx_room = dws->fifo_len - dw_readl(dws, DW_SPI_TXFLR);

  /*
   * Another concern is about the tx/rx mismatch, we
   * though to use (dws->fifo_len - rxflr - txflr) as
   * one maximum value for tx, but it doesn't cover the
   * data which is out of tx/rx fifo and inside the
   * shift registers. So a control from sw point of
   * view is taken.
   */
  rxtx_gap = dws->fifo_len - (dws->rx_len - dws->tx_len);

  return MIN3((uint32_t)dws->tx_len, tx_room, rxtx_gap);
}

/* Return the max entries we should read out of rx fifo */
static inline uint32_t dw_spi_rx_max(struct dw_spi *dws)
{
  return MIN(dws->rx_len, dw_readl(dws, DW_SPI_RXFLR));
}

static void dw_writer(struct dw_spi *dws)
{
  uint32_t max = dw_spi_tx_max(dws);
  uint32_t txw = 0;

  while (max--) {
    if (dws->tx) {
      if (dws->n_bytes == 1)
        txw = *(uint8_t *)(dws->tx);
      else if (dws->n_bytes == 2)
        txw = *(uint16_t *)(dws->tx);
      else
        txw = *(uint32_t *)(dws->tx);

      dws->tx += dws->n_bytes;
    }
    dw_writel(dws, DW_SPI_DR, txw);
    --dws->tx_len;
  }
}

static void dw_reader(struct dw_spi *dws)
{
  uint32_t max = dw_spi_rx_max(dws);
  uint32_t rxw;

  while (max--) {
    rxw = dw_readl(dws, DW_SPI_DR);
    if (dws->rx) {
      if (dws->n_bytes == 1)
        *(uint8_t *)(dws->rx) = rxw;
      else if (dws->n_bytes == 2)
        *(uint16_t *)(dws->rx) = rxw;
      else
        *(uint32_t *)(dws->rx) = rxw;

      dws->rx += dws->n_bytes;
    }
    --dws->rx_len;
  }
}


uint32_t dw_spi_prepare_cr0(struct dw_spi *dws, struct spi_device *spi)
{
	uint32_t cr0 = 0;

	if (dw_spi_ip_is(dws, PSSI)) {
		/* CTRLR0[ 5: 4] Frame Format */
		cr0 |= FIELD_PREP(DW_PSSI_CTRLR0_FRF_MASK, DW_SPI_CTRLR0_FRF_MOTO_SPI);

		/*
		 * SPI mode (SCPOL|SCPH)
		 * CTRLR0[ 6] Serial Clock Phase
		 * CTRLR0[ 7] Serial Clock Polarity
		 */
		if (spi->mode & SPI_CPOL)
			cr0 |= DW_PSSI_CTRLR0_SCPOL;
		if (spi->mode & SPI_CPHA)
			cr0 |= DW_PSSI_CTRLR0_SCPHA;

		/* CTRLR0[11] Shift Register Loop */
		if (spi->mode & SPI_LOOP)
			cr0 |= DW_PSSI_CTRLR0_SRL;
	} else {
		/* CTRLR0[ 7: 6] Frame Format */
		cr0 |= FIELD_PREP(DW_HSSI_CTRLR0_FRF_MASK, DW_SPI_CTRLR0_FRF_MOTO_SPI);

		/*
		 * SPI mode (SCPOL|SCPH)
		 * CTRLR0[ 8] Serial Clock Phase
		 * CTRLR0[ 9] Serial Clock Polarity
		 */
		if (spi->mode & SPI_CPOL)
			cr0 |= DW_HSSI_CTRLR0_SCPOL;
		if (spi->mode & SPI_CPHA)
			cr0 |= DW_HSSI_CTRLR0_SCPHA;

		/* CTRLR0[13] Shift Register Loop */
		if (spi->mode & SPI_LOOP)
			cr0 |= DW_HSSI_CTRLR0_SRL;

		/* CTRLR0[31] MST */
		if (dw_spi_ver_is_ge(dws, HSSI, 102A))
			cr0 |= DW_HSSI_CTRLR0_MST;
	}

	return cr0;
}


void dw_spi_update_config(struct dw_spi *dws, struct spi_device *spi, struct dw_spi_cfg *cfg)
{
	uint32_t cr0 = spi->cr0;
	uint32_t speed_hz;
	uint16_t clk_div;

	/* CTRLR0[ 4/3: 0] or CTRLR0[ 20: 16] Data Frame Size */
	cr0 |= (cfg->dfs - 1) << dws->dfs_offset;

	if (dw_spi_ip_is(dws, PSSI))
		/* CTRLR0[ 9:8] Transfer Mode */
		cr0 |= FIELD_PREP(DW_PSSI_CTRLR0_TMOD_MASK, cfg->tmode);
	else
		/* CTRLR0[11:10] Transfer Mode */
		cr0 |= FIELD_PREP(DW_HSSI_CTRLR0_TMOD_MASK, cfg->tmode);

	dw_writel(dws, DW_SPI_CTRLR0, cr0);

	if (cfg->tmode == DW_SPI_CTRLR0_TMOD_EPROMREAD ||
	    cfg->tmode == DW_SPI_CTRLR0_TMOD_RO)
		dw_writel(dws, DW_SPI_CTRLR1, cfg->ndf ? cfg->ndf - 1 : 0);

	/* Note DW APB SSI clock divider doesn't support odd numbers */
	clk_div = (DIV_ROUND_UP(dws->max_freq, cfg->freq) + 1) & 0xfffe;
	speed_hz = dws->max_freq / clk_div;

	if (dws->current_freq != speed_hz) {
		dw_spi_set_clk(dws, clk_div);
		dws->current_freq = speed_hz;
	}
}

/*
 * The iterative procedure of the poll-based transfer is simple: write as much
 * as possible to the Tx FIFO, wait until the pending to receive data is ready
 * to be read, read it from the Rx FIFO and check whether the performed
 * procedure has been successful.
 *
 * Note this method the same way as the IRQ-based transfer won't work well for
 * the SPI devices connected to the controller with native CS due to the
 * automatic CS assertion/de-assertion.
 */
int dw_spi_poll_transfer(struct dw_spi *dws)
{
	do {
		dw_writer(dws);
		dw_reader(dws);
	} while (dws->rx_len);

	return 0;
}


void dw_spi_hw_init(struct dw_spi *dws)
{
    dw_spi_reset_chip(dws);

    /*
    * Retrieve the Synopsys component version if it hasn't been specified
    * by the platform. CoreKit version ID is encoded as a 3-chars ASCII
    * code enclosed with '*' (typical for the most of Synopsys IP-cores).
    */
	if (!dws->ver) {
		dws->ver = dw_readl(dws, DW_SPI_VERSION);
		DEBUG_PRINT("dws->ver = %x\n", dws->ver);

        DEBUG_PRINT("Synopsys DWC%sSSI v%c.%c%c\n",
			dw_spi_ip_is(dws, PSSI) ? " APB " : " ",
			DW_SPI_GET_BYTE(dws->ver, 3), DW_SPI_GET_BYTE(dws->ver, 2),
			DW_SPI_GET_BYTE(dws->ver, 1));
	}

	/*
	* Try to detect the FIFO depth if not set by interface driver,
	* the depth could be from 2 to 256 from HW spec
	*/
	if (!dws->fifo_len) {
		uint32_t fifo;

		for (fifo = 1; fifo < 256; fifo++) {
			dw_writel(dws, DW_SPI_TXFTLR, fifo);
			if (fifo != dw_readl(dws, DW_SPI_TXFTLR))
				break;
		}
		dw_writel(dws, DW_SPI_TXFTLR, 0);

		dws->fifo_len = (fifo == 1) ? 0 : fifo;
		DEBUG_PRINT("Detected FIFO size: %u bytes\n", dws->fifo_len);
	}

	/*
	* Detect CTRLR0.DFS field size and offset by testing the lowest bits
	* writability. Note DWC SSI controller also has the extended DFS, but
	* with zero offset.
	*/
	if (dw_spi_ip_is(dws, PSSI)) {
		uint32_t cr0, tmp = dw_readl(dws, DW_SPI_CTRLR0);

		dw_spi_enable_chip(dws, 0);
		dw_writel(dws, DW_SPI_CTRLR0, 0xffffffff);
		cr0 = dw_readl(dws, DW_SPI_CTRLR0);
		dw_writel(dws, DW_SPI_CTRLR0, tmp);
		dw_spi_enable_chip(dws, 1);

		if (!(cr0 & DW_PSSI_CTRLR0_DFS_MASK)) {
			dws->caps |= DW_SPI_CAP_DFS32;
			dws->dfs_offset = __bf_shf(DW_PSSI_CTRLR0_DFS32_MASK);
			DEBUG_PRINT("Detected 32-bits max data frame size\n");
		}
	} else {
		dws->caps |= DW_SPI_CAP_DFS32;
	}

    /* enable HW fixup for explicit CS deselect for Amazon's alpine chip */
	if (dws->caps & DW_SPI_CAP_CS_OVERRIDE)
		dw_writel(dws, DW_SPI_CS_OVERRIDE, 0xF);
}

