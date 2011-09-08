/*
 * account-manager.h - proxy for the Ytstenut account manager
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

#ifndef TP_YTS_ACCOUNT_MANAGER_H
#define TP_YTS_ACCOUNT_MANAGER_H

#include <telepathy-glib/account.h>
#include <telepathy-glib/dbus.h>
#include <telepathy-glib/defs.h>
#include <telepathy-glib/proxy.h>

G_BEGIN_DECLS

#define TP_YTS_ACCOUNT_MANAGER_OBJECT_PATH \
    "/org/freedesktop/ytstenut/xpmn/AccountManager"

typedef struct _TpYtsAccountManager TpYtsAccountManager;
typedef struct _TpYtsAccountManagerClass TpYtsAccountManagerClass;
typedef struct _TpYtsAccountManagerPrivate TpYtsAccountManagerPrivate;

struct _TpYtsAccountManager {
    /*<private>*/
    TpProxy parent;
    TpYtsAccountManagerPrivate *priv;
};

struct _TpYtsAccountManagerClass {
    /*<private>*/
    TpProxyClass parent_class;
    GCallback _padding[7];
};

GType tp_yts_account_manager_get_type (void);

#define TP_TYPE_YTS_ACCOUNT_MANAGER \
  (tp_yts_account_manager_get_type ())
#define TP_YTS_ACCOUNT_MANAGER(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), TP_TYPE_YTS_ACCOUNT_MANAGER, \
        TpYtsAccountManager))
#define TP_YTS_ACCOUNT_MANAGER_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST ((klass), TP_TYPE_YTS_ACCOUNT_MANAGER, \
        TpYtsAccountManagerClass))
#define TP_IS_YTS_ACCOUNT_MANAGER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TP_TYPE_YTS_ACCOUNT_MANAGER))
#define TP_IS_YTS_ACCOUNT_MANAGER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), TP_TYPE_YTS_ACCOUNT_MANAGER))
#define TP_YTS_ACCOUNT_MANAGER_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), TP_TYPE_YTS_ACCOUNT_MANAGER, \
        TpYtsAccountManagerClass))

TpYtsAccountManager *tp_yts_account_manager_new (TpDBusDaemon *bus_daemon)
    G_GNUC_WARN_UNUSED_RESULT;

TpYtsAccountManager *tp_yts_account_manager_dup (void)
    G_GNUC_WARN_UNUSED_RESULT;

void tp_yts_account_manager_get_account_async (TpYtsAccountManager *self,
    GCancellable *cancellable, GAsyncReadyCallback callback,
    gpointer user_data);

TpAccount *tp_yts_account_manager_get_account_finish (TpYtsAccountManager *self,
    GAsyncResult *result, GError **error);

void tp_yts_account_manager_hold (TpYtsAccountManager *self);

void tp_yts_account_manager_release (TpYtsAccountManager *self);

G_END_DECLS

#include <telepathy-ytstenut-glib/_gen/cli-account-manager.h>

#endif
