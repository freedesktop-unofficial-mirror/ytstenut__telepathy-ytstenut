/*
 * server-file-transfer.c - send/receive a file to another ytstenut service
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
static const gchar *file_path = NULL;

static void
getoutofhere (void)
{
  g_main_loop_quit (loop);
}

static void
provide_file_cb (GObject *source_object,
    GAsyncResult *result,
    gpointer user_data)
{
  TpFileTransferChannel *channel = TP_FILE_TRANSFER_CHANNEL (source_object);
  GError *error = NULL;

  if (!tp_file_transfer_channel_provide_file_finish (channel, result, &error))
    {
      g_printerr ("failed to provide file: %s\n", error->message);
      g_clear_error (&error);
      getoutofhere ();
    }
}

static void
state_changed_cb (TpFileTransferChannel *channel,
    GParamSpec *pspec,
    gpointer user_data)
{
  TpFileTransferState state = tp_file_transfer_channel_get_state (channel, NULL);

  if (state == TP_FILE_TRANSFER_STATE_ACCEPTED
      && tp_channel_get_requested (TP_CHANNEL (channel)))
    {
      GFile *file = g_file_new_for_path (file_path);

      tp_file_transfer_channel_provide_file_async (channel, file,
          provide_file_cb, NULL);

      g_object_unref (file);
    }
  else if (state == TP_FILE_TRANSFER_STATE_COMPLETED)
    {
      g_print ("transfer complete!\n");
      getoutofhere ();
    }
}

static void
transferred_bytes_cb (TpFileTransferChannel *channel,
    GParamSpec *pspec,
    gpointer user_data)
{
  guint64 bytes = tp_file_transfer_channel_get_transferred_bytes (channel);

  g_print ("transferred %" G_GUINT64_FORMAT " bytes...\n", bytes);
}

static void
create_and_handle_cb (GObject *source,
    GAsyncResult *result,
    gpointer user_data)
{
  TpAccountChannelRequest *channel_request = TP_ACCOUNT_CHANNEL_REQUEST (source);
  GError *error = NULL;

  TpChannel *channel = tp_account_channel_request_create_and_handle_channel_finish (
      channel_request, result, NULL, &error);

  if (channel == NULL)
    {
      g_printerr ("failed to create channel: %s\n", error->message);
      g_clear_error (&error);
      getoutofhere ();
      return;
    }

  g_signal_connect (channel, "notify::state",
      G_CALLBACK (state_changed_cb), NULL);
  g_signal_connect (channel, "notify::transferred-bytes",
      G_CALLBACK (transferred_bytes_cb), NULL);

}

static void
got_account (TpAccount *account)
{
  TpYtsClient *client;
  GHashTable *request;
  TpAccountChannelRequest *channel_request;
  GFile *file;
  GFileInfo *info;
  GError *error = NULL;

  const gchar *name, *mimetype;
  GTimeVal mtime;
  goffset filesize;

  g_print ("requesting FT channel...\n");

  file = g_file_new_for_path (file_path);
  info = g_file_query_info (file, "*", G_FILE_QUERY_INFO_NONE, NULL, &error);

  if (info == NULL)
    {
      g_printerr ("failed to get information for file: %s\n", error->message);
      goto out;
    }

  name = g_file_info_get_name (info);
  mimetype = g_file_info_get_content_type (info);
  g_file_info_get_modification_time (info, &mtime);
  filesize = g_file_info_get_size (info);

  /* Now we have everything prepared to continue, let's create the
   * Ytstenut channel handler with service name specified on the
   * command line. */
  client = tp_yts_client_new (local_service, account);
  tp_yts_client_register (client, NULL);

  /* So let's request a channel. */
  request = tp_asv_new (
      TP_PROP_CHANNEL_CHANNEL_TYPE, G_TYPE_STRING, TP_IFACE_CHANNEL_TYPE_FILE_TRANSFER,
      TP_PROP_CHANNEL_TARGET_HANDLE_TYPE, G_TYPE_UINT, TP_HANDLE_TYPE_CONTACT,
      TP_PROP_CHANNEL_TARGET_ID, G_TYPE_STRING, contact_id,
      TP_PROP_CHANNEL_TYPE_FILE_TRANSFER_CONTENT_TYPE, G_TYPE_STRING, mimetype,
      TP_PROP_CHANNEL_TYPE_FILE_TRANSFER_DATE, G_TYPE_INT64, (gint64) mtime.tv_sec,
      TP_PROP_CHANNEL_TYPE_FILE_TRANSFER_DESCRIPTION, G_TYPE_STRING, "",
      TP_PROP_CHANNEL_TYPE_FILE_TRANSFER_FILENAME, G_TYPE_STRING, name,
      TP_PROP_CHANNEL_TYPE_FILE_TRANSFER_INITIAL_OFFSET, G_TYPE_UINT64, (guint64) 0,
      TP_PROP_CHANNEL_TYPE_FILE_TRANSFER_SIZE, G_TYPE_UINT64, (guint64) filesize,
      /* here is the remote service */
      TP_PROP_CHANNEL_INTERFACE_FILE_TRANSFER_METADATA_SERVICE_NAME, G_TYPE_STRING, remote_service,
      NULL);
  channel_request = tp_account_channel_request_new (account, request,
      TP_USER_ACTION_TIME_CURRENT_TIME);
  g_hash_table_unref (request);

  tp_account_channel_request_create_and_handle_channel_async (channel_request, NULL,
      create_and_handle_cb, NULL);

