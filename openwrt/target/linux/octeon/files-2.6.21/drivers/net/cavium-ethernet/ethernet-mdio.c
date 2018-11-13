/*************************************************************************
* Cavium Octeon Ethernet Driver
*
* Author: Cavium Networks info@caviumnetworks.com
*
* Copyright (c) 2003-2007  Cavium Networks (support@cavium.com). All rights
* reserved.
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
*************************************************************************/
#include <linux/kernel.h>
#include <linux/ethtool.h>
#include <linux/mii.h>
#include <net/dst.h>

#include "cvmx-sysinfo.h"
#include "wrapper-cvmx-includes.h"
#include "ethernet-headers.h"


/**
 * Perform an MII read. Called by the generic MII routines
 *
 * @param dev      Device to perform read for
 * @param phy_id   The MII phy id
 * @param location Register location to read
 * @return Result from the read or zero on failure
 */
static int cvm_oct_mdio_read(struct net_device *dev, int phy_id, int location)
{
    cvmx_smi_cmd_t          smi_cmd;
    cvmx_smi_rd_dat_t       smi_rd;

    smi_cmd.u64 = 0;
    smi_cmd.s.phy_op = 1;
    smi_cmd.s.phy_adr = phy_id;
    smi_cmd.s.reg_adr = location;
    cvmx_write_csr(CVMX_SMI_CMD, smi_cmd.u64);

    do
    {
        if (!in_interrupt())
            yield();
        smi_rd.u64 = cvmx_read_csr(CVMX_SMI_RD_DAT);
    } while (smi_rd.s.pending);

    if (smi_rd.s.val)
        return smi_rd.s.dat;
    else
        return 0;
}

static int cvm_oct_mdio_dummy_read(struct net_device *dev, int phy_id, int location)
{
    return 0xffff;
}


/**
 * Perform an MII write. Called by the generic MII routines
 *
 * @param dev      Device to perform write for
 * @param phy_id   The MII phy id
 * @param location Register location to write
 * @param val      Value to write
 */
static void cvm_oct_mdio_write(struct net_device *dev, int phy_id, int location, int val)
{
    cvmx_smi_cmd_t          smi_cmd;
    cvmx_smi_wr_dat_t       smi_wr;

    smi_wr.u64 = 0;
    smi_wr.s.dat = val;
    cvmx_write_csr(CVMX_SMI_WR_DAT, smi_wr.u64);

    smi_cmd.u64 = 0;
    smi_cmd.s.phy_op = 0;
    smi_cmd.s.phy_adr = phy_id;
    smi_cmd.s.reg_adr = location;
    cvmx_write_csr(CVMX_SMI_CMD, smi_cmd.u64);

    do
    {
        if (!in_interrupt())
            yield();
        smi_wr.u64 = cvmx_read_csr(CVMX_SMI_WR_DAT);
    } while (smi_wr.s.pending);
}

static void cvm_oct_mdio_dummy_write(struct net_device *dev, int phy_id, int location, int val)
{
}


static void cvm_oct_get_drvinfo(struct net_device *dev, struct ethtool_drvinfo *info)
{
    strcpy(info->driver, "cavium-ethernet");
    strcpy(info->version, OCTEON_SDK_VERSION_STRING);
    strcpy(info->bus_info, "Builtin");
}


static int cvm_oct_get_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
    cvm_oct_private_t* priv = (cvm_oct_private_t*)netdev_priv(dev);
    return mii_ethtool_gset(&priv->mii_info, cmd);
}


static int cvm_oct_set_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
    cvm_oct_private_t* priv = (cvm_oct_private_t*)netdev_priv(dev);
    return mii_ethtool_sset(&priv->mii_info, cmd);
}


static int cvm_oct_nway_reset(struct net_device *dev)
{
    cvm_oct_private_t* priv = (cvm_oct_private_t*)netdev_priv(dev);
    return mii_nway_restart(&priv->mii_info);
}


static u32 cvm_oct_get_link(struct net_device *dev)
{
    cvm_oct_private_t* priv = (cvm_oct_private_t*)netdev_priv(dev);
    return mii_link_ok(&priv->mii_info);
}


struct ethtool_ops cvm_oct_ethtool_ops = {
    .get_drvinfo    = cvm_oct_get_drvinfo,
    .get_settings   = cvm_oct_get_settings,
    .set_settings   = cvm_oct_set_settings,
    .nway_reset     = cvm_oct_nway_reset,
    .get_link       = cvm_oct_get_link,
    .get_sg         = ethtool_op_get_sg,
    .get_tx_csum    = ethtool_op_get_tx_csum,
};


/**
 * IOCTL support for PHY control
 *
 * @param dev    Device to change
 * @param rq     the request
 * @param cmd    the command
 * @return Zero on success
 */
int cvm_oct_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
    cvm_oct_private_t*      priv = (cvm_oct_private_t*)netdev_priv(dev);
    struct mii_ioctl_data*  data = if_mii(rq);
    unsigned int            duplex_chg;
    return generic_mii_ioctl(&priv->mii_info, data, cmd, &duplex_chg);
}


/**
 * Setup the MDIO device structures
 *
 * @param dev    Device to setup
 *
 * @return Zero on success, negative on failure
 */
int cvm_oct_mdio_setup_device(struct net_device *dev)
{
    cvm_oct_private_t* priv = (cvm_oct_private_t*)netdev_priv(dev);
    int phy_id = cvmx_helper_board_get_mii_address(priv->port);
    if (phy_id != -1)
    {
        priv->mii_info.dev = dev;
        priv->mii_info.phy_id = phy_id;
        priv->mii_info.phy_id_mask = 0xff;
        priv->mii_info.supports_gmii = 1;
        priv->mii_info.reg_num_mask = 0x1f;
        priv->mii_info.mdio_read = cvm_oct_mdio_read;
        priv->mii_info.mdio_write = cvm_oct_mdio_write;
    }
    else
    {
        /* Supply dummy MDIO routines so the kernel won't crash
            if the user tries to read them */
        priv->mii_info.mdio_read = cvm_oct_mdio_dummy_read;
        priv->mii_info.mdio_write = cvm_oct_mdio_dummy_write;
    }
    return 0;
}

