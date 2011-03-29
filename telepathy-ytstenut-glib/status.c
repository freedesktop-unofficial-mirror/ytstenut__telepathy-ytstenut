/*
 * status.c - proxy for Ytstenut service status
 *
 * Copyright (C) 2009 Collabora Ltd. <http://www.collabora.co.uk/>
 * Copyright (C) 2009 Nokia Corporation
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

#include "status.h"

#include <telepathy-glib/defs.h>
#include <telepathy-glib/gtypes.h>
#include <telepathy-glib/interfaces.h>
#include <telepathy-glib/proxy-subclass.h>
#include <telepathy-glib/util.h>

#include "extensions/connection-future.h"
#include "_gen/interfaces.h"
#include "_gen/cli-status-body.h"

#define DEBUG(msg, ...) \
    g_debug ("%s: " msg, G_STRFUNC, ##__VA_ARGS__)

#define IFACE_CONNECTION_FUTURE \
    "org.freedesktop.Telepathy.Connection.FUTURE"

/**
 * SECTION:status
 * @title: TpYtsStatus
 * @short_description: proxy object for Ytstenut service status
 *
 * The #TpYtsStatus object is used to communicate with the Ytstenut
 * Status service.
 */

/**
 * TpYtsStatus:
 *
 * The Ytstenut Status holds information about Ytstenut services and
 * their advertised status. We can also advertise status of services on
 * this device.
 */

/**
 * TpYtsStatusClass:
 *
 * The class of a #TpYtsStatus.
 */

G_DEFINE_TYPE (TpYtsStatus, tp_yts_status, TP_TYPE_PROXY);

static void
tp_yts_status_init (TpYtsStatus *self)
{

}

static void
tp_yts_status_class_init (TpYtsStatusClass *klass)
{
  TpProxyClass *proxy_class = (TpProxyClass *) klass;
  GType tp_type = TP_TYPE_YTS_STATUS;

  proxy_class->interface = TP_YTS_IFACE_QUARK_STATUS;

  tp_proxy_init_known_interfaces ();
  tp_proxy_or_subclass_hook_on_interface_add (tp_type,
      tp_yts_status_add_signals);
  tp_proxy_subclass_add_error_mapping (tp_type,
      TP_ERROR_PREFIX, TP_ERRORS, TP_TYPE_ERROR);
}

void
tp_yts_status_ensure_for_connection_async (TpConnection *connection,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data)
{
  g_return_if_fail (TP_IS_CONNECTION (connection));

  extensions_tp_connection_future_ensure_sidecar_async (connection,
      TP_YTS_IFACE_STATUS, cancellable, callback, user_data);
}

TpYtsStatus *
tp_yts_status_ensure_for_connection_finish (TpConnection *connection,
    GAsyncResult *result,
    GError **error)
{
  TpYtsStatus *status = NULL;
  gchar *object_path;
  GHashTable *immutable_properties;

  g_return_val_if_fail (TP_IS_CONNECTION (connection), NULL);

  object_path = extensions_tp_connection_future_ensure_sidecar_finish (
      connection, result, &immutable_properties, error);

  if (error == NULL)
    {
      status = g_object_new (TP_TYPE_YTS_STATUS,
          "dbus-daemon", tp_proxy_get_dbus_daemon (connection),
          "dbus-connection", tp_proxy_get_dbus_connection (connection),
          "bus-name", tp_proxy_get_bus_name (connection),
          "object-path", object_path,
          "immutable-properties", immutable_properties,
          NULL);
      g_hash_table_unref (immutable_properties);
      g_free (object_path);
    }

  return status;
}

static void
on_status_advertise_status_returned (TpYtsStatus *self,
    const GError *error,
    gpointer user_data,
    GObject *weak_object)
{
  GSimpleAsyncResult *res = G_SIMPLE_ASYNC_RESULT (user_data);

  if (error != NULL)
    {
      DEBUG ("Status.AdvertiseStatus failed: %s", error->message);
      g_simple_async_result_set_from_error (res, error);
    }

  g_simple_async_result_complete (res);
}

void tp_yts_status_advertise_status_async (TpYtsStatus *self,
    const gchar *capability,
    const gchar *service_name,
    const gchar *status_xml,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data)
{
  GSimpleAsyncResult *res;

  g_return_if_fail (TP_IS_YTS_STATUS (self));

  res = g_simple_async_result_new (G_OBJECT (self), callback, user_data,
      tp_yts_status_advertise_status_async);

  tp_yts_status_call_advertise_status (self, -1,
      capability, service_name, status_xml, on_status_advertise_status_returned,
      res, g_object_unref, G_OBJECT (self));
}

