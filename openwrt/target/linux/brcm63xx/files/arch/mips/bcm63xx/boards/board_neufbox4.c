/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2008 Maxime Bizon <mbizon@freebox.fr>
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/platform_device.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/physmap.h>
#include <linux/ssb/ssb.h>
#include <linux/bitops.h>
#include <linux/interrupt.h>
#include <linux/reboot.h>
#include <linux/input.h>
#include <linux/gpio.h>
#include <linux/gpio_buttons.h>
#include <linux/leds.h>
#include <asm/addrspace.h>
#include <bcm63xx_board.h>
#include <bcm63xx_cpu.h>
#include <bcm63xx_regs.h>
#include <bcm63xx_io.h>
#include <bcm63xx_irq.h>
#include <bcm63xx_board.h>
#include <bcm63xx_dev_pci.h>
#include <bcm63xx_dev_uart.h>
#include <bcm63xx_dev_wdt.h>
#include <bcm63xx_dev_enet.h>
#include <bcm63xx_dev_pcmcia.h>
#include <bcm63xx_dev_usb_ohci.h>
#include <bcm63xx_dev_usb_ehci.h>
#include <bcm63xx_dev_usb_udc.h>
#include <bcm63xx_dev_spi.h>
#include <board_bcm963xx.h>

#define PFX	"board_bcm963xx: "

static struct bcm963xx_nvram nvram;
static unsigned int mac_addr_used = 0;
static struct board_info board;

/*
 * known 6338 boards
 */

#ifdef CONFIG_BCM63XX_CPU_6338
static struct board_info __initdata board_96338gw = {
	.name				= "96338GW",
	.expected_cpu_id		= 0x6338,

	.has_enet0			= 1,
	.enet0 = {
		.has_phy		= 1,
		.use_internal_phy	= 1,
	},

	.has_ohci0			= 1,
};
#endif

/*
 * known 6348 boards
 */
#ifdef CONFIG_BCM63XX_CPU_6348
static struct board_info __initdata board_96348r = {
	.name				= "96348R",
	.expected_cpu_id		= 0x6348,

	.has_enet0			= 1,
	.has_pci			= 1,

	.enet0 = {
		.has_phy		= 1,
		.use_internal_phy	= 1,
	},
};

static struct board_info __initdata board_96348gw_10 = { 
	.name				= "96348GW-10",
	.expected_cpu_id		= 0x6348,
	
	.has_enet0			= 1,
	.has_enet1			= 1,
	.has_pci			= 1, 
	
	.enet0 = {
		.has_phy		= 1,
		.use_internal_phy	= 1,
	},
	.enet1 = {
		.force_speed_100	= 1,
		.force_duplex_full	= 1,
	},
	
	.has_ohci0			= 1,
	.has_pccard			= 1,
	.has_ehci0			= 1,
}; 

static struct board_info __initdata board_96348gw_11 = {
	.name				= "96348GW-11",
	.expected_cpu_id		= 0x6348,

	.has_enet0			= 1,
	.has_enet1			= 1,
	.has_pci			= 1,

	.enet0 = {
		.has_phy		= 1,
		.use_internal_phy	= 1,
	},

	.enet1 = {
		.force_speed_100	= 1,
		.force_duplex_full	= 1,
	},


	.has_ohci0 = 1,
	.has_pccard = 1,
	.has_ehci0 = 1,
};

static struct board_info __initdata board_96348gw = {
	.name				= "96348GW",
	.expected_cpu_id		= 0x6348,

	.has_enet0			= 1,
	.has_enet1			= 1,
	.has_pci			= 1,

	.enet0 = {
		.has_phy		= 1,
		.use_internal_phy	= 1,
	},
	.enet1 = {
		.force_speed_100	= 1,
		.force_duplex_full	= 1,
	},

	.has_ohci0 = 1,
};

static struct board_info __initdata board_FAST2404 = {
        .name                           = "F@ST2404",
        .expected_cpu_id                = 0x6348,

        .has_enet0                      = 1,
        .has_enet1                      = 1,
        .has_pci                        = 1,

        .enet0 = {
                .has_phy                = 1,
                .use_internal_phy       = 1,
        },

