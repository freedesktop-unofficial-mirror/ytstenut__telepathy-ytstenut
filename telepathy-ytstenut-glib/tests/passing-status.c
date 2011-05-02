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
  /* we called Hold in main, so let's let that go here by calling
   * Release */
  tp_yts_account_manager_release (am);
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
      tp_yts_status_ensure_async (account,
          NULL, status_ensured_cb, NULL);
    }

  g_object_unref (account);
  g_clear_error (&error);
}

int
main (int argc,
    char **argv)
{
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
