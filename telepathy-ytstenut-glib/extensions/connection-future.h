/*
 * connection-future.h - proxy for Connection.FUTURE
 *
 * Copyright (C) 2011 Intel Corp.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef EXTENSIONS_CONNECTION_FUTURE_H
#define EXTENSIONS_CONNECTION_FUTURE_H

#include <telepathy-glib/connection.h>
#include <telepathy-glib/dbus.h>
#include <telepathy-glib/defs.h>
#include <telepathy-glib/proxy.h>

G_BEGIN_DECLS

void _tp_yts_connection_future_ensure_sidecar_async (
    TpConnection *connection, const gchar *interface, GCancellable *cancellable,
    GAsyncReadyCallback callback, gpointer user_data);

gchar *_tp_yts_connection_future_ensure_sidecar_finish (
    TpConnection *connection, GAsyncResult *result,
    GHashTable **immutable_properties, GError **error);

G_END_DECLS

#include "_gen/cli-connection-future.h"

#endif
