/* Copyright (C) 1998-99 Martin Baulig
   This file is part of LibGTop 1.0.

   Contributed by Martin Baulig <martin@home-of-linux.org>, August 1998.

   LibGTop is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License,
   or (at your option) any later version.

   LibGTop is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.

   You should have received a copy of the GNU General Public License
   along with LibGTop; see the file COPYING. If not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <config.h>
#include <glibtop.h>
#include <glibtop/error.h>
#include <glibtop/sem_limits.h>

#include <glibtop_suid.h>

#if defined(__bsdi__) && (_BSDI_VERSION < 199700)
/* Older versions of BSDI don't seem to have this. */

void
_glibtop_init_sem_limits_p (glibtop *server)
{ }

void
glibtop_get_sem_limits_p (glibtop *server, glibtop_sem_limits *buf)
{
        glibtop_init_p (server, (1L << GLIBTOP_SYSDEPS_SEM_LIMITS), 0);

        memset (buf, 0, sizeof (glibtop_sem_limits));
}

#else

/* #define KERNEL to get declaration of `struct seminfo'. */

#if (defined(__FreeBSD__) && (__FreeBSD_version < 410000)) || defined(__bsdi__)
#define KERNEL 1
#else
#define _KERNEL 1
#endif

#include <sys/ipc.h>
#include <sys/sem.h>

static unsigned long _glibtop_sysdeps_sem_limits =
(1L << GLIBTOP_IPC_SEMMAP) + (1L << GLIBTOP_IPC_SEMMNI) +
(1L << GLIBTOP_IPC_SEMMNS) + (1L << GLIBTOP_IPC_SEMMNU) +
(1L << GLIBTOP_IPC_SEMMSL) + (1L << GLIBTOP_IPC_SEMOPM) +
(1L << GLIBTOP_IPC_SEMUME) + (1L << GLIBTOP_IPC_SEMUSZ) +
(1L << GLIBTOP_IPC_SEMVMX) + (1L << GLIBTOP_IPC_SEMAEM);

/* The values in this structure never change at runtime, so we only
 * read it once during initialization. We have to use the name `_seminfo'
 * since `seminfo' is already declared external in <sys/sem.h>. */
static struct seminfo _seminfo;

/* nlist structure for kernel access */
static struct nlist nlst [] = {
	{ "_seminfo" },
	{ 0 }
};

/* Init function. */

void
_glibtop_init_sem_limits_p (glibtop *server)
{
	if (kvm_nlist (server->machine->kd, nlst) < 0) {
		glibtop_warn_io_r (server, "kvm_nlist (sem_limits)");
		return;
	}

	if (kvm_read (server->machine->kd, nlst [0].n_value,
		      &_seminfo, sizeof (_seminfo)) != sizeof (_seminfo)) {
		glibtop_warn_io_r (server, "kvm_read (seminfo)");
		return;
	}

	server->sysdeps.sem_limits = _glibtop_sysdeps_sem_limits;
}

/* Provides information about sysv sem limits. */

void
glibtop_get_sem_limits_p (glibtop *server, glibtop_sem_limits *buf)
{
	glibtop_init_p (server, (1L << GLIBTOP_SYSDEPS_SEM_LIMITS), 0);

	memset (buf, 0, sizeof (glibtop_sem_limits));

	if (server->sysdeps.sem_limits == 0)
		return;

	buf->semmap = _seminfo.semmap;
	buf->semmni = _seminfo.semmni;
	buf->semmns = _seminfo.semmns;
	buf->semmnu = _seminfo.semmnu;
	buf->semmsl = _seminfo.semmsl;
	buf->semopm = _seminfo.semopm;
	buf->semvmx = _seminfo.semvmx;
	buf->semaem = _seminfo.semaem;

	buf->flags = _glibtop_sysdeps_sem_limits;
}

#endif /* either a newer BSDI or no BSDI at all. */

