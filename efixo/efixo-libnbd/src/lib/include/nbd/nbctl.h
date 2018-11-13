/*!
 * \file nbctl.h
 *
 * \note Copyright 2007 Miguel GAIO <miguel.gaio@efixo.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Id: nbctl.h 17908 2010-10-04 07:47:33Z mgo $
 */

#ifndef _NBCTL_H_
#define _NBCTL_H_

#include <stddef.h>

/*! \fn int nbd_sys_reboot (void)
    \brief Clean reboot system.
    \return: 0: success  -1: failure
 */
int nbd_sys_reboot(void);

/*! \fn int nbd_sys_hard_reboot (void)
    \brief Reboot system.
    \return: 0: success  -1: failure
 */
int nbd_sys_hard_reboot(void);

/*! \fn int nbd_sys_dump (char const* file)
    \brief Dump NB.
    \param file export to.
    \return: 0: success  -1: failure
 */
int nbd_sys_dump(char const *file);

/*! \fn int nbd_sys_set_rootfs (char const *target)
    \brief Sets rootfs target to flash, nfs or usb.
    \return: 0: success  -1: failure
 */
int nbd_sys_set_rootfs(char const *target);

/*! \fn int nbd_sys_mac (char const *offset, char const *format, char *data, size_t len)
    \brief Get base mac address with offset in given format.
    \param offset Mac index.
    \param format Output format.
    \param data Buffer to write into.
    \param len Max size.
    \return: 0: success  -1: failure
 */
int nbd_sys_mac(char const *offset, char const *format, char *data, size_t len);

/*! \fn int nbd_sys_delay_run (char const *delay, char const *script, char const *arg)
    \brief Run rc script.
    \param delay Delay.
    \param script Script file.
    \param arg Arg pass to script.
    \return: 0: success  -1: failure
 */
int nbd_sys_delay_run(char const *delay, char const *script, char const *arg);

/*! \fn int nbd_sys_async_run (char const *script, char const *arg)
    \brief Run rc script.
    \param script Script file.
    \param arg Arg pass to script.
    \return: 0: success  -1: failure
 */
int nbd_sys_async_run(char const *script, char const *arg);

/*! \fn int nbd_sys_sync_run (char const* script, char const* arg)
    \brief Run rc script.
    \param script Script file.
    \param arg Arg pass to script.
    \return: 0: success  -1: failure
 */
int nbd_sys_sync_run(char const *script, char const *arg);

/*! \fn int nbd_halt_web_autologin()
    \brief put web_autologin value to off
    \return: 0: success  -1: failure
 */
int nbd_halt_webautologin(void);

/*! \fn int nbd_notify_failure(int errcode)
 *  \brief notify server for failure process (control leds alarm)
 *   \return: 0: success  -1: failure
 */
int nbd_notify_failure(int errcode);

/*! \fn int nbd_gruiks(char **xml, size_t * size)
 *  \brief get xml gruik list
 *   \return: 0: success  -1: failure
 */
int nbd_gruiks(char **xml, size_t * size);

#endif /* _NBCTL_H_ */