        .enet1 = {
                .force_speed_100        = 1,
                .force_duplex_full      = 1,
        },


        .has_ohci0 = 1,
        .has_pccard = 1,
        .has_ehci0 = 1,
};

static struct board_info __initdata board_DV201AMR = {
	.name				= "DV201AMR",
	.expected_cpu_id		= 0x6348,

	.has_pci			= 1,
	.has_ohci0			= 1,
	.has_udc0			= 1,

	.has_enet0			= 1,
	.has_enet1			= 1,
	.enet0 = {
		.has_phy		= 1,
		.use_internal_phy	= 1,
	},
	.enet1 = {
		.force_speed_100	= 1,
		.force_duplex_full	= 1,
	},
};

static struct board_info __initdata board_96348gw_a = {
	.name				= "96348GW-A",
	.expected_cpu_id		= 0x6348,

	.has_enet0			= 1,
	.has_enet1			= 1,
	.has_pci			= 1,

	.enet0 = {
		.has_phy		= 1,
		.use_internal_phy	= 1,
	},
	.enet1 = {
		.force_speed_100	= 1,
		.force_duplex_full	= 1,
	},

	.has_ohci0 = 1,
};


#endif

/*
 * known 6358 boards
 */
#ifdef CONFIG_BCM63XX_CPU_6358
static struct board_info __initdata board_96358vw = {
	.name				= "96358VW",
	.expected_cpu_id		= 0x6358,

	.has_enet0			= 1,
	.has_enet1			= 1,
	.has_pci			= 1,

	.enet0 = {
		.has_phy		= 1,
		.use_internal_phy	= 1,
	},

	.enet1 = {
		.force_speed_100	= 1,
		.force_duplex_full	= 1,
	},


	.has_ohci0 = 1,
	.has_pccard = 1,
	.has_ehci0 = 1,
};

static struct board_info __initdata board_96358vw2 = {
	.name				= "96358VW2",
	.expected_cpu_id		= 0x6358,

	.has_enet0			= 1,
	.has_enet1			= 1,
	.has_pci			= 1,

	.enet0 = {
		.has_phy		= 1,
		.use_internal_phy	= 1,
	},

	.enet1 = {
		.force_speed_100	= 1,
		.force_duplex_full	= 1,
	},


	.has_ohci0 = 1,
	.has_pccard = 1,
	.has_ehci0 = 1,
};

static struct board_info __initdata board_AGPFS0 = {
	.name                           = "AGPF-S0",
	.expected_cpu_id                = 0x6358,

	.has_enet0                      = 1,
	.has_enet1                      = 1,
	.has_pci                        = 1,

	.enet0 = {
		.has_phy                = 1,
		.use_internal_phy       = 1,
	},

	.enet1 = {
		.force_speed_100        = 1,
		.force_duplex_full      = 1,
	},

	.has_ohci0 = 1,
	.has_ehci0 = 1,
};
#endif

/*
 * all boards
 */
static const struct board_info __initdata *bcm963xx_boards[] = {
#ifdef CONFIG_BCM63XX_CPU_6338
	&board_96338gw,
#endif
#ifdef CONFIG_BCM63XX_CPU_6348
	&board_96348r,
	&board_96348gw,
	&board_96348gw_10,
	&board_96348gw_11,
	&board_FAST2404,
	&board_DV201AMR,
	&board_96348gw_a,
#endif

#ifdef CONFIG_BCM63XX_CPU_6358
	&board_96358vw,
	&board_96358vw2,
	&board_AGPFS0,
#endif
};

/*
 * early init callback, read nvram data from flash and checksum it
 */
