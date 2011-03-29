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
#include "_gen/gtypes.h"
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

enum {
  PROP_0,
  PROP_DISCOVERED_SERVICES,
  PROP_DISCOVERED_STATUSES,
};

struct _TpYtsStatusPrivate {
  TpProxySignalConnection *properties_changed;
  GHashTable *discovered_services;
  GHashTable *discovered_statuses;
};

static void tp_yts_status_async_initable_init (GAsyncInitableIface *iface);

G_DEFINE_TYPE_WITH_CODE (TpYtsStatus, tp_yts_status, TP_TYPE_PROXY,
    G_IMPLEMENT_INTERFACE (G_TYPE_ASYNC_INITABLE,
        tp_yts_status_async_initable_init)
);

static void
tp_yts_status_init (TpYtsStatus *self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, TP_TYPE_YTS_STATUS,
      TpYtsStatusPrivate);

  /* Placeholder values for these tables */
  self->priv->discovered_services = g_hash_table_new (g_str_hash, g_str_equal);
  self->priv->discovered_statuses = g_hash_table_new (g_str_hash, g_str_equal);
}

static void
tp_yts_status_get_property (GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *pspec)
{
  TpYtsStatus *self = TP_YTS_STATUS (object);

  switch (property_id)
    {
      case PROP_DISCOVERED_SERVICES:
        g_value_set_boxed (value, self->priv->discovered_services);
        break;
      case PROP_DISCOVERED_STATUSES:
        g_value_set_boxed (value, self->priv->discovered_statuses);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

static void
tp_yts_status_dispose (GObject *object)
{
  TpYtsStatus *self = TP_YTS_STATUS (object);

  if (self->priv->properties_changed != NULL)
    tp_proxy_signal_connection_disconnect (self->priv->properties_changed);
  self->priv->properties_changed = NULL;

  G_OBJECT_CLASS (tp_yts_status_parent_class)->dispose (object);
}

static void
tp_yts_status_finalize (GObject *object)
{
  TpYtsStatus *self = TP_YTS_STATUS (object);

  g_hash_table_unref (self->priv->discovered_services);
  g_hash_table_unref (self->priv->discovered_statuses);

  G_OBJECT_CLASS (tp_yts_status_parent_class)->finalize (object);
}

static void
tp_yts_status_class_init (TpYtsStatusClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  TpProxyClass *proxy_class = TP_PROXY_CLASS (klass);
  GType tp_type = TP_TYPE_YTS_STATUS;

  object_class->get_property = tp_yts_status_get_property;
  object_class->dispose = tp_yts_status_dispose;
  object_class->finalize = tp_yts_status_finalize;

  proxy_class->interface = TP_YTS_IFACE_QUARK_STATUS;

  g_type_add_class_private (TP_TYPE_YTS_STATUS, sizeof (TpYtsStatus));

  g_object_class_install_property (object_class, PROP_DISCOVERED_SERVICES,
      g_param_spec_boxed ("discovered-services", "Discovered Services",
          "Discovered Ytstenut Service Information",
          TP_YTS_HASH_TYPE_SERVICE_MAP,
          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_DISCOVERED_STATUSES,
      g_param_spec_boxed ("discovered-statuses", "Discovered Statuses",
          "Discovered Ytstenut Status Information",
          TP_YTS_HASH_TYPE_CONTACT_CAPABILITY_MAP,
          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  tp_proxy_init_known_interfaces ();
  tp_proxy_or_subclass_hook_on_interface_add (tp_type,
      tp_yts_status_add_signals);
  tp_proxy_subclass_add_error_mapping (tp_type,
      TP_ERROR_PREFIX, TP_ERRORS, TP_TYPE_ERROR);
}

static void
on_properties_changed (TpProxy *proxy,
    const gchar *interface_name,
    GHashTable *changed_properties,
    const gchar **invalidated_properties,
    gpointer user_data,
    GObject *weak_object)
{
  TpYtsStatus *self = TP_YTS_STATUS (proxy);
  GHashTableIter iter;
  gpointer key;
  gpointer value;

  g_hash_table_iter_init (&iter, changed_properties);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      if (!tp_strdiff (key, "DiscoveredServices"))
        {
          g_hash_table_unref (self->priv->discovered_services);
          self->priv->discovered_services = g_value_dup_boxed (value);
          g_object_notify (G_OBJECT (self), "discovered-services");
        }
      if (!tp_strdiff (key, "DiscoveredStatuses"))
        {
          g_hash_table_unref (self->priv->discovered_statuses);
          self->priv->discovered_statuses = g_value_dup_boxed (value);
          g_object_notify (G_OBJECT (self), "discovered-statuses");
        }
    }
}

static void
on_properties_get_all_returned (TpProxy *proxy,
    GHashTable *properties,
    const GError *error,
    gpointer user_data,
    GObject *weak_object)
{
  GSimpleAsyncResult *res = G_SIMPLE_ASYNC_RESULT (user_data);

  if (error == NULL)
    {
      /* Feed these properties in as if we received a change notificaiton */
      on_properties_changed (proxy, TP_YTS_IFACE_STATUS, properties, NULL, NULL,
          NULL);
    }
  else
    {
      g_simple_async_result_set_from_error (res, error);
    }

  g_simple_async_result_complete (res);
}

static void
tp_yts_status_init_async (GAsyncInitable *initable,
    int io_priority,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data)
{
  TpYtsStatus *self = TP_YTS_STATUS (initable);
  GSimpleAsyncResult *res;
  GError *error = NULL;

  res = g_simple_async_result_new (G_OBJECT (self), callback, user_data,
      tp_yts_status_init_async);

  self->priv->properties_changed =
      tp_cli_dbus_properties_connect_to_properties_changed (self,
          on_properties_changed, NULL, NULL, NULL, &error);

  if (error != NULL)
    {
      g_simple_async_result_set_from_error (res, error);
      g_clear_error (&error);
      g_simple_async_result_complete_in_idle (res);
      return;
    }

  tp_cli_dbus_properties_call_get_all (self, -1, TP_YTS_IFACE_STATUS,
      on_properties_get_all_returned, res, g_object_unref, G_OBJECT (self));
}

static gboolean
tp_yts_status_init_finish (GAsyncInitable *initable,
    GAsyncResult *res,
    GError **error)
{
  if (g_simple_async_result_propagate_error (G_SIMPLE_ASYNC_RESULT (res),
      error))
    return FALSE;

  return TRUE;
}

static void
tp_yts_status_async_initable_init (GAsyncInitableIface *iface)
{
  iface->init_async = tp_yts_status_init_async;
  iface->init_finish = tp_yts_status_init_finish;
}

static void
on_status_new_returned (GObject *source_object,
    GAsyncResult *result,
    gpointer user_data)
{
  GSimpleAsyncResult *res = G_SIMPLE_ASYNC_RESULT (user_data);

  g_return_if_fail (TP_IS_YTS_STATUS (source_object));

  g_simple_async_result_set_op_res_gpointer (res, g_object_ref (source_object),
      g_object_unref);
  g_simple_async_result_complete_in_idle (res);

  g_object_unref (res);
}

static void
on_connection_future_ensure_sidecar_returned (GObject *source_object,
    GAsyncResult *result,
    gpointer user_data)
{
  GSimpleAsyncResult *res = G_SIMPLE_ASYNC_RESULT (user_data);
  GHashTable *immutable_properties;
  gchar *object_path;
  GError *error = NULL;

  g_return_if_fail (TP_IS_CONNECTION (source_object));

  object_path = extensions_tp_connection_future_ensure_sidecar_finish (
      TP_CONNECTION (source_object), result, &immutable_properties, &error);

  if (error != NULL)
    {
      g_simple_async_result_set_from_error (res, error);
      g_clear_error (&error);
      g_simple_async_result_complete_in_idle (res);
      g_object_unref (res);
      return;
    }

  g_async_initable_new_async (TP_TYPE_YTS_STATUS, G_PRIORITY_DEFAULT, NULL,
      on_status_new_returned, res,
      "dbus-daemon", tp_proxy_get_dbus_daemon (source_object),
      "dbus-connection", tp_proxy_get_dbus_connection (source_object),
      "bus-name", tp_proxy_get_bus_name (source_object),
      "object-path", object_path,
      "immutable-properties", immutable_properties,
      NULL);

  g_hash_table_unref (immutable_properties);
  g_free (object_path);
}

void
tp_yts_status_ensure_for_connection_async (TpConnection *connection,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data)
{
  GSimpleAsyncResult *res;

  g_return_if_fail (TP_IS_CONNECTION (connection));

  res = g_simple_async_result_new (G_OBJECT (connection), callback, user_data,
      tp_yts_status_ensure_for_connection_async);

  extensions_tp_connection_future_ensure_sidecar_async (connection,
      TP_YTS_IFACE_STATUS, cancellable,
      on_connection_future_ensure_sidecar_returned, res);
}

TpYtsStatus *
tp_yts_status_ensure_for_connection_finish (TpConnection *connection,
    GAsyncResult *result,
    GError **error)
{
  GSimpleAsyncResult *res;

  g_return_val_if_fail (TP_IS_CONNECTION (connection), NULL);
  g_return_val_if_fail (G_IS_SIMPLE_ASYNC_RESULT (result), FALSE);

  res = G_SIMPLE_ASYNC_RESULT (result);
  g_return_val_if_fail (g_simple_async_result_is_valid (result,
          G_OBJECT (connection), tp_yts_status_advertise_status_async), FALSE);

  if (g_simple_async_result_propagate_error (res, error))
    return FALSE;

  return g_object_ref (g_simple_async_result_get_op_res_gpointer (res));
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

GHashTable *
tp_yts_status_get_discovered_statuses (TpYtsStatus *self)
{
  g_return_val_if_fail (TP_IS_YTS_STATUS (self), NULL);
  return self->priv->discovered_statuses;
}

GHashTable *
tp_yts_status_get_discovered_services (TpYtsStatus *self)
{
  g_return_val_if_fail (TP_IS_YTS_STATUS (self), NULL);
  return self->priv->discovered_services;
}
