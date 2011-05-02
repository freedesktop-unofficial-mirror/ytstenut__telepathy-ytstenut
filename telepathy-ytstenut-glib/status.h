/*
 * status.h - proxy for Ytstenut service status
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

#ifndef TP_YTS_STATUS_H
#define TP_YTS_STATUS_H

#include <telepathy-glib/account.h>
#include <telepathy-glib/dbus.h>
#include <telepathy-glib/defs.h>
#include <telepathy-glib/proxy.h>

G_BEGIN_DECLS

typedef struct _TpYtsStatus TpYtsStatus;
typedef struct _TpYtsStatusClass TpYtsStatusClass;
typedef struct _TpYtsStatusPrivate TpYtsStatusPrivate;

struct _TpYtsStatus {
    /*<private>*/
    TpProxy parent;
    TpYtsStatusPrivate *priv;
};

struct _TpYtsStatusClass {
    /*<private>*/
    TpProxyClass parent_class;
    GCallback _padding[7];
};

GType tp_yts_status_get_type (void);

#define TP_TYPE_YTS_STATUS (tp_yts_status_get_type ())
#define TP_YTS_STATUS(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
    TP_TYPE_YTS_STATUS, TpYtsStatus))
#define TP_YTS_STATUS_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
    TP_TYPE_YTS_STATUS, TpYtsStatusClass))
#define TP_IS_YTS_STATUS(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
    TP_TYPE_YTS_STATUS))
#define TP_IS_YTS_STATUS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
    TP_TYPE_YTS_STATUS))
#define TP_YTS_STATUS_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), \
    TP_TYPE_YTS_STATUS, TpYtsStatusClass))

void tp_yts_status_ensure_async (TpAccount *account,
    GCancellable *cancellable, GAsyncReadyCallback callback,
    gpointer user_data);

TpYtsStatus *tp_yts_status_ensure_finish (
    TpAccount *account, GAsyncResult *result, GError **error);

void tp_yts_status_advertise_status_async (TpYtsStatus *self,
    const gchar *capability, const gchar *service_name, const gchar *status_xml,
    GCancellable *cancellable, GAsyncReadyCallback callback,
    gpointer user_data);

gboolean tp_yts_status_advertise_status_finish (TpYtsStatus *self,
    GAsyncResult *result, GError **error);

GHashTable *tp_yts_status_get_discovered_statuses (TpYtsStatus *self);

GHashTable *tp_yts_status_get_discovered_services (TpYtsStatus *self);

G_END_DECLS

#include <telepathy-ytstenut-glib/_gen/cli-status.h>

#endif