void __init board_prom_init(void)
{
	unsigned int check_len, i;
	u8 *boot_addr, *cfe, *p;
	char cfe_version[32];
	u32 val;

	/* read base address of boot chip select (0) */
	val = bcm_mpi_readl(MPI_CSBASE_REG(0));
	val &= MPI_CSBASE_BASE_MASK;
	boot_addr = (u8 *)KSEG1ADDR(val);

	/* dump cfe version */
	cfe = boot_addr + BCM963XX_CFE_VERSION_OFFSET;
	if (!memcmp(cfe, "cfe-v", 5))
		snprintf(cfe_version, sizeof(cfe_version), "%u.%u.%u-%u.%u",
			 cfe[5], cfe[6], cfe[7], cfe[8], cfe[9]);
	else
		strcpy(cfe_version, "unknown");
	printk(KERN_INFO PFX "CFE version: %s\n", cfe_version);

	/* extract nvram data */
	memcpy(&nvram, boot_addr + BCM963XX_NVRAM_OFFSET, sizeof(nvram));

	/* check checksum before using data */
	if (nvram.version <= 4)
		check_len = offsetof(struct bcm963xx_nvram, checksum_old);
	else
		check_len = sizeof(nvram);
	val = 0;
	p = (u8 *)&nvram;
	while (check_len--)
		val += *p;
	if (val) {
		printk(KERN_ERR PFX "invalid nvram checksum\n");
		return;
	}

	/* find board by name */
	for (i = 0; i < ARRAY_SIZE(bcm963xx_boards); i++) {
		if (strncmp(nvram.name, bcm963xx_boards[i]->name,
			    sizeof(nvram.name)))
			continue;
		/* copy, board desc array is marked initdata */
		memcpy(&board, bcm963xx_boards[i], sizeof(board));
		break;
	}

	/* bail out if board is not found, will complain later */
	if (!board.name[0]) {
		char name[17];
		memcpy(name, nvram.name, 16);
		name[16] = 0;
		printk(KERN_ERR PFX "unknown bcm963xx board: %s\n",
		       name);
		return;
	}

	/* setup pin multiplexing depending on board enabled device,
	 * this has to be done this early since PCI init is done
	 * inside arch_initcall */
	val = 0;

	if (board.has_pci) {
		bcm63xx_pci_enabled = 1;
		if (BCMCPU_IS_6348())
			val |= GPIO_MODE_6348_G2_PCI;
	}

	if (board.has_pccard) {
		if (BCMCPU_IS_6348())
			val |= GPIO_MODE_6348_G1_MII_PCCARD;
	}

	if (board.has_enet0 && !board.enet0.use_internal_phy) {
		if (BCMCPU_IS_6348())
			val |= GPIO_MODE_6348_G3_EXT_MII |
				GPIO_MODE_6348_G0_EXT_MII;
	}

	if (board.has_enet1 && !board.enet1.use_internal_phy) {
		if (BCMCPU_IS_6348())
			val |= GPIO_MODE_6348_G3_EXT_MII |
				GPIO_MODE_6348_G0_EXT_MII;
	}

	bcm_gpio_writel(val, GPIO_MODE_REG);
}

/*
 * second stage init callback, good time to panic if we couldn't
 * identify on which board we're running since early printk is working
 */
void __init board_setup(void)
{
	if (!board.name[0])
		panic("unable to detect bcm963xx board");
	printk(KERN_INFO PFX "board name: %s\n", board.name);

	/* make sure we're running on expected cpu */
	if (bcm63xx_get_cpu_id() != board.expected_cpu_id)
		panic("unexpected CPU for bcm963xx board");
}

/*
 * return board name for /proc/cpuinfo
 */
const char *board_get_name(void)
{
	return board.name;
}

/*
 * register & return a new board mac address
 */
static int board_get_mac_address(u8 *mac)
{
	u8 *p;
	int count;

	if (mac_addr_used >= nvram.mac_addr_count) {
		printk(KERN_ERR PFX "not enough mac address\n");
		return -ENODEV;
	}

	memcpy(mac, nvram.mac_addr_base, ETH_ALEN);
	p = mac + ETH_ALEN - 1;
	count = mac_addr_used;

	while (count--) {
		do {
			(*p)++;
			if (*p != 0)
				break;
			p--;
		} while (p != mac);
	}

	if (p == mac) {
		printk(KERN_ERR PFX "unable to fetch mac address\n");
		return -ENODEV;
	}

	mac_addr_used++;
	return 0;
}

static struct resource mtd_resources[] = {
	{
		.start		= 0,	/* filled at runtime */
		.end		= 0,	/* filled at runtime */
		.flags		= IORESOURCE_MEM,
	}
};

