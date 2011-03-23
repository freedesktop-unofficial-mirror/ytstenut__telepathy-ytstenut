/*
 * diddle-account-manager.c - play around with the Ytstenut account manager
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

static void
on_get_account (GObject *object,
    GAsyncResult *res,
    gpointer user_data)
{
  GError *error = NULL;
  TpAccount *account = NULL;

  g_assert (TP_IS_YTS_ACCOUNT_MANAGER (object));
  g_assert (G_IS_ASYNC_RESULT (res));
  g_assert (user_data == on_get_account);

  account = tp_yts_account_manager_get_account_finish (
      TP_YTS_ACCOUNT_MANAGER (object), res, &error);
  g_assert_no_error (error);
  g_assert (TP_IS_ACCOUNT (account));
  g_object_unref (account);

  g_main_loop_quit (loop);
}

static void
diddle_get_account (TpYtsAccountManager *manager)
{
  tp_yts_account_manager_get_account_async (manager, NULL, on_get_account,
      on_get_account);
}

static gboolean
main_loop_quit_later (gpointer user_data)
{
  g_main_loop_quit (loop);
  return FALSE; /* Remove this source */
}

static void
diddle_hold (TpYtsAccountManager *manager)
{
  tp_yts_account_manager_hold (manager);
  g_timeout_add (100, main_loop_quit_later, NULL);
}

static void
diddle_release (TpYtsAccountManager *manager)
{
  tp_yts_account_manager_release (manager);
  g_timeout_add (100, main_loop_quit_later, NULL);
}

struct {
  const gchar *name;
  void (*diddler) (TpYtsAccountManager *manager);
} the_diddlers[] = {
    { "hold", diddle_hold },
    { "get-account", diddle_get_account },
    { "release", diddle_release },
};

int
main (int argc, const char *argv[])
{
  TpYtsAccountManager *manager;
  guint i;

  g_type_init ();
  loop = g_main_loop_new (NULL, FALSE);

  manager = tp_yts_account_manager_dup ();
  g_assert (TP_IS_YTS_ACCOUNT_MANAGER (manager));

  for (i = 0; i < G_N_ELEMENTS (the_diddlers); ++i)
    {
      g_printerr ("%s ... ", the_diddlers[i].name);
      (the_diddlers[i].diddler) (manager);
      g_main_loop_run (loop);
      g_printerr ("DONE\n");
  }

  g_main_loop_unref (loop);
  return 0;
}
