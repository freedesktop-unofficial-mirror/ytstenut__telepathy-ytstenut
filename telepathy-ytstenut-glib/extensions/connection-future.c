/*
 * connection-future.c - proxy for Connection.FUTURE
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

#include "config.h"

#include "connection-future.h"
#include "extensions.h"

#include <telepathy-glib/defs.h>
#include <telepathy-glib/gtypes.h>
#include <telepathy-glib/interfaces.h>
#include <telepathy-glib/proxy-subclass.h>
#include <telepathy-glib/util.h>

#include "_gen/cli-connection-future-body.h"

#define DEBUG(msg, ...) \
    g_debug ("%s: " msg, G_STRFUNC, ##__VA_ARGS__)

typedef struct {
  gchar *object_path;
  GHashTable *immutable_properties;
} connection_future_ensure_sidecar_values;

static void
free_connection_future_ensure_sidecar_values (gpointer data)
{
  connection_future_ensure_sidecar_values *values = data;
  g_free (values->object_path);
  g_hash_table_unref (values->immutable_properties);
  g_slice_free (connection_future_ensure_sidecar_values, values);
}

static void
on_connection_future_ensure_sidecar_returned (TpConnection *connection,
    const gchar *object_path,
    GHashTable *immutable_properties,
    const GError *error,
    gpointer user_data,
    GObject *weak_object)
{
  GSimpleAsyncResult *res = G_SIMPLE_ASYNC_RESULT (user_data);
  connection_future_ensure_sidecar_values *values;

  if (error == NULL)
    {
      values = g_slice_new (connection_future_ensure_sidecar_values);
      values->object_path = g_strdup (object_path);
      values->immutable_properties = g_hash_table_ref (immutable_properties);
      g_simple_async_result_set_op_res_gpointer (res, values,
          free_connection_future_ensure_sidecar_values);
    }
  else
    {
      DEBUG ("Connection.FUTURE.EnsureSidecar failed: %s", error->message);
      g_simple_async_result_set_from_error (res, error);
    }

  g_simple_async_result_complete_in_idle (res);
}

void
_tp_yts_connection_future_ensure_sidecar_async (TpConnection *connection,
    const gchar *interface,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data)
{
  GSimpleAsyncResult *res;

  g_return_if_fail (TP_IS_CONNECTION (connection));

  if (!tp_proxy_borrow_interface_by_id (TP_PROXY (connection),
      _TP_YTS_IFACE_QUARK_CONNECTION_FUTURE, NULL))
    {
      tp_proxy_add_interface_by_id (TP_PROXY (connection),
          _TP_YTS_IFACE_QUARK_CONNECTION_FUTURE);
      g_signal_connect (connection, "interface-added",
          G_CALLBACK (_tp_yts_cli_connection_future_add_signals), NULL);
    }

  res = g_simple_async_result_new (G_OBJECT (connection), callback, user_data,
      _tp_yts_connection_future_ensure_sidecar_async);

  _tp_yts_cli_connection_future_call_ensure_sidecar (connection, -1,
      interface, on_connection_future_ensure_sidecar_returned,
      res, g_object_unref, G_OBJECT (connection));
}

gchar *
_tp_yts_connection_future_ensure_sidecar_finish (TpConnection *connection,
    GAsyncResult *result,
    GHashTable **immutable_properties,
    GError **error)
{
  connection_future_ensure_sidecar_values *values;
  GSimpleAsyncResult *res;

  g_return_val_if_fail (TP_IS_CONNECTION (connection), NULL);
  g_return_val_if_fail (G_IS_SIMPLE_ASYNC_RESULT (result), NULL);

  res = G_SIMPLE_ASYNC_RESULT (result);
  g_return_val_if_fail (g_simple_async_result_is_valid (result,
      G_OBJECT (connection),
      _tp_yts_connection_future_ensure_sidecar_async), NULL);

  if (g_simple_async_result_propagate_error (res, error))
    return NULL;

  values = g_simple_async_result_get_op_res_gpointer (res);
  if (immutable_properties)
    *immutable_properties = g_hash_table_ref (values->immutable_properties);
  return g_strdup (values->object_path);
}
