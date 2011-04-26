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
  /* we called Hold in main, so let's let that go here by calling
   * Release */
  tp_yts_account_manager_release (am);
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
am_get_account_cb (GObject *source_object,
    GAsyncResult *result,
    gpointer user_data)
{
  GError *error = NULL;
  TpAccount *account = tp_yts_account_manager_get_account_finish (
      TP_YTS_ACCOUNT_MANAGER (source_object), result, &error);
  const gchar *service = user_data;

  if (account == NULL)
    {
      g_printerr ("Failed to get automatic account: %s\n", error->message);
      getoutofhere ();
    }
  else
    {
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
    }

  g_object_unref (account);
  g_clear_error (&error);
}

int
main (int argc,
    char **argv)
{
  if (argv[1] == NULL || !tp_dbus_check_valid_interface_name (argv[1], NULL))
    {
      g_print ("usage: %s [service name]\n", argv[0]);
      return 1;
    }

  g_type_init ();

  /* First we need to get the automatic Ytstenut account so create a
   * TpYtsAccountManager and call get_account on it. */
  am = tp_yts_account_manager_dup ();

  /* call Hold on the account so it stays around */
  tp_yts_account_manager_hold (am);

  tp_yts_account_manager_get_account_async (am, NULL,
      am_get_account_cb, argv[1]);

  loop = g_main_loop_new (NULL, FALSE);
  g_main_loop_run (loop);

  g_main_loop_unref (loop);

  g_object_unref (am);

  return 0;
}
