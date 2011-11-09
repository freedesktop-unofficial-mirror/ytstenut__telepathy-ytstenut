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
static TpYtsClient *client = NULL;

static void
getoutofhere (void)
{
  tp_clear_object (&client);
  g_main_loop_quit (loop);
}

static gboolean
leave_timeout_cb (gpointer data)
{
  g_print ("Let's go!\n");
  g_object_unref (data);
  getoutofhere ();
  return FALSE;
}

static void
advertise_clear_status_cb (GObject *source_object,
    GAsyncResult *result,
    gpointer user_data)
{
  TpYtsStatus *status = TP_YTS_STATUS (source_object);
  GError *error = NULL;

  if (!tp_yts_status_advertise_status_finish (status, result, &error))
    {
      g_printerr ("Failed to advertise status: %s\n", error->message);
      getoutofhere ();
    }
  else
    {
      g_print ("Cleared status fine...\n");
      g_timeout_add_seconds (3, leave_timeout_cb, status);
    }

  g_clear_error (&error);
}

static gboolean
timeout_cb (gpointer data)
{
  TpYtsStatus *status = data;

  g_print ("Clearing status\n");

  tp_yts_status_advertise_status_async (status,
      "urn:ytstenut:capabilities:yts-caps-cats",
      "passing.status", /* this should be the same as the client name */
      NULL, NULL, advertise_clear_status_cb, NULL);
  return FALSE;
}

static void
advertise_status_cb (GObject *source_object,
    GAsyncResult *result,
    gpointer user_data)
{
  TpYtsStatus *status = TP_YTS_STATUS (source_object);
  GError *error = NULL;

  if (!tp_yts_status_advertise_status_finish (status, result, &error))
    {
      g_printerr ("Failed to advertise status: %s\n", error->message);
      getoutofhere ();
    }
  else
    {
      g_print ("Advertised status fine...\n");
      g_timeout_add_seconds (3, timeout_cb, status);
    }

  g_clear_error (&error);
}

static void
status_ensured_cb (GObject *source_object,
    GAsyncResult *result,
    gpointer user_data)
{
  TpAccount *account = user_data;
  TpYtsStatus *status;
  GError *error = NULL;

  status = tp_yts_status_ensure_finish (
      TP_ACCOUNT (source_object), result, &error);

  if (status == NULL)
    {
      g_printerr ("Failed to ensure status: %s\n", error->message);
      getoutofhere ();
    }
  else
    {
      client = tp_yts_client_new ("passing.status", account);
      tp_yts_client_register (client, NULL);

      /* We need the XML here otherwise the activity attribute won't
       * be present and it'll think we'll be trying to remove the
       * status. */
      tp_yts_status_advertise_status_async (status,
          "urn:ytstenut:capabilities:yts-caps-cats",
          "passing.status", /* this should be the same as the client name */
          "<status xmlns='urn:ytstenut:status' from-service='service.name' "
          "capability='urn:ytstenut:capabilities:yts-caps-cats' "
          "activity='looking-at-cats-ooooooh'><look>at how cute they "
          "are!</look></status>",
          NULL, advertise_status_cb, NULL);
    }

  g_clear_error (&error);
}

static void
got_account (TpAccount *account)
{
  tp_yts_status_ensure_async (account,
      NULL, status_ensured_cb, NULL);

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

  got_account (account);
}

int
main (int argc,
    char **argv)
{
  TpAccount *account;
  gchar *path;

  if (argc < 2)
    {
      g_print ("usage: %s [account]\n", argv[0]);
      return 1;
    }

  g_type_init ();

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
