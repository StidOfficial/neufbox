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
 * File defining functions for working with different Octeon
 * models.
 *
 * <hr>$Revision: 38324 $<hr>
 */
#include "cvmx.h"
#include "cvmx-pow.h"
#include "cvmx-warn.h"

#if defined(CVMX_BUILD_FOR_LINUX_USER) || defined(CVMX_BUILD_FOR_STANDALONE)
#include <octeon-app-init.h>
#include "cvmx-sysinfo.h"

/**
 * This function checks to see if the software is compatible with the
 * chip it is running on.  This is called in the application startup code
 * and does not need to be called directly by the application.
 * Does not return if software is incompatible.
 *
 * @param chip_id chip id that the software is being run on.
 *
 * @return 0: runtime checking or exact version match
 *         1: chip is newer revision than compiled for, but software will run properly.
 */
int octeon_model_version_check(uint32_t chip_id)
{
    //printf("Model Number: %s\n", octeon_model_get_string(chip_id));
#if OCTEON_IS_COMMON_BINARY()
    if (chip_id == OCTEON_CN38XX_PASS1)
    {
        printf("Runtime Octeon Model checking binaries do not support OCTEON_CN38XX_PASS1 chips\n");
#ifdef CVMX_BUILD_FOR_STANDALONE
        if (cvmx_sysinfo_get()->board_type == CVMX_BOARD_TYPE_SIM)
            CVMX_BREAK;
        while (1);
#else
        exit(-1);
#endif
    }
#else
    /* Check for special case of mismarked 3005 samples, and adjust cpuid */
    if (chip_id == OCTEON_CN3010_PASS1 && (cvmx_read_csr(0x80011800800007B8ull) & (1ull << 34)))
        chip_id |= 0x10;

    if ((OCTEON_MODEL & 0xffffff) != chip_id)
    {
        if (!OCTEON_IS_MODEL((OM_IGNORE_REVISION | chip_id)) || (OCTEON_MODEL & 0xffffff) > chip_id || (((OCTEON_MODEL & 0xffffff) ^ chip_id) & 0x10))
        {
            printf("ERROR: Software not configured for this chip\n"
                   "         Expecting ID=0x%08x, Chip is 0x%08x\n", (OCTEON_MODEL & 0xffffff), (unsigned int)chip_id);
            if ((OCTEON_MODEL & 0xffffff) > chip_id)
                printf("Refusing to run on older revision than program was compiled for.\n");
#ifdef CVMX_BUILD_FOR_STANDALONE
            if (cvmx_sysinfo_get()->board_type == CVMX_BOARD_TYPE_SIM)
                CVMX_BREAK;
            while (1);
#else
            exit(-1);
#endif
        }
        else
        {
            printf("\n###################################################\n");
            printf("WARNING: Software configured for older revision than running on.\n"
                   "         Compiled for ID=0x%08x, Chip is 0x%08x\n", (OCTEON_MODEL & 0xffffff), (unsigned int)chip_id);
            printf("###################################################\n\n");
            return(1);
        }
    }
#endif

    cvmx_warn_if(CVMX_ENABLE_PARAMETER_CHECKING, "Parameter checks are enabled. Expect some performance loss due to the extra checking\n");
    cvmx_warn_if(CVMX_ENABLE_CSR_ADDRESS_CHECKING, "CSR address checks are enabled. Expect some performance loss due to the extra checking\n");
    cvmx_warn_if(CVMX_ENABLE_POW_CHECKS, "POW state checks are enabled. Expect some performance loss due to the extra checking\n");

    return(0);
}

#endif
/**
 * Given the chip processor ID from COP0, this function returns a
 * string representing the chip model number. The string is of the
 * form CNXXXXpX.X-FREQ-SUFFIX.
 * - XXXX = The chip model number
 * - X.X = Chip pass number
 * - FREQ = Current frequency in Mhz
 * - SUFFIX = NSP, EXP, SCP, SSP, or CP
 *
 * @param chip_id Chip ID
 *
 * @return Model string
 */
const char *octeon_model_get_string(uint32_t chip_id)
{
    static char         buffer[32];
    return octeon_model_get_string_buffer(chip_id,buffer);
}

