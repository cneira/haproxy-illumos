/*
 * include/types/connection.h
 * This file describes the connection struct and associated constants.
 *
 * Copyright (C) 2000-2012 Willy Tarreau - w@1wt.eu
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation, version 2.1
 * exclusively.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef _TYPES_CONNECTION_H
#define _TYPES_CONNECTION_H

#include <stdlib.h>
#include <sys/socket.h>

#include <common/config.h>

/* referenced below */
struct sock_ops;
struct protocol;

/* Polling flags that are manipulated by I/O callbacks and handshake callbacks
 * indicate what they expect from a file descriptor at each layer. For each
 * direction, we have 2 bits, one stating whether any suspected activity on the
 * FD induce a call to the iocb, and another one indicating that the FD has
 * already returned EAGAIN and that polling on it is essential before calling
 * the iocb again :
 *   POL ENA  state
 *    0   0   STOPPED : any activity on this FD is ignored
 *    0   1   ENABLED : any (suspected) activity may call the iocb
 *    1   0   STOPPED : as above
 *    1   1   POLLED  : the FD is being polled for activity
 *
 * - Enabling an I/O event consists in ORing with 1.
 * - Stopping an I/O event consists in ANDing with ~1.
 * - Polling for an I/O event consists in ORing with ~3.
 *
 * The last computed state is remembered in CO_FL_CURR_* so that differential
 * changes can be applied. For pollers that do not support speculative I/O,
 * POLLED is the same as ENABLED and the POL flag can safely be ignored.
 */

/* flags for use in connection->flags */
enum {
	CO_FL_NONE          = 0x00000000,
	CO_FL_ERROR         = 0x00000001,  /* a fatal error was reported     */
	CO_FL_CONNECTED     = 0x00000002,  /* the connection is now established */
	CO_FL_WAIT_L4_CONN  = 0x00000004,  /* waiting for L4 to be connected */
	CO_FL_WAIT_L6_CONN  = 0x00000008,  /* waiting for L6 to be connected (eg: SSL) */

	CO_FL_NOTIFY_SI     = 0x00000010,  /* notify stream interface about changes */

	/* flags below are used for connection handshakes */
	CO_FL_SI_SEND_PROXY = 0x00000020,  /* send a valid PROXY protocol header */

	/* below we have all handshake flags grouped into one */
	CO_FL_HANDSHAKE     = CO_FL_SI_SEND_PROXY,

	/* when any of these flags is set, polling is defined by socket-layer
	 * operations, as opposed to data-layer.
	 */
	CO_FL_POLL_SOCK     = CO_FL_HANDSHAKE | CO_FL_WAIT_L4_CONN | CO_FL_WAIT_L6_CONN,

	/* flags used to remember what shutdown have been performed/reported */
	CO_FL_DATA_RD_SH    = 0x00010000,  /* DATA layer was notified about shutr/read0 */
	CO_FL_DATA_WR_SH    = 0x00020000,  /* DATA layer asked for shutw */
	CO_FL_SOCK_RD_SH    = 0x00040000,  /* SOCK layer was notified about shutr/read0 */
	CO_FL_SOCK_WR_SH    = 0x00080000,  /* SOCK layer asked for shutw */

	/****** NOTE: do not change the values of the flags below ******/
	CO_FL_RD_ENA = 1, CO_FL_RD_POL = 2, CO_FL_WR_ENA = 4, CO_FL_WR_POL = 8,

	/* flags describing the DATA layer expectations regarding polling */
	CO_FL_DATA_RD_ENA   = CO_FL_RD_ENA << 20,  /* receiving is allowed */
	CO_FL_DATA_RD_POL   = CO_FL_RD_POL << 20,  /* receiving needs to poll first */
	CO_FL_DATA_WR_ENA   = CO_FL_WR_ENA << 20,  /* sending is desired */
	CO_FL_DATA_WR_POL   = CO_FL_WR_POL << 20,  /* sending needs to poll first */

	/* flags describing the SOCK layer expectations regarding polling */
	CO_FL_SOCK_RD_ENA   = CO_FL_RD_ENA << 24,  /* receiving is allowed */
	CO_FL_SOCK_RD_POL   = CO_FL_RD_POL << 24,  /* receiving needs to poll first */
	CO_FL_SOCK_WR_ENA   = CO_FL_WR_ENA << 24,  /* sending is desired */
	CO_FL_SOCK_WR_POL   = CO_FL_WR_POL << 24,  /* sending needs to poll first */

	/* flags storing the current polling state */
	CO_FL_CURR_RD_ENA   = CO_FL_RD_ENA << 28,  /* receiving is allowed */
	CO_FL_CURR_RD_POL   = CO_FL_RD_POL << 28,  /* receiving needs to poll first */
	CO_FL_CURR_WR_ENA   = CO_FL_WR_ENA << 28,  /* sending is desired */
	CO_FL_CURR_WR_POL   = CO_FL_WR_POL << 28,  /* sending needs to poll first */
};

/* This structure describes a connection with its methods and data.
 * A connection may be performed to proxy or server via a local or remote
 * socket, and can also be made to an internal applet. It can support
 * several data schemes (applet, raw, ssl, ...). It can support several
 * connection control schemes, generally a protocol for socket-oriented
 * connections, but other methods for applets.
 */
struct connection {
	const struct sock_ops *data;  /* operations at the data layer */
	const struct protocol *ctrl;  /* operations at the control layer, generally a protocol */
	union {                       /* definitions which depend on connection type */
		struct {              /*** information used by socket-based connections ***/
			int fd;       /* file descriptor for a stream driver when known */
		} sock;
	} t;
	unsigned int flags;           /* CO_F_* */
	int data_st;                  /* data layer state, initialized to zero */
	void *data_ctx;               /* general purpose pointer, initialized to NULL */
	struct sockaddr *peeraddr;    /* pointer to peer's network address, or NULL if unset */
	socklen_t peerlen;            /* peer's address length, or 0 if unset */
};

#endif /* _TYPES_CONNECTION_H */

/*
 * Local variables:
 *  c-indent-level: 8
 *  c-basic-offset: 8
 * End:
 */