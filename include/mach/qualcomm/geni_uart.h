#ifndef __QCOM_GENI_UART__
#define __QCOM_GENI_UART__

#include <io.h>

#define UART_OVERSAMPLING	32
#define STALE_TIMEOUT	160

/* Registers*/
#define GENI_FORCE_DEFAULT_REG	0x20
#define GENI_SER_M_CLK_CFG	0x48
#define GENI_SER_S_CLK_CFG	0x4C
#define SE_HW_PARAM_0	0xE24
#define SE_GENI_STATUS	0x40
#define SE_GENI_S_CMD0	0x630
#define SE_GENI_S_CMD_CTRL_REG	0x634
#define SE_GENI_S_IRQ_CLEAR	0x648
#define SE_GENI_S_IRQ_STATUS	0x640
#define SE_GENI_S_IRQ_EN	0x644
#define SE_GENI_M_CMD0	0x600
#define SE_GENI_M_CMD_CTRL_REG	0x604
#define SE_GENI_M_IRQ_CLEAR	0x618
#define SE_GENI_M_IRQ_STATUS	0x610
#define SE_GENI_M_IRQ_EN	0x614
#define SE_GENI_TX_FIFOn	0x700
#define SE_GENI_RX_FIFOn	0x780
#define SE_GENI_TX_FIFO_STATUS	0x800
#define SE_GENI_RX_FIFO_STATUS	0x804
#define SE_GENI_TX_WATERMARK_REG	0x80C
#define SE_GENI_TX_PACKING_CFG0	0x260
#define SE_GENI_TX_PACKING_CFG1	0x264
#define SE_GENI_RX_PACKING_CFG0	0x284
#define SE_GENI_RX_PACKING_CFG1	0x288
#define SE_UART_RX_STALE_CNT	0x294
#define SE_UART_TX_TRANS_LEN	0x270
#define SE_UART_TX_STOP_BIT_LEN	0x26c
#define SE_UART_TX_WORD_LEN	0x268
#define SE_UART_RX_WORD_LEN	0x28c
#define SE_UART_TX_TRANS_CFG	0x25c
#define SE_UART_TX_PARITY_CFG	0x2a4
#define SE_UART_RX_TRANS_CFG	0x280
#define SE_UART_RX_PARITY_CFG	0x2a8

#define M_TX_FIFO_WATERMARK_EN	(BIT(30))
#define DEF_TX_WM	2
/* GENI_FORCE_DEFAULT_REG fields */
#define FORCE_DEFAULT	(BIT(0))

#define S_CMD_ABORT_EN	(BIT(5))

#define UART_START_READ	0x1

/* GENI_M_CMD_CTRL_REG */
#define M_GENI_CMD_CANCEL	(BIT(2))
#define M_GENI_CMD_ABORT	(BIT(1))
#define M_GENI_DISABLE	(BIT(0))

#define M_CMD_ABORT_EN	(BIT(5))
#define M_CMD_DONE_EN	(BIT(0))
#define M_CMD_DONE_DISABLE_MASK	(~M_CMD_DONE_EN)

#define S_GENI_CMD_ABORT	(BIT(1))

/* GENI_S_CMD0 fields */
#define S_OPCODE_MSK	(GENMASK(31, 27))
#define S_PARAMS_MSK	(GENMASK(26, 0))

/* GENI_STATUS fields */
#define M_GENI_CMD_ACTIVE	(BIT(0))
#define S_GENI_CMD_ACTIVE	(BIT(12))
#define M_CMD_DONE_EN	(BIT(0))
#define S_CMD_DONE_EN	(BIT(0))

#define M_OPCODE_SHIFT	27
#define S_OPCODE_SHIFT	27
#define M_TX_FIFO_WATERMARK_EN	(BIT(30))
#define UART_START_TX	0x1
#define UART_CTS_MASK	(BIT(1))
#define M_SEC_IRQ_EN	(BIT(31))
#define TX_FIFO_WC_MSK	(GENMASK(27, 0))
#define RX_FIFO_WC_MSK	(GENMASK(24, 0))

#define S_RX_FIFO_WATERMARK_EN	(BIT(26))
#define S_RX_FIFO_LAST_EN	(BIT(27))
#define M_RX_FIFO_WATERMARK_EN	(BIT(26))
#define M_RX_FIFO_LAST_EN	(BIT(27))

/* GENI_SER_M_CLK_CFG/GENI_SER_S_CLK_CFG */
#define SER_CLK_EN	(BIT(0))
#define CLK_DIV_MSK	(GENMASK(15, 4))
#define CLK_DIV_SHFT	4

/* SE_HW_PARAM_0 fields */
#define TX_FIFO_WIDTH_MSK	(GENMASK(29, 24))
#define TX_FIFO_WIDTH_SHFT	24
#define TX_FIFO_DEPTH_MSK	(GENMASK(21, 16))
#define TX_FIFO_DEPTH_SHFT	16

/* GENI SE QUP Registers */
#define QUP_HW_VER_REG		0x4
#define  QUP_SE_VERSION_2_5	0x20050000

static inline void qcom_geni_serial_setup_tx(void __iomem *base, uint32_t xmit_size)
{
	uint32_t m_cmd;

	writel(xmit_size, base + SE_UART_TX_TRANS_LEN);
	m_cmd = UART_START_TX << M_OPCODE_SHIFT;
	writel(m_cmd, base + SE_GENI_M_CMD0);
}

static inline bool qcom_geni_serial_poll_bit(void *base, int offset,
				      int field, bool set)
{
	u32 reg;
	uint timeout_us = 1000 * 100;

	while (timeout_us) {
		reg = readl(base + offset);
		if ((bool)(reg & field) == set)
			return true;
#if defined(udelay)
		udelay(10);
#endif
		timeout_us -= 10;
	}
	return false;
}

#define QCOM_UART_PUTC_IMPL(base, c) \
		/* Wait for UART to be idle */ \
		qcom_geni_serial_poll_bit(base, SE_GENI_STATUS, \
					M_GENI_CMD_ACTIVE, false); \
		qcom_geni_serial_setup_tx(base, 1); \
		writel(c, base + SE_GENI_TX_FIFOn) \

#endif /* __QCOM_GENI_UART__ */
