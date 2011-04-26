/*
 * client-ping.c - send a message to another service
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

static GMainLoop *loop = NULL;
static TpYtsAccountManager *am = NULL;

static const gchar *local_service = NULL;
static const gchar *remote_service = NULL;

static void
getoutofhere (void)
{
  /* we called Hold in main, so let's let that go here by calling
   * Release */
  tp_yts_account_manager_release (am);
  g_main_loop_quit (loop);
}

static void
replied_cb (TpYtsChannel *proxy,
    GHashTable *attributes,
    const gchar *body,
    gpointer user_data,
    GObject *weak_object)
{
  GHashTableIter iter;
  gpointer key, value;

  g_print ("Replied! Here are the reply attributes:\n");

  g_hash_table_iter_init (&iter, attributes);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      g_print (" * %s = %s\n", (const gchar *) key, (const gchar *) value);
    }

  g_print ("body: %s\n", body);

  getoutofhere ();
}

static void
failed_cb (TpYtsChannel *proxy,
    guint error_type,
    const gchar *stanza_error_name,
    const gchar *ytstenut_error_name,
    const gchar *text,
    gpointer user_data,
    GObject *weak_object)
{
  g_print ("Got a fail!\n");

  g_print ("%s, %s, %s\n", stanza_error_name, ytstenut_error_name, text);

  getoutofhere ();
}

static void
closed_cb (TpChannel *channel,
    gpointer user_data,
    GObject *weak_object)
{
  g_print ("Channel closed\n");

  getoutofhere ();
}

static void
request_cb (GObject *source_object,
    GAsyncResult *result,
    gpointer user_data)
{
  GError *error = NULL;

  if (!tp_yts_channel_request_finish (
          TP_YTS_CHANNEL (source_object), result, &error))
    {
      g_printerr ("Failed to Request on channel: %s\n", error->message);
      getoutofhere ();
    }
  else
    {
      /* The channel was successfully sent to the other side, let's
       * see what happens now. */
      g_print ("Requested! Now we play the waiting game!\n");
    }

  g_clear_error (&error);
}

static void
request_channel_cb (GObject *source_object,
    GAsyncResult *result,
    gpointer user_data)
{
  TpYtsChannel *channel;
  GError *error = NULL;

  channel = tp_yts_client_request_channel_finish (
      TP_YTS_CLIENT (source_object), result, &error);

  if (channel == NULL)
    {
      g_printerr ("Failed to request channel: %s\n", error->message);
      getoutofhere ();
    }
  else
    {
      g_print ("Successfully requested channel!\n");

      /* Connect to these three signals so we can see what's going
       * on. */
      tp_yts_channel_connect_to_replied (channel, replied_cb,
          NULL, NULL, NULL, NULL);
      tp_yts_channel_connect_to_failed (channel, failed_cb,
          NULL, NULL, NULL, NULL);
      tp_cli_channel_connect_to_closed (TP_CHANNEL (channel), closed_cb,
          NULL, NULL, NULL, NULL);

      tp_yts_channel_request_async (channel, NULL, request_cb, NULL);

      g_print ("Request called...\n");
    }

  g_object_unref (source_object);
  g_clear_error (&error);
}

static void
connection_prepared_cb (GObject *source_object,
    GAsyncResult *result,
    gpointer user_data)
{
  TpAccount *account = user_data;
  GError *error = NULL;

  if (!tp_proxy_prepare_finish (source_object, result, &error))
    {
      g_printerr ("Failed to prepare connection: %s\n", error->message);
      getoutofhere ();
    }
  else
    {
      TpContact *contact;
      TpConnection *connection;
      TpYtsClient *client;

      /* Now we have everything prepared to continue, let's create the
       * Ytstenut channel handler with service name specified on the
       * command line. */
      client = tp_yts_client_new (local_service, account);
      tp_yts_client_register (client, NULL);

      connection = tp_account_get_connection (account);

      /* This contact is why we prepared the connection with the
       * CONNECTED feature. */
      contact = tp_connection_get_self_contact (connection);

      /* So let's request a channel to ourselves. It's to ourselves
       * because that's where client-pong is being run (hopefully!) */
      tp_yts_client_request_channel_async (client,
          contact, /* the TpContact we want the channel to go to */
          remote_service, /* the remote service name */
          TP_YTS_REQUEST_TYPE_GET, /* request type */
          NULL, /* an a{ss} of request attributes or nothing */
          NULL, /* the UTF-8 XML body */
          NULL, /* a GCancellable */
          request_channel_cb, /* callback */
          NULL); /* user data */

      g_print ("requested...\n");
    }

  g_object_unref (account);
  g_clear_error (&error);
}

static void
am_get_account_cb (GObject *source_object,
    GAsyncResult *result,
    gpointer user_data)
{
  GError *error = NULL;
  TpAccount *account = tp_yts_account_manager_get_account_finish (
      TP_YTS_ACCOUNT_MANAGER (source_object), result, &error);

  if (account == NULL)
    {
      g_printerr ("Failed to get automatic account: %s\n", error->message);
      getoutofhere ();
    }
  else
    {
      /* We got the account fine, but we need to ensure some features
       * on it so we have the :self-contact property set. */
      GQuark features[] = { TP_CONNECTION_FEATURE_CONNECTED, 0 };
      TpConnection *connection;

      connection = tp_account_get_connection (account);

      g_print ("Trying to prepare account, this will only continue "
          "if the account is connected...\n");

      tp_proxy_prepare_async (connection, features, connection_prepared_cb,
          g_object_ref (account));
    }

  g_object_unref (account);
  g_clear_error (&error);
}

int
main (int argc,
    char **argv)
{
  if (argv[1] == NULL || argv[2] == NULL
      || !tp_dbus_check_valid_interface_name (argv[1], NULL)
      || !tp_dbus_check_valid_interface_name (argv[2], NULL))
    {
      g_print ("usage: %s [local service name] [remote service name]\n", argv[0]);
      return 1;
    }

  g_type_init ();

  local_service = argv[1];
  remote_service = argv[2];

  /* First we need to get the automatic Ytstenut account so create a
   * TpYtsAccountManager and call get_account on it. */
  am = tp_yts_account_manager_dup ();

  /* call Hold on the account so it stays around */
  tp_yts_account_manager_hold (am);

  tp_yts_account_manager_get_account_async (am, NULL,
      am_get_account_cb, NULL);

  loop = g_main_loop_new (NULL, FALSE);
  g_main_loop_run (loop);

  g_main_loop_unref (loop);

  g_object_unref (am);

  return 0;
}
