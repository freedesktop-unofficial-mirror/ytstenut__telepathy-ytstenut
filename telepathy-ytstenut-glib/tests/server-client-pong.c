/*
 * client-pong.c - return a message when an incoming channel appears
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

#include <telepathy-ytstenut-glib/telepathy-ytstenut-glib.h>

#include <telepathy-glib/account.h>

static GMainLoop *loop = NULL;

static void
getoutofhere (void)
{
  g_main_loop_quit (loop);
}

static void
reply_cb (GObject *source_object,
    GAsyncResult *result,
    gpointer user_data)
{
  TpYtsChannel *channel = TP_YTS_CHANNEL (source_object);
  GError *error = NULL;

  /* Done */

  if (!tp_yts_channel_reply_finish (channel, result, &error))
    g_printerr ("Failed to reply: %s\n", error->message);
  else
    g_print ("Successfully replied\n");

  g_clear_error (&error);
  getoutofhere ();
}

static void
received_channels (TpYtsClient *client,
    gpointer user_data)
{
  TpYtsChannel *channel;

  /* ::received-channels was emitted, so let's reply to each channel
   * which has popped up. */

  g_print ("received channels\n");

  while ((channel = tp_yts_client_accept_channel (client)) != NULL)
    {
      /* We'll create some rubbish to put in the request attribute
       * argument of Reply */
      GHashTable *tmp = g_hash_table_new (g_str_hash, g_str_equal);
      g_hash_table_insert (tmp, "nyan", "cat");

      g_print ("request type: %u\n",
          tp_yts_channel_get_request_type (channel));
      g_print ("request attributes: %p\n",
          tp_yts_channel_get_request_attributes (channel));
      g_print ("request body: %s\n",
          tp_yts_channel_get_request_body (channel));
      g_print ("target service: %s\n",
          tp_yts_channel_get_target_service (channel));
      g_print ("initiator service: %s\n",
          tp_yts_channel_get_initiator_service (channel));

      /* Call Reply() */
      tp_yts_channel_reply_async (channel, tmp, NULL,
          NULL, reply_cb, NULL);
    }
}

static void
got_account (TpAccount *account,
    gpointer user_data)
{
  TpYtsClient *client;

  /* Now we can create the Ytstenut channel handler with service
   * name specified on the command line. */
  client = tp_yts_client_new ((const gchar *) user_data, account);
  g_signal_connect (client, "received-channels",
      G_CALLBACK (received_channels), NULL);

  tp_yts_client_register (client, NULL);

  g_print ("listening...\n");
}

static void
account_prepared_cb (GObject *source_object,
    GAsyncResult *result,
    gpointer user_data)
{
  TpAccount *account = TP_ACCOUNT (source_object);
  GError *error = NULL;

  if (!tp_account_prepare_finish (account, result, &error))
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
  TpYtsAccountManager *am;
  TpAccount *account;
  gchar *path;

  if (argc < 3
      || !tp_dbus_check_valid_interface_name (argv[2], NULL))
    {
      g_print ("usage: %s [account] [service name]\n", argv[0]);
      return 1;
    }

  g_type_init ();

  am = tp_yts_account_manager_dup ();

  path = g_strdup_printf ("%s%s", TP_ACCOUNT_OBJECT_PATH_BASE, argv[1]);
  account = tp_yts_account_manager_ensure_account (am, path, NULL);
  g_free (path);

  tp_account_prepare_async (account, NULL, account_prepared_cb, argv[2]);

  loop = g_main_loop_new (NULL, FALSE);
  g_main_loop_run (loop);

  g_main_loop_unref (loop);

  g_object_unref (account);
  g_object_unref (am);

  return 0;
}
