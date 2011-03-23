/*
 * account-manager.c - proxy for the Ytstenut account manager
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

#include "account-manager.h"

#include <telepathy-glib/defs.h>
#include <telepathy-glib/gtypes.h>
#include <telepathy-glib/interfaces.h>
#include <telepathy-glib/proxy-subclass.h>
#include <telepathy-glib/util.h>

#include "_gen/interfaces.h"
#include "_gen/cli-account-manager-body.h"

#define DEBUG(msg, ...) \
    g_debug ("%s: " msg, G_STRFUNC, ##__VA_ARGS__)

/**
 * SECTION:account-manager
 * @title: TpYtsAccountManager
 * @short_description: proxy object for the Ytstenut account manager
 *
 * The #TpYtsAccountManager object is used to communicate with the Ytstenut
 * AccountManager service.
 */

/**
 * TpYtsAccountManager:
 *
 * The Ytstenut Account Manager holds information about the Ytstenut account
 * and controls its presence.
 */

/**
 * TpYtsAccountManagerClass:
 *
 * The class of a #TpYtsAccountManager.
 */

#define MC5_BUS_NAME "org.freedesktop.Telepathy.MissionControl5"

G_DEFINE_TYPE (TpYtsAccountManager, tp_yts_account_manager, TP_TYPE_PROXY);

static void
tp_yts_account_manager_init (TpYtsAccountManager *self)
{

}

static void
tp_yts_account_manager_class_init (TpYtsAccountManagerClass *klass)
{
  TpProxyClass *proxy_class = (TpProxyClass *) klass;
  GType tp_type = TP_TYPE_YTS_ACCOUNT_MANAGER;

  proxy_class->interface = TP_YTS_IFACE_QUARK_ACCOUNT_MANAGER;

  tp_proxy_init_known_interfaces ();
  tp_proxy_or_subclass_hook_on_interface_add (tp_type,
      tp_yts_account_manager_add_signals);
  tp_proxy_subclass_add_error_mapping (tp_type,
      TP_ERROR_PREFIX, TP_ERRORS, TP_TYPE_ERROR);
}

/**
 * tp_yts_account_manager_new:
 * @bus_daemon: Proxy for the D-Bus daemon
 *
 * Convenience function to create a new account manager proxy.
 *
 * Use tp_yts_account_manager_dup() instead if you want an account manager proxy
 * on the starter or session bus (which is almost always the right thing for
 * Telepathy).
 *
 * Returns: a new reference to an account manager proxy
 */
TpYtsAccountManager *
tp_yts_account_manager_new (TpDBusDaemon *bus_daemon)
{
  TpYtsAccountManager *self;

  g_return_val_if_fail (TP_IS_DBUS_DAEMON (bus_daemon), NULL);

  self = TP_YTS_ACCOUNT_MANAGER (g_object_new (TP_TYPE_YTS_ACCOUNT_MANAGER,
          "dbus-daemon", bus_daemon,
          "dbus-connection", ((TpProxy *) bus_daemon)->dbus_connection,
          "bus-name", MC5_BUS_NAME,
          "object-path", TP_YTS_ACCOUNT_MANAGER_OBJECT_PATH,
          NULL));

  return self;
}

static gpointer starter_account_manager_proxy = NULL;

/**
 * tp_yts_account_manager_dup:
 *
 * Returns an account manager proxy on the D-Bus daemon on which this
 * process was activated (if it was launched by D-Bus service activation), or
 * the session bus (otherwise).
 *
 * The returned #TpYtsAccountManager is cached; the same #TpYtsAccountManager
 * object will be returned by this function repeatedly, as long as at least one
 * reference exists.
 *
 * Returns: (transfer full): an account manager proxy on the starter or session
 *          bus, or %NULL if it wasn't possible to get a dbus daemon proxy for
 *          the appropriate bus
 */