/* Version of octeon_model_get_string() that takes buffer as argument, as
** running early in u-boot static/global variables don't work when running from
** flash
*/
const char *octeon_model_get_string_buffer(uint32_t chip_id, char * buffer)
{
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
    extern uint64_t octeon_get_clock_rate(void);
#endif
    const char *        family;
    const char *        core_model;
    char                pass[4];
    int                 clock_mhz;
    const char *        suffix;
    cvmx_l2d_fus3_t     fus3;
    int                 num_cores;
    cvmx_mio_fus_dat2_t fus_dat2;
    cvmx_mio_fus_dat3_t fus_dat3;
    char fuse_model[10];
    uint32_t fuse_data = 0;

    fus3.u64 = cvmx_read_csr(CVMX_L2D_FUS3);
    fus_dat2.u64 = cvmx_read_csr(CVMX_MIO_FUS_DAT2);
    fus_dat3.u64 = cvmx_read_csr(CVMX_MIO_FUS_DAT3);
    num_cores = __builtin_popcount(cvmx_read_csr(CVMX_CIU_FUSE));

    /* Make sure the non existant devices look disabled */
    switch ((chip_id >> 8) & 0xff)
    {
        case 6: /* CN50XX */
        case 2: /* CN30XX */
            fus_dat3.s.nodfa_dte = 1;
            fus_dat3.s.nozip = 1;
            break;
        case 4: /* CN57XX or CN56XX */
            fus_dat3.s.nodfa_dte = 1;
            break;
        default:
            break;
    }

    /* Make a guess at the suffix */
    /* NSP = everything */
    /* EXP = No crypto */
    /* SCP = No DFA, No zip */
    /* CP = No DFA, No crypto, No zip */
    if (fus_dat3.s.nodfa_dte)
    {
        if (fus_dat2.s.nocrypto)
            suffix = "CP";
        else
            suffix = "SCP";
    }
    else if (fus_dat2.s.nocrypto)
        suffix = "EXP";
    else
        suffix = "NSP";

    /* Assume pass number is encoded using <5:3><2:0>. Exceptions will be
        fixed later */
    sprintf(pass, "%d.%d", (int)((chip_id>>3)&7)+1, (int)chip_id&7);

    /* Use the number of cores to determine the last 2 digits of the model
        number. There are some exceptions that are fixed later */
    switch (num_cores)
    {
        case 16: core_model = "60"; break;
        case 15:
        case 14: core_model = "55"; break;
        case 13:
        case 12: core_model = "50"; break;
        case 11:
        case 10: core_model = "45"; break;
        case  9: core_model = "42"; break;
        case  8: core_model = "40"; break;
        case  7:
        case  6: core_model = "34"; break;
        case  5: core_model = "32"; break;
        case  4: core_model = "30"; break;
        case  3: core_model = "25"; break;
        case  2: core_model = "20"; break;
        case  1: core_model = "10"; break;
        default: core_model = "XX"; break;
    }

    /* Now figure out the family, the first two digits */
    switch ((chip_id >> 8) & 0xff)
    {
        case 0: /* CN38XX, CN37XX or CN36XX */
            if (fus3.cn38xx.crip_512k)
            {
                /* For some unknown reason, the 16 core one is called 37 instead of 36 */
                if (num_cores >= 16)
                    family = "37";
                else
                    family = "36";
            }
            else
                family = "38";
            /* This series of chips didn't follow the standard pass numbering */
            switch (chip_id & 0xf)
            {
                case 0: strcpy(pass, "1.X"); break;
                case 1: strcpy(pass, "2.X"); break;
                case 3: strcpy(pass, "3.X"); break;
                default:strcpy(pass, "X.X"); break;
            }
            break;
        case 1: /* CN31XX or CN3020 */
            if ((chip_id & 0x10) || fus3.cn31xx.crip_128k)
                family = "30";
            else
                family = "31";
            /* This series of chips didn't follow the standard pass numbering */
            switch (chip_id & 0xf)
            {
                case 0: strcpy(pass, "1.0"); break;
                case 2: strcpy(pass, "1.1"); break;
                default:strcpy(pass, "X.X"); break;
            }
            break;
        case 2: /* CN3010 or CN3005 */
            family = "30";
            /* A chip with half cache is an 05 */
            if (fus3.cn30xx.crip_64k)
                core_model = "05";
            /* This series of chips didn't follow the standard pass numbering */
            switch (chip_id & 0xf)
            {
                case 0: strcpy(pass, "1.0"); break;
                case 2: strcpy(pass, "1.1"); break;
                default:strcpy(pass, "X.X"); break;
            }
            break;
        case 3: /* CN58XX */
            family = "58";
            /* Special case. 4 core, no crypto */
            if ((num_cores == 4) && fus_dat2.cn38xx.nocrypto)
                core_model = "29";

            /* Pass 1 uses different encodings for pass numbers */
            if ((chip_id & 0xFF)< 0x8)
            {
                switch (chip_id & 0x3)
                {
                    case 0: strcpy(pass, "1.0"); break;
                    case 1: strcpy(pass, "1.1"); break;
                    case 3: strcpy(pass, "1.2"); break;
                    default:strcpy(pass, "1.X"); break;
                }
            }
            break;
        case 4: /* CN57XX, CN56XX, CN55XX, CN54XX */
            if (fus_dat2.cn56xx.raid_en)
            {
                if (fus3.cn56xx.crip_1024k)
                    family = "55";
                else
                    family = "57";
                if (fus_dat2.cn56xx.nocrypto)
                    suffix = "SP";
                else
                    suffix = "SSP";
            }
            else 
            {
                if (fus_dat2.cn56xx.nocrypto)
                    suffix = "CP";
                else
                {
                    suffix = "NSP";
                    if (fus_dat3.s.nozip)
                        suffix = "SCP";
                }
                if (fus3.cn56xx.crip_1024k)
                    family = "54";
                else
                    family = "56";
            }
            break;
        case 6: /* CN50XX */
            family = "50";
            break;
        case 7: /* CN52XX */
            family = "52";
            break;
        case 8: /* CN51XX */
            family = "51";
            break;
        default:
            family = "XX";
            core_model = "XX";
            strcpy(pass, "X.X");
            suffix = "XXX";
            break;
    }

#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
    clock_mhz = octeon_get_clock_rate() / 1000000;
#else
    if (cvmx_sysinfo_get())
        clock_mhz = cvmx_sysinfo_get()->cpu_clock_hz / 1000000;
    else
        clock_mhz = 0;
#endif

    if (family[0] != '3')
    {
        /* Check for model in fuses, overrides normal decode */
        /* This is _not_ valid for Octeon CN3XXX models */
        fuse_data |= cvmx_fuse_read_byte(51);
        fuse_data = fuse_data << 8;
        fuse_data |= cvmx_fuse_read_byte(50);
        fuse_data = fuse_data << 8;
        fuse_data |= cvmx_fuse_read_byte(49);
        fuse_data = fuse_data << 8;
        fuse_data |= cvmx_fuse_read_byte(48);
        if (fuse_data & 0x7ffff)
        {
            int model = fuse_data & 0x3fff;
            int suffix = (fuse_data >> 14) & 0x1f;
            if (suffix && model)  /* Have both number and suffix in fuses, so both */
            {
                sprintf(fuse_model, "%d%c",model, 'A' + suffix - 1);
                core_model = "";
                family = fuse_model;
            }
            else if (suffix && !model)   /* Only have suffix, so add suffix to 'normal' model number */
            {
                sprintf(fuse_model, "%s%c", core_model, 'A' + suffix - 1);
                core_model = fuse_model;
            }
            else /* Don't have suffix, so just use model from fuses */
            {
                sprintf(fuse_model, "%d",model);
                core_model = "";
                family = fuse_model;
            }
        }
    }
#ifdef CVMX_BUILD_FOR_UBOOT
    sprintf(buffer, "CN%s%s-%s pass %s", family, core_model, suffix, pass);
#else
    sprintf(buffer, "CN%s%sp%s-%d-%s", family, core_model, pass, clock_mhz, suffix);
#endif
    return buffer;
}