static struct platform_device mtd_dev = {
	.name			= "bcm963xx-flash",
	.resource		= mtd_resources,
	.num_resources		= ARRAY_SIZE(mtd_resources),
};

/*
 * Register a sane SPROMv2 to make the on-board
 * bcm4318 WLAN work
 */
static struct ssb_sprom bcm63xx_sprom = {
	.revision		= 0x02,
	.board_rev		= 0x17,
	.country_code		= 0x0,
	.ant_available_bg 	= 0x3,
	.pa0b0			= 0x15ae,
	.pa0b1			= 0xfa85,
	.pa0b2			= 0xfe8d,
	.pa1b0			= 0xffff,
	.pa1b1			= 0xffff,
	.pa1b2			= 0xffff,
	.gpio0			= 0xff,
	.gpio1			= 0xff,
	.gpio2			= 0xff,
	.gpio3			= 0xff,
	.maxpwr_bg		= 0x004c,
	.itssi_bg		= 0x00,
	.boardflags_lo		= 0x2848,
	.boardflags_hi		= 0x0000,
};

#define GPIO_CPU_BASE 0
#define GPIO_CPU(gpio) (GPIO_CPU_BASE + (gpio))
#define GPIO_74HC164D_BASE 64
#define GPIO_74HC164D(gpio) (GPIO_74HC164D_BASE + (gpio))

static struct resource neufbox4_74hc164d_data[] __initdata = {
	{
	 .start = GPIO_CPU(6), /* clock */
	 .end = GPIO_CPU(7), /* data */
	 },
};

static struct platform_device neufbox4_74hc164d_dev = {
	.name = "74hc164",
	.resource = neufbox4_74hc164d_data,
	.num_resources = ARRAY_SIZE(neufbox4_74hc164d_data),
};


static struct gpio_led neufbox4_leds_array[] = {
	{.name = "wan",.gpio = GPIO_74HC164D(4),.active_low = 1,},
	{.name = "traffic",.gpio = GPIO_CPU(2),.active_low = 1,},
	{.name = "tel",.gpio = GPIO_74HC164D(3),.active_low = 1,},
	{.name = "tv",.gpio = GPIO_74HC164D(2),.active_low = 1,},
	{.name = "wifi",.gpio = GPIO_CPU(15),.active_low = 1,},
	{.name = "alarm",.gpio = GPIO_74HC164D(0),.active_low = 1,},
	{.name = "red",.gpio = GPIO_CPU(29),.active_low = 1,},
	{.name = "green",.gpio = GPIO_CPU(30),.active_low = 1,},
	{.name = "blue",.gpio = GPIO_CPU(4),.active_low = 1,},
};

static struct gpio_led_platform_data neufbox4_led_data = {
	.num_leds = ARRAY_SIZE(neufbox4_leds_array),
	.leds = neufbox4_leds_array,
};

static struct platform_device neufbox4_gpio_leds_dev = {
	.name = "leds-gpio",
	.id = -1,
	.dev = {
		.platform_data = &neufbox4_led_data,
	}
};

static struct gpio_button neufbox4_buttons[] = {
	{
		.desc		= "clip_button",
		.type		= EV_KEY,
		.code		= BTN_0,
		.threshold	= 5,
		.gpio		= GPIO_CPU(31),
	},
	{
		.desc		= "service_button",
		.type		= EV_KEY,
		.code		= BTN_1,
		.threshold	= 5,
		.gpio		= GPIO_CPU(27),
	},
};

#define NEUFBOX4_BUTTON_INTERVAL	20
static struct gpio_buttons_platform_data neufbox4_gpio_buttons_data = {
	.poll_interval	= NEUFBOX4_BUTTON_INTERVAL,
	.nbuttons = ARRAY_SIZE(neufbox4_buttons),
	.buttons = neufbox4_buttons,
};

static struct platform_device neufbox4_gpio_buttons_dev = {
	.name = "gpio-buttons",
	.id = -1,
	.dev = {
		.platform_data = &neufbox4_gpio_buttons_data,
	}
};

