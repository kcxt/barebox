// SPDX-License-Identifier: GPL-2.0-or-later

/* Simple barebox driver for GENI UART */

#include <common.h>
#include <driver.h>
#include <init.h>
#include <malloc.h>
#include <of.h>
#include <io.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <mach/qualcomm/geni_uart.h>
#include <debug_ll.h>

/* For DEBUG_LL */
void * __section(".data") uart_base = IOMEM(CONFIG_DEBUG_QUALCOMM_GENI_UART_PHYS_ADDR);

/*
 * We wrap our port structure around the generic console_device.
 */
struct geni_uart {
	void __iomem		*base;
	struct console_device	uart;		/* uart */
	struct clk		*clk;		/* uart clock */
	u32			uartclk;
};

static inline struct geni_uart *
to_geni_uart(struct console_device *uart)
{
	return container_of(uart, struct geni_uart, uart);
}

static void qcom_uart_putc(struct console_device *cdev, char c)
{
	struct geni_uart *uart = to_geni_uart(cdev);

	QCOM_UART_PUTC_IMPL(uart->base, c);
}

static int qcom_uart_getc(struct console_device *cdev)
{
	struct geni_uart *uart = to_geni_uart(cdev);
	u32 rx_fifo;
	u32 m_irq_status;
	u32 s_irq_status;

	writel(1 << S_OPCODE_SHIFT, uart->base + SE_GENI_S_CMD0);

	qcom_geni_serial_poll_bit(cdev, SE_GENI_M_IRQ_STATUS, M_SEC_IRQ_EN,
				  true);

	m_irq_status = readl(uart->base + SE_GENI_M_IRQ_STATUS);
	s_irq_status = readl(uart->base + SE_GENI_S_IRQ_STATUS);
	writel(m_irq_status, uart->base + SE_GENI_M_IRQ_CLEAR);
	writel(s_irq_status, uart->base + SE_GENI_S_IRQ_CLEAR);
	qcom_geni_serial_poll_bit(cdev, SE_GENI_RX_FIFO_STATUS, RX_FIFO_WC_MSK,
				  true);

	if (!readl(uart->base + SE_GENI_RX_FIFO_STATUS))
		return 0;

	rx_fifo = readl(uart->base + SE_GENI_RX_FIFOn);
	return rx_fifo & 0xff;
}

static int qcom_uart_tstc(struct console_device *cdev)
{
	struct geni_uart *uart = to_geni_uart(cdev);

	return readl(uart->base + SE_GENI_RX_FIFO_STATUS) &
			RX_FIFO_WC_MSK;
}

static int qcom_uart_probe(struct device *dev)
{
	struct geni_uart *uart;
	struct console_device *cdev;
	struct resource *res;

	res = dev_get_resource(dev, IORESOURCE_MEM, 0);
	if (IS_ERR(res)) {
		return PTR_ERR(res);
	}

	uart = xzalloc(sizeof(struct geni_uart));
	uart->base = (void *)res->start;

	/* In case someone calls puts_ll we want it to be the right UART... */
	uart_base = uart->base;

	cdev = &uart->uart;
	cdev->dev = dev;
	cdev->tstc = qcom_uart_tstc;
	cdev->putc = qcom_uart_putc;
	cdev->getc = qcom_uart_getc;
	cdev->linux_console_name = "ttyMSM";
	cdev->linux_earlycon_name = "qcom_geni";
	cdev->phys_base = uart->base;

	console_register(cdev);

	return 0;
}

static struct of_device_id qcom_serial_dt_ids[] = {
	{ .compatible = "qcom,geni-debug-uart" },
};

static struct driver qcom_geni_serial_driver = {
	.name = "qcom_geni_serial",
	.probe = qcom_uart_probe,
	.of_compatible = of_match_ptr(qcom_serial_dt_ids),
};
console_platform_driver(qcom_geni_serial_driver);

static int qcom_geni_qup_probe(struct device *dev)
{
	return of_platform_populate(dev->of_node, NULL, dev);
}

static struct of_device_id qcom_geniqup_dt_ids[] = {
	{ .compatible = "qcom,geni-se-qup" },
};

static struct driver ns16550_serial_driver = {
	.name = "qcom_geni_qup",
	.probe = qcom_geni_qup_probe,
	.of_compatible = of_match_ptr(qcom_geniqup_dt_ids),
};
core_platform_driver(ns16550_serial_driver);

