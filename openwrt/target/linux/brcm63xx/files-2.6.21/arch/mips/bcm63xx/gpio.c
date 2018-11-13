/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2008 Maxime Bizon <mbizon@freebox.fr>
 * Copyright (C) 2008 Florian Fainelli <florian@openwrt.org>
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>

#include <bcm63xx_cpu.h>
#include <bcm63xx_gpio.h>
#include <bcm63xx_io.h>
#include <bcm63xx_regs.h>

static void bcm63xx_gpio_set(struct gpio_chip *chip,
				unsigned gpio, int val)
{
	u32 reg;
	u32 mask;
	u32 tmp;
	unsigned long flags;

	if (gpio >= chip->ngpio)
		BUG();

	if (gpio < 32) {
		reg = GPIO_DATA_LO_REG;
		mask = 1 << gpio;
	} else {
		reg = GPIO_DATA_HI_REG;
		mask = 1 << (gpio - 32);
	}

	local_irq_save(flags);
	tmp = bcm_gpio_readl(reg);
	if (val)
		tmp |= mask;
	else
		tmp &= ~mask;
	bcm_gpio_writel(tmp, reg);
	local_irq_restore(flags);
}

static int bcm63xx_gpio_get(struct gpio_chip *chip, unsigned gpio)
{
	u32 reg;
	u32 mask;

	if (gpio >= chip->ngpio)
		BUG();

	if (gpio < 32) {
		reg = GPIO_DATA_LO_REG;
		mask = 1 << gpio;
	} else {
		reg = GPIO_DATA_HI_REG;
		mask = 1 << (gpio - 32);
	}

	return !!(bcm_gpio_readl(reg) & mask);
}

static int bcm63xx_gpio_set_direction(struct gpio_chip *chip,
					unsigned gpio, int dir)
{
	u32 reg;
	u32 mask;
	u32 tmp;
	unsigned long flags;

	if (gpio >= chip->ngpio)
		BUG();

	if (gpio < 32) {
		reg = GPIO_CTL_LO_REG;
		mask = 1 << gpio;
	} else {
		reg = GPIO_CTL_HI_REG;
		mask = 1 << (gpio - 32);
	}

	local_irq_save(flags);
	tmp = bcm_gpio_readl(reg);
	if (dir == GPIO_DIR_IN)
		tmp &= ~mask;
	else
		tmp |= mask;
	bcm_gpio_writel(tmp, reg);
	local_irq_restore(flags);

	return 0;
}

static int bcm63xx_gpio_direction_input(struct gpio_chip *chip, unsigned gpio)
{
	return bcm63xx_gpio_set_direction(chip, gpio, GPIO_DIR_IN);
}

static int bcm63xx_gpio_direction_output(struct gpio_chip *chip,
					unsigned gpio, int value)
{
	bcm63xx_gpio_set(chip, gpio, value);
	return bcm63xx_gpio_set_direction(chip, gpio, GPIO_DIR_OUT);
}


static struct gpio_chip bcm63xx_gpio_chip = {
	.label			= "bcm63xx-gpio",
	.direction_input	= bcm63xx_gpio_direction_input,
	.direction_output	= bcm63xx_gpio_direction_output,
	.get			= bcm63xx_gpio_get,
	.set			= bcm63xx_gpio_set,
	.base			= 0,
	.ngpio			= BCM63XX_GPIO_COUNT,
};

static int __init bcm63xx_gpio_init(void)
{
	printk(KERN_INFO "registering %d GPIOs\n", BCM63XX_GPIO_COUNT);
	return gpiochip_add(&bcm63xx_gpio_chip);
}
arch_initcall(bcm63xx_gpio_init);
