// SPDX-License-Identifier: GPL-2.0-only
#include <common.h>
#include <asm/barebox-arm.h>
#include <mach/qualcomm/debug_ll.h>
#include <asm-generic/sections.h>
#include <linux/libfdt.h>
#include <debug_ll.h>

void * __section(".data") uart_base = IOMEM(CONFIG_DEBUG_QUALCOMM_GENI_UART_PHYS_ADDR);

static void qcom_uart_putc_pbl(void *base, int c)
{
	QCOM_UART_PUTC_IMPL(base, c);
}

ENTRY_FUNCTION(start_qualcomm, x0, x1, x2)
{
	void *text;

	arm_cpu_lowlevel_init();

	putc_ll('>');

	text = runtime_address(_text);

	setup_c();

	/* This is kinda ugly but the absolute _text is 0 UNTIL relocation when it becomes NOT 0, so we need to get the runtime address
	 * (start of image) and subtract that so that we actually have an offset and not an absolute function address...
	 */
	pbl_set_putc((void (*)(void *ctx, int c))((ulong)qcom_uart_putc_pbl - (ulong)text), (void *)CONFIG_DEBUG_QUALCOMM_GENI_UART_PHYS_ADDR);

	barebox_arm_entry((unsigned long)0xa0000000, 0x1000000, (void *)x0);
}