static irqreturn_t neufbox4_reset_handler(int irq, void *dev_id)
{
	printk(KERN_INFO "Restarting system.\n");
	machine_restart(NULL);

	/* This should never be reached. */
	return IRQ_HANDLED;
}

#define NEUFBOX4_PID_OFFSET 0xff80
#define NEUFBOX4_GPIO_RESET GPIO_CPU(34)

/*
 * third stage init callback, register all board devices.
 */
static int __init neufbox4_register_devices(void)
{
	char pid[20];
	u8 *boot_addr;
	u32 val;
	int irq;

	/* read base address of boot chip select (0) */
	val = bcm_mpi_readl(MPI_CSBASE_REG(0));
	val &= MPI_CSBASE_BASE_MASK;
	boot_addr = (u8 *) KSEG1ADDR(val);

	/* extract neufbox4 pid */
	memcpy(pid, boot_addr + NEUFBOX4_PID_OFFSET, sizeof(pid));

	if (memcmp(pid, "NB4-", sizeof("NB4-") - 1)) {
		return -1;
	}

	/* FIXUP:
	 * Foxconn boards: GPIO CPU active_low = 0
	 */
	if (!memcmp(pid, "NB4-FXC-r", sizeof("NB4-FXC-r") - 1)) {
		int i;

		for (i = 0; i < ARRAY_SIZE(neufbox4_leds_array); i++) {
			neufbox4_leds_array[i].active_low =
			    (neufbox4_leds_array[i].gpio >= GPIO_74HC164D_BASE);
		}
	}

	/* Enable Serial leds */
	platform_device_register(&neufbox4_74hc164d_dev);
	platform_device_register(&neufbox4_gpio_leds_dev);
	platform_device_register(&neufbox4_gpio_buttons_dev);

	irq = IRQ_EXT_0;
	/* FIXUP:
	 * Foxconn boards NB4-FXC-r0
	 */
	if (!memcmp(pid, "NB4-FXC-r0", sizeof("NB4-FXC-r0") - 1))
		irq = IRQ_EXT_3;

	gpio_request(NEUFBOX4_GPIO_RESET, "reset button");
	gpio_direction_input(NEUFBOX4_GPIO_RESET);
	request_irq(irq, neufbox4_reset_handler,
		    IRQF_DISABLED | IRQF_TRIGGER_LOW, "reset button", NULL);

	return 0;
}

int __init board_register_devices(void)
{
	u32 val;

	bcm63xx_uart_register();
	bcm63xx_wdt_register();
	bcm63xx_spi_register();

	if (board.has_pccard)
		bcm63xx_pcmcia_register();

	if (board.has_enet0 &&
	    !board_get_mac_address(board.enet0.mac_addr))
		bcm63xx_enet_register(0, &board.enet0);

	if (board.has_enet1 &&
	    !board_get_mac_address(board.enet1.mac_addr))
		bcm63xx_enet_register(1, &board.enet1);

	if (board.has_ohci0)
		bcm63xx_ohci_register();

	if (board.has_ehci0)
		bcm63xx_ehci_register();

	if (board.has_udc0)
		bcm63xx_udc_register();
	/* Generate MAC address for WLAN and
	 * register our SPROM */
	if (!board_get_mac_address(bcm63xx_sprom.il0mac)) {
		memcpy(bcm63xx_sprom.et0mac, bcm63xx_sprom.il0mac, ETH_ALEN);
		memcpy(bcm63xx_sprom.et1mac, bcm63xx_sprom.il0mac, ETH_ALEN);
		if (ssb_arch_set_fallback_sprom(&bcm63xx_sprom) < 0)
			printk(KERN_ERR "failed to register fallback SPROM\n");
	}

	/* read base address of boot chip select (0) */
	val = bcm_mpi_readl(MPI_CSBASE_REG(0));
	val &= MPI_CSBASE_BASE_MASK;
	mtd_resources[0].start = val;
	mtd_resources[0].end = 0x1FFFFFFF;

	platform_device_register(&mtd_dev);

	if (BCMCPU_IS_6358()) {
		neufbox4_register_devices();
	}

	return 0;
}