gboolean
tp_yts_status_advertise_status_finish (TpYtsStatus *self,
    GAsyncResult *result,
    GError **error)
{
  GSimpleAsyncResult *res;

  g_return_val_if_fail (TP_IS_YTS_STATUS (self), FALSE);
  g_return_val_if_fail (G_IS_SIMPLE_ASYNC_RESULT (result), FALSE);

  res = G_SIMPLE_ASYNC_RESULT (result);
  g_return_val_if_fail (g_simple_async_result_is_valid (result,
          G_OBJECT (self), tp_yts_status_advertise_status_async), FALSE);

  if (g_simple_async_result_propagate_error (res, error))
    return FALSE;

  return TRUE;
}

static void
on_status_get_discovered_statuses_returned (TpProxy *proxy,
    const GValue *value,
    const GError *error,
    gpointer user_data,
    GObject *weak_object)
{
  GSimpleAsyncResult *res = G_SIMPLE_ASYNC_RESULT (user_data);
  GHashTable *statuses;

  if (error == NULL)
    {
      statuses = g_value_dup_boxed (value);
      g_simple_async_result_set_op_res_gpointer (res, statuses,
          (GDestroyNotify)g_hash_table_unref);
    }
  else
    {
      DEBUG ("Failed to get DiscoveredStatuses: %s", error->message);
      g_simple_async_result_set_from_error (res, error);
    }

  g_simple_async_result_complete (res);
}

void
tp_yts_status_get_discovered_statuses_async (TpYtsStatus *self,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data)
{
  GSimpleAsyncResult *res;

  g_return_if_fail (TP_IS_YTS_STATUS (self));

  res = g_simple_async_result_new (G_OBJECT (self), callback, user_data,
      tp_yts_status_get_discovered_statuses_async);

  tp_cli_dbus_properties_call_get (TP_PROXY (self), -1,
      TP_YTS_IFACE_STATUS, "DiscoveredStatus",
      on_status_get_discovered_statuses_returned, res, g_object_unref,
      G_OBJECT (self));

}

GHashTable *
tp_yts_status_get_discovered_statuses_finish (TpYtsStatus *self,
    GAsyncResult *result,
    GError **error)
{
  GSimpleAsyncResult *res;

  g_return_val_if_fail (TP_IS_YTS_STATUS (self), NULL);
  g_return_val_if_fail (G_IS_SIMPLE_ASYNC_RESULT (result), NULL);

  res = G_SIMPLE_ASYNC_RESULT (result);
  g_return_val_if_fail (g_simple_async_result_is_valid (result,
          G_OBJECT (self), tp_yts_status_get_discovered_statuses_async), NULL);

  if (g_simple_async_result_propagate_error (res, error))
    return NULL;

  return g_hash_table_ref (g_simple_async_result_get_op_res_gpointer (res));
}

static void
on_status_get_discovered_services_returned (TpProxy *proxy,
    const GValue *value,
    const GError *error,
    gpointer user_data,
    GObject *weak_object)
{
  GSimpleAsyncResult *res = G_SIMPLE_ASYNC_RESULT (user_data);
  GHashTable *services;

  if (error == NULL)
    {
      services = g_value_dup_boxed (value);
      g_simple_async_result_set_op_res_gpointer (res, services,
          (GDestroyNotify)g_hash_table_unref);
    }
  else
    {
      DEBUG ("Failed to get DiscoveredServices: %s", error->message);
      g_simple_async_result_set_from_error (res, error);
    }

  g_simple_async_result_complete (res);
}

void
tp_yts_status_get_discovered_services_async (TpYtsStatus *self,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data)
{
  GSimpleAsyncResult *res;

  g_return_if_fail (TP_IS_YTS_STATUS (self));

  res = g_simple_async_result_new (G_OBJECT (self), callback, user_data,
      tp_yts_status_get_discovered_services_async);

  tp_cli_dbus_properties_call_get (TP_PROXY (self), -1,
      TP_YTS_IFACE_STATUS, "DiscoveredServices",
      on_status_get_discovered_services_returned, res, g_object_unref,
      G_OBJECT (self));

}

GHashTable *
tp_yts_status_get_discovered_services_finish (TpYtsStatus *self,
    GAsyncResult *result,
    GError **error)
{
  GSimpleAsyncResult *res;

  g_return_val_if_fail (TP_IS_YTS_STATUS (self), NULL);
  g_return_val_if_fail (G_IS_SIMPLE_ASYNC_RESULT (result), NULL);

  res = G_SIMPLE_ASYNC_RESULT (result);
  g_return_val_if_fail (g_simple_async_result_is_valid (result,
          G_OBJECT (self), tp_yts_status_get_discovered_services_async), NULL);

  if (g_simple_async_result_propagate_error (res, error))
    return NULL;

  return g_hash_table_ref (g_simple_async_result_get_op_res_gpointer (res));
}
