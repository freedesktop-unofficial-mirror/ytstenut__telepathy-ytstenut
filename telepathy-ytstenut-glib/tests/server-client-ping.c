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

#include <telepathy-glib/telepathy-glib.h>

static GMainLoop *loop = NULL;

static const gchar *local_service = NULL;
static const gchar *remote_service = NULL;
static const gchar *contact_id = NULL;

static void
getoutofhere (void)
{
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

  g_print ("%u, %s, %s, %s\n", error_type, stanza_error_name, ytstenut_error_name, text);

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
get_contact_cb (TpConnection *connection,
    guint n_contacts,
    TpContact * const *contacts,
    const gchar * const *requested_ids,
    GHashTable *failed_id_errors,
    const GError *error,
    gpointer user_data,
    GObject *weak_object)
{
  TpAccount *account = user_data;

  if (error != NULL)
    {
      g_printerr ("Failed to get contact: %s\n", error->message);
      getoutofhere ();
    }
  else
    {
      TpYtsClient *client;
      TpContact *contact = contacts[0];

      /* Now we have everything prepared to continue, let's create the
       * Ytstenut channel handler with service name specified on the
       * command line. */
      client = tp_yts_client_new (local_service, account);
      tp_yts_client_register (client, NULL);

      /* So let's request a channel to 'contact'. */
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
      TpConnection *connection;
      const gchar * const ids[] = { contact_id, NULL };

      connection = tp_account_get_connection (account);

      g_print ("trying to get TpContact for %s\n", contact_id);

      tp_connection_get_contacts_by_id (connection, 1,
          ids, 0, NULL, get_contact_cb,
          account, NULL, NULL);
    }

  g_clear_error (&error);
}

static void
notify_connection_cb (GObject *gobject,
    GParamSpec *pspec,
    gpointer user_data)
{
  TpAccount *account = TP_ACCOUNT (gobject);
  GQuark features[] = { TP_CONNECTION_FEATURE_CONNECTED, 0 };
  TpConnection *connection = tp_account_get_connection (account);

  if (connection == NULL)
    return;

  g_print ("Trying to prepare connection, this will only continue "
      "if the account is connected...\n");

  /* account already reffed */
  tp_proxy_prepare_async (connection, features, connection_prepared_cb,
      account);
}

static void
got_account (TpAccount *account)
{
  /* We got the account fine, but we need to ensure some features
   * on it so we have the :self-contact property set. */
  TpConnection *connection = tp_account_get_connection (account);

  if (connection != NULL)
    {
      notify_connection_cb (g_object_ref (account), NULL, NULL);
    }
  else
    {
      g_signal_connect (account, "notify::connection",
          G_CALLBACK (notify_connection_cb), g_object_ref (account));
    }
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

  got_account (account);
}

int
main (int argc,
    char **argv)
{
  TpYtsAccountManager *am;
  TpAccount *account;
  gchar *path;

  if (argc < 5
      || !tp_dbus_check_valid_interface_name (argv[3], NULL)
      || !tp_dbus_check_valid_interface_name (argv[4], NULL))
    {
      g_print ("usage: %s [account name] [contact id] [local service name] [remote service name]\n", argv[0]);
      return 1;
    }

  g_type_init ();

  contact_id = argv[2];
  local_service = argv[3];
  remote_service = argv[4];

  am = tp_yts_account_manager_dup ();

  path = g_strdup_printf ("%s%s", TP_ACCOUNT_OBJECT_PATH_BASE, argv[1]);
  account = tp_yts_account_manager_ensure_account (am, path, NULL);
  g_free (path);

  tp_proxy_prepare_async (account, NULL, account_prepared_cb, NULL);

  loop = g_main_loop_new (NULL, FALSE);
  g_main_loop_run (loop);

  g_main_loop_unref (loop);

  g_object_unref (account);
  g_object_unref (am);

  return 0;
}
