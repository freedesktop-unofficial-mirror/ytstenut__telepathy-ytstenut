/*
 * nosey-status.c - a little demo to print out status/service changes
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

#include <signal.h>

#include <telepathy-glib/util.h>

#include <telepathy-ytstenut-glib/telepathy-ytstenut-glib.h>

static GMainLoop *loop = NULL;
static TpYtsAccountManager *am = NULL;
static TpYtsClient *client = NULL;

static void
getoutofhere (void)
{
  tp_clear_object (&client);
  /* we called Hold in main, so let's let that go here by calling
   * Release */
  tp_yts_account_manager_release (am);
  g_main_loop_quit (loop);
}

static void
status_changed_cb (TpYtsStatus *proxy,
    const gchar *contact_id,
    const gchar *capability,
    const gchar *service_name,
    const gchar *status,
    gpointer user_data,
    GObject *weak_object)
{
  g_print ("StatusChanged:\n");
  g_print (" - contact ID: %s\n", contact_id);
  g_print (" - capability: %s\n", capability);
  g_print (" - service name: %s\n", service_name);
  g_print (" - status: %s\n", status);
}

static void
service_added_cb (TpYtsStatus *proxy,
    const gchar *contact_id,
    const gchar *service_name,
    const GValueArray *service,
    gpointer user_data,
    GObject *weak_object)
{
  const gchar *type;
  GHashTable *names;
  gchar **caps;

  GHashTableIter iter;
  gpointer key, value;

  tp_value_array_unpack ((GValueArray *) service, 3,
      &type, &names, &caps);

  g_print ("ServiceAdded:\n");
  g_print (" - contact ID: %s\n", contact_id);
  g_print (" - service name: %s\n", service_name);

  g_hash_table_iter_init (&iter, names);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      g_print (" - name: %s: %s\n", (const gchar *) key, (const gchar *) value);
    }

  while (caps != NULL && *caps != NULL)
    {
      g_print (" - capability: %s\n", *caps);
      caps++;
    }
}

static void
service_removed_cb (TpYtsStatus *proxy,
    const gchar *contact_id,
    const gchar *service_name,
    gpointer user_data,
    GObject *weak_object)
{
  g_print ("ServiceRemoved:\n");
  g_print (" - contact ID: %s\n", contact_id);
  g_print (" - service name: %s\n", service_name);
}

static void
status_ensured_cb (GObject *source_object,
    GAsyncResult *result,
    gpointer user_data)
{
  TpYtsStatus *status;
  GError *error = NULL;

  status = tp_yts_status_ensure_for_connection_finish (
      TP_CONNECTION (source_object), result, &error);

  if (status == NULL)
    {
      g_printerr ("Failed to ensure status: %s\n", error->message);
      getoutofhere ();
    }
  else
    {
      GHashTable *hash;
      GHashTableIter iter;
      gpointer key, value;

      g_print ("Got status sidecar.\n");

      hash = tp_yts_status_get_discovered_services (status);
      if (g_hash_table_size (hash) > 0)
        {
          g_print ("Discovered services:\n");

          g_hash_table_iter_init (&iter,
              tp_yts_status_get_discovered_services (status));
          while (g_hash_table_iter_next (&iter, &key, &value))
            {
              const gchar *contact_id = key;
              /*GHashTable *map = value;*/

              g_print ("%s:\n", contact_id);

              /* TODO: the rest */
            }
        }

      hash = tp_yts_status_get_discovered_statuses (status);
      if (g_hash_table_size (hash) > 0)
        {
          g_print ("Discovered statuses:\n");

          g_hash_table_iter_init (&iter, hash);
          while (g_hash_table_iter_next (&iter, &key, &value))
            {
              const gchar *contact_id = key;
              /*GHashTable *map = value;*/

              g_print ("%s:\n", contact_id);

              /* TODO: the rest */
            }
        }

      tp_yts_status_connect_to_status_changed (status, status_changed_cb,
          NULL, NULL, NULL, NULL);
      tp_yts_status_connect_to_service_added (status, service_added_cb,
          NULL, NULL, NULL, NULL);
      tp_yts_status_connect_to_service_removed (status, service_removed_cb,
          NULL, NULL, NULL, NULL);
    }

  g_clear_error (&error);
}

static void
connection_prepared_cb (GObject *source_object,
    GAsyncResult *result,
    gpointer user_data)
{
  TpConnection *connection = TP_CONNECTION (source_object);
  TpAccount *account = user_data;
  GError *error = NULL;

  if (!tp_proxy_prepare_finish (source_object, result, &error))
    {
      g_printerr ("Failed to prepare connection: %s\n", error->message);
      getoutofhere ();
    }
  else
    {
      client = tp_yts_client_new ("nosey.status", account);

      tp_yts_client_add_interests (client,
          "urn:ytstenut:capabilities:yts-caps-cats",
          NULL);

      tp_yts_client_register (client, NULL);

      tp_yts_status_ensure_for_connection_async (connection,
          NULL, status_ensured_cb, NULL);
    }

  g_object_unref (account);
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

  g_print ("Trying to prepare account, this will only continue "
      "if the account is connected...\n");

  /* account already reffed */
  tp_proxy_prepare_async (connection, features, connection_prepared_cb,
      account);
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

  g_object_unref (account);
  g_clear_error (&error);
}

static void
sigint_handler (int s)
{
  g_print ("Cleaning up...\n");
  getoutofhere ();
}

int
main (int argc,
    char **argv)
{
  signal (SIGINT, sigint_handler);

  g_type_init ();

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
