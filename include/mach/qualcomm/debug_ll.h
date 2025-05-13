/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef __MACH_QUALCOMM_DEBUG_LL_H__
#define __MACH_QUALCOMM_DEBUG_LL_H__

#include <common.h>
#include <io.h>

#ifdef CONFIG_DEBUG_QUALCOMM_GENI_UART

#include <mach/qualcomm/geni_uart.h>

extern void * __section(".data") uart_base;

static inline void __udelay(int us)
{
	volatile int i;

	for (i = 0; i < us; i++);
}

static inline void PUTC_LL(int c)
{
	void __iomem *base = uart_base;
	uint32_t reg;

	// Poll for command to finish
	while (true) {
		reg = readl(base + SE_GENI_STATUS);
		if ((bool)(reg & M_GENI_CMD_ACTIVE) == false)
			break;
		// __udelay(10);
	}

	qcom_geni_serial_setup_tx(base, 1);
	writel(c, base + SE_GENI_TX_FIFOn);
}

#endif

#endif /* __MACH_QUALCOMM_DEBUG_LL_H__ */
