/*
 * passing-service.c - demo to appear as a service and then disappear
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

#include <telepathy-glib/util.h>

#include <telepathy-ytstenut-glib/telepathy-ytstenut-glib.h>

static GMainLoop *loop = NULL;
static TpYtsAccountManager *am = NULL;

static void
getoutofhere (void)
{
  g_main_loop_quit (loop);
}

static gboolean
timeout_cb (gpointer data)
{
  g_object_unref (data);
  getoutofhere ();
  return FALSE;
}

static void
got_account (TpAccount *account,
    gpointer user_data)
{
  const gchar *service = user_data;
  TpYtsClient *client = tp_yts_client_new (service, account);

  tp_yts_client_add_names (client,
      "en_GB", "Magic Icecream Maker",
      "en_US", "Magic Icecream Vendor",
      "fr", "Machine à Glace Magique",
      NULL);

  tp_yts_client_add_capabilities (client,
      "urn:ytstenut:capabilities:video",
      "urn:ytstenut:data:jingle:rtp",
      NULL);

  tp_yts_client_add_interests (client,
      "urn:ytstenut:capabilities:slipping-on-bananaskins",
      NULL);

  tp_yts_client_register (client, NULL);

  g_timeout_add_seconds (10, timeout_cb, client);

  g_object_unref (account);
}

static void
account_prepared_cb (GObject *source_object,
    GAsyncResult *result,
    gpointer user_data)
{
  TpAccount *account = TP_ACCOUNT (source_object);
  GError *error = NULL;

  if (!tp_proxy_prepare_finish (account, result, &error))
    {
      g_print ("failed to prepare account: %s\n", error->message);
      g_clear_error (&error);
      getoutofhere ();
      return;
    }

  got_account (account, user_data);
}

int
main (int argc,
    char **argv)
{
  TpAccount *account;
  gchar *path;

  if (argc < 3 || !tp_dbus_check_valid_interface_name (argv[2], NULL))
    {
      g_print ("usage: %s [account] [service name]\n", argv[0]);
      return 1;
    }

  g_type_init ();

  am = tp_yts_account_manager_dup ();

  path = g_strdup_printf ("%s%s", TP_ACCOUNT_OBJECT_PATH_BASE, argv[1]);
  account = tp_yts_account_manager_ensure_account (am, path, NULL);
  g_free (path);

  tp_proxy_prepare_async (account, NULL, account_prepared_cb, argv[2]);

  loop = g_main_loop_new (NULL, FALSE);
  g_main_loop_run (loop);

  g_main_loop_unref (loop);

  g_object_unref (account);
  g_object_unref (am);

  return 0;
}
