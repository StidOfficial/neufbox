/***********************license start***************
 * Copyright (c) 2003-2008  Cavium Networks (support@cavium.com). All rights
 * reserved.
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *
 *     * Neither the name of Cavium Networks nor the names of
 *       its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written
 *       permission.
 *
 * This Software, including technical data, may be subject to U.S.  export
 * control laws, including the U.S.  Export Administration Act and its
 * associated regulations, and may be subject to export or import regulations
 * in other countries.  You warrant that You will comply strictly in all
 * respects with all such regulations and acknowledge that you have the
 * responsibility to obtain licenses to export, re-export or import the
 * Software.
 *
 * TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND CAVIUM NETWORKS MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE, INCLUDING ITS CONDITION, ITS CONFORMITY TO ANY
 * REPRESENTATION OR DESCRIPTION, OR THE EXISTENCE OF ANY LATENT OR PATENT
 * DEFECTS, AND CAVIUM SPECIFICALLY DISCLAIMS ALL IMPLIED (IF ANY) WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR
 * PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET
 * POSSESSION OR CORRESPONDENCE TO DESCRIPTION.  THE ENTIRE RISK ARISING OUT
 * OF USE OR PERFORMANCE OF THE SOFTWARE LIES WITH YOU.
 *
 *
 * For any questions regarding licensing please contact marketing@caviumnetworks.com
 *
 ***********************license end**************************************/






/**
 * @file
 *
 * Helper functions to abstract board specific data about
 * network ports from the rest of the cvmx-helper files.
 *
 * <hr>$Revision: 38140 $<hr>
 */
#ifndef __CVMX_HELPER_BOARD_H__
#define __CVMX_HELPER_BOARD_H__

#ifdef	__cplusplus
extern "C" {
#endif


typedef enum {
    USB_CLOCK_TYPE_REF_12,
    USB_CLOCK_TYPE_REF_24,
    USB_CLOCK_TYPE_REF_48,
    USB_CLOCK_TYPE_CRYSTAL_12,
} cvmx_helper_board_usb_clock_types_t;

/**
 * cvmx_override_board_link_get(int ipd_port) is a function
 * pointer. It is meant to allow customization of the process of
 * talking to a PHY to determine link speed. It is called every
 * time a PHY must be polled for link status. Users should set
 * this pointer to a function before calling any cvmx-helper
 * operations.
 */
extern cvmx_helper_link_info_t (*cvmx_override_board_link_get)(int ipd_port);

/**
 * Return the MII PHY address associated with the given IPD
 * port. A result of -1 means there isn't a MII capable PHY
 * connected to this port. On chips supporting multiple MII
 * busses the bus number is encoded in bits <15:8>.
 *
 * This function must be modifed for every new Octeon board.
 * Internally it uses switch statements based on the cvmx_sysinfo
 * data to determine board types and revisions. It relys on the
 * fact that every Octeon board receives a unique board type
 * enumeration from the bootloader.
 *
 * @param ipd_port Octeon IPD port to get the MII address for.
 *
 * @return MII PHY address and bus number or -1.
 */
extern int cvmx_helper_board_get_mii_address(int ipd_port);

/**
 * This function as a board specific method of changing the PHY
 * speed, duplex, and autonegotiation. This programs the PHY and
 * not Octeon. This can be used to force Octeon's links to
 * specific settings.
 *
 * @param phy_addr  The address of the PHY to program
 * @param enable_autoneg
 *                  Non zero if you want to enable autonegotiaion.
 * @param link_info Link speed to program. If the speed is zero and autonegotiation
 *                  is enabled, all possible negotiation speeds are advertised.
 *
 * @return Zero on success, negative on failure
 */
int cvmx_helper_board_link_set_phy(int phy_addr, int enable_autoneg,
                                   cvmx_helper_link_info_t link_info);

/**
 * @INTERNAL
 * This function is the board specific method of determining an
 * ethernet ports link speed. Most Octeon boards have Marvell PHYs
 * and are handled by the fall through case. This function must be
 * updated for boards that don't have the normal Marvell PHYs.
 *
 * This function must be modifed for every new Octeon board.
 * Internally it uses switch statements based on the cvmx_sysinfo
 * data to determine board types and revisions. It relys on the
 * fact that every Octeon board receives a unique board type
 * enumeration from the bootloader.
 *
 * @param ipd_port IPD input port associated with the port we want to get link
 *                 status for.
 *
 * @return The ports link status. If the link isn't fully resolved, this must
 *         return zero.
 */
extern cvmx_helper_link_info_t __cvmx_helper_board_link_get(int ipd_port);

/**
 * @INTERNAL
 * This function is called by cvmx_helper_interface_probe() after it
 * determines the number of ports Octeon can support on a specific
 * interface. This function is the per board location to override
 * this value. It is called with the number of ports Octeon might
 * support and should return the number of actual ports on the
 * board.
 *
 * This function must be modifed for every new Octeon board.
 * Internally it uses switch statements based on the cvmx_sysinfo
 * data to determine board types and revisions. It relys on the
 * fact that every Octeon board receives a unique board type
 * enumeration from the bootloader.
 *
 * @param interface Interface to probe
 * @param supported_ports
 *                  Number of ports Octeon supports.
 *
 * @return Number of ports the actual board supports. Many times this will
 *         simple be "support_ports".
 */
extern int __cvmx_helper_board_interface_probe(int interface, int supported_ports);

/**
 * @INTERNAL
 * Enable packet input/output from the hardware. This function is
 * called after by cvmx_helper_packet_hardware_enable() to
 * perform board specific initialization. For most boards
 * nothing is needed.
 *
 * @param interface Interface to enable
 *
 * @return Zero on success, negative on failure
 */
extern int __cvmx_helper_board_hardware_enable(int interface);




/**
 * @INTERNAL
 * Gets the clock type used for the USB block based on board type.
 * Used by the USB code for auto configuration of clock type.
 *
 * @return USB clock type enumeration
 */
cvmx_helper_board_usb_clock_types_t __cvmx_helper_board_usb_get_clock_type(void);


/**
 * @INTERNAL
 * Adjusts the number of available USB ports on Octeon based on board
 * specifics.
 * 
 * @param supported_ports expected number of ports based on chip type;
 * 
 *
 * @return number of available usb ports, based on board specifics.
 *         Return value is supported_ports if function does not
 *         override.
 */
int __cvmx_helper_board_usb_get_num_ports(int supported_ports);

#ifdef	__cplusplus
}
#endif

#endif  /* __CVMX_HELPER_BOARD_H__ */