TpYtsAccountManager *
tp_yts_account_manager_dup (void)
{
  TpDBusDaemon *dbus;

  if (starter_account_manager_proxy != NULL)
    return g_object_ref (starter_account_manager_proxy);

  dbus = tp_dbus_daemon_dup (NULL);

  if (dbus == NULL)
    return NULL;

  starter_account_manager_proxy = tp_yts_account_manager_new (dbus);
  g_assert (starter_account_manager_proxy != NULL);
  g_object_add_weak_pointer (starter_account_manager_proxy,
      &starter_account_manager_proxy);

  g_object_unref (dbus);

  return starter_account_manager_proxy;
}

static void
on_account_manager_get_account_returned (TpProxy *proxy,
    const GValue *value,
    const GError *error,
    gpointer user_data,
    GObject *weak_object)
{
  GSimpleAsyncResult *res = G_SIMPLE_ASYNC_RESULT (user_data);
  TpAccount *account;
  GError *err = NULL;
  const gchar *path;

  if (error == NULL)
    {
      path = g_value_get_boxed (value);
      account = tp_account_new (tp_proxy_get_dbus_daemon (proxy), path, &err);
      if (err == NULL)
        {
          g_simple_async_result_set_op_res_gpointer (res, account,
              g_object_unref);
        }
      else
        {
          DEBUG ("Failed to create TpAccount: %s", err->message);
          g_simple_async_result_set_from_error (res, err);
          g_clear_error (&err);
        }
    }
  else
    {
      DEBUG ("Failed to get account path: %s", error->message);
      g_simple_async_result_set_from_error (res, error);
    }

  g_simple_async_result_complete (res);
  g_object_unref (res);
}

void
tp_yts_account_manager_get_account_async (TpYtsAccountManager *manager,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data)
{
  GSimpleAsyncResult *res;

  g_return_if_fail (TP_IS_YTS_ACCOUNT_MANAGER (manager));

  res = g_simple_async_result_new (G_OBJECT (manager), callback, user_data,
      tp_yts_account_manager_get_account_async);

  tp_cli_dbus_properties_call_get (TP_PROXY (manager), -1,
      TP_YTS_IFACE_ACCOUNT_MANAGER, "Account",
      on_account_manager_get_account_returned, res, NULL, G_OBJECT (manager));
}

TpAccount *
tp_yts_account_manager_get_account_finish (TpYtsAccountManager *manager,
    GAsyncResult *result,
    GError **error)
{
  GSimpleAsyncResult *res;

  g_return_val_if_fail (TP_IS_YTS_ACCOUNT_MANAGER (manager), NULL);
  g_return_val_if_fail (G_IS_SIMPLE_ASYNC_RESULT (result), NULL);

  res = G_SIMPLE_ASYNC_RESULT (result);
  g_return_val_if_fail (g_simple_async_result_is_valid (result,
          G_OBJECT (manager), tp_yts_account_manager_get_account_async), NULL);

  if (g_simple_async_result_propagate_error (res, error))
    return NULL;

  return g_object_ref (g_simple_async_result_get_op_res_gpointer (res));
}

static void
on_account_manager_hold_returned (TpYtsAccountManager *proxy,
    const GError *error,
    gpointer user_data,
    GObject *weak_object)
{
  if (error != NULL)
    {
      DEBUG ("%s.Hold() call failed: %s", TP_YTS_IFACE_ACCOUNT_MANAGER,
          error->message);
    }
}

void
tp_yts_account_manager_hold (TpYtsAccountManager *manager)
{
  g_return_if_fail (TP_IS_YTS_ACCOUNT_MANAGER (manager));

  tp_yts_account_manager_call_hold (manager, -1,
      on_account_manager_hold_returned, NULL, NULL, G_OBJECT (manager));
}

static void
on_account_manager_release_returned (TpYtsAccountManager *proxy,
    const GError *error,
    gpointer user_data,
    GObject *weak_object)
{
  if (error != NULL)
    {
      DEBUG ("%s.Release() call failed: %s", TP_YTS_IFACE_ACCOUNT_MANAGER,
          error->message);
    }
}

void
tp_yts_account_manager_release (TpYtsAccountManager *manager)
{
  g_return_if_fail (TP_IS_YTS_ACCOUNT_MANAGER (manager));

  tp_yts_account_manager_call_release (manager, -1,
      on_account_manager_release_returned, NULL, NULL, G_OBJECT (manager));
}