out:
  g_object_unref (file);
  g_clear_error (&error);
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

static void
accept_cb (GObject *source,
    GAsyncResult *result,
    gpointer user_data)
{
  TpFileTransferChannel *chan = TP_FILE_TRANSFER_CHANNEL (source);
  GError *error = NULL;

  if (!tp_file_transfer_channel_accept_file_finish (chan, result, &error))
    {
      g_printerr ("Failed to accept transfer: %s\n", error->message);
      g_clear_error (&error);
    }
  else
    {
      g_print ("saving file to: %s\n", file_path);
    }
}

static void
handle_channels_cb (TpSimpleHandler *handler,
    TpAccount *account,
    TpConnection *connection,
    GList *channels,
    GList *requests_satisfied,
    gint64 user_action_time,
    TpHandleChannelsContext *context,
    gpointer user_data)
{
  for (; channels != NULL; channels = channels->next)
    {
      TpFileTransferChannel *chan = channels->data;
      GFile *file;

      if (!TP_IS_FILE_TRANSFER_CHANNEL (chan))
        continue;

      g_signal_connect (chan, "notify::state",
          G_CALLBACK (state_changed_cb), NULL);
      g_signal_connect (chan, "notify::transferred-bytes",
          G_CALLBACK (transferred_bytes_cb), NULL);

      g_print ("handling file transfer channel: %s\n",
          tp_proxy_get_object_path (chan));

      /* this is ugly, but it's only an example */
      file_path = "/tmp/test-ft";
      file = g_file_new_for_path (file_path);

      tp_file_transfer_channel_accept_file_async (chan,
          file, 0, accept_cb, NULL);
    }

  tp_handle_channels_context_accept (context);
}

int
main (int argc,
    char **argv)
{
  TpYtsAccountManager *am;
  TpBaseClient *handler = NULL;
  TpAccount *account = NULL;
  gchar *path;

  if (argc < 5
      || !tp_dbus_check_valid_interface_name (argv[3], NULL)
      || !tp_dbus_check_valid_interface_name (argv[4], NULL))
    {
      g_print ("usage: %s [account name] [contact id] [local service name] [remote service name] [file to send]\n", argv[0]);
      return 1;
    }

  g_type_init ();

  contact_id = argv[2];
  local_service = argv[3];
  remote_service = argv[4];
  file_path = argv[5]; /* will be NULL when not provided because argv is NULL-terminated */

  am = tp_yts_account_manager_dup ();

  if (file_path != NULL)
    {
      /* sending */
      path = g_strdup_printf ("%s%s", TP_ACCOUNT_OBJECT_PATH_BASE, argv[1]);
      account = tp_yts_account_manager_ensure_account (am, path, NULL);
      g_free (path);

      tp_proxy_prepare_async (account, NULL, account_prepared_cb, NULL);
    }
  else
    {
      /* receiving */
      handler = tp_simple_handler_new_with_factory (tp_proxy_get_factory (am),
          TRUE, FALSE, local_service,
          TRUE, handle_channels_cb, NULL, NULL);

      tp_base_client_take_handler_filter (handler, tp_asv_new (
              TP_PROP_CHANNEL_CHANNEL_TYPE, G_TYPE_STRING, TP_IFACE_CHANNEL_TYPE_FILE_TRANSFER,
              TP_PROP_CHANNEL_TARGET_HANDLE_TYPE, G_TYPE_UINT, TP_HANDLE_TYPE_CONTACT,
              TP_PROP_CHANNEL_REQUESTED, G_TYPE_BOOLEAN, FALSE,
              TP_PROP_CHANNEL_INTERFACE_FILE_TRANSFER_METADATA_SERVICE_NAME, G_TYPE_STRING, local_service,
              NULL));

      tp_base_client_register (handler, NULL);

      g_print ("waiting for a file transfer channel...\n");
    }

  loop = g_main_loop_new (NULL, FALSE);
  g_main_loop_run (loop);

  g_main_loop_unref (loop);

  if (account != NULL)
    g_object_unref (account);
  if (handler != NULL)
    g_object_unref (handler);
  g_object_unref (am);

  return 0;
}
