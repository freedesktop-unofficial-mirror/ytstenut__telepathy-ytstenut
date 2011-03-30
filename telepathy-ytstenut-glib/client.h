/*
 * client - client side Ytstenut implementation
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

#ifndef TP_YTS_CLIENT_H
#define TP_YTS_CLIENT_H

#include "generated.h"
#include "channel.h"

#include <telepathy-glib/base-client.h>

G_BEGIN_DECLS

typedef struct _TpYtsClient TpYtsClient;
typedef struct _TpYtsClientClass TpYtsClientClass;
typedef struct _TpYtsClientPrivate TpYtsClientPrivate;

struct _TpYtsClient {
  /*<private>*/
  TpBaseClient parent;
  TpYtsClientPrivate *priv;
};

struct _TpYtsClientClass {
  /*<private>*/
  TpBaseClientClass parent_class;
  GCallback _padding[7];
};

GType tp_yts_client_get_type (void);

#define TP_TYPE_YTS_CLIENT (tp_yts_client_get_type ())
#define TP_YTS_CLIENT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
    TP_TYPE_YTS_CLIENT, TpYtsClient))
#define TP_YTS_CLIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
    TP_TYPE_YTS_CLIENT, TpYtsClientClass))
#define TP_IS_YTS_CLIENT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
    TP_TYPE_YTS_CLIENT))
#define TP_IS_YTS_CLIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
    TP_TYPE_YTS_CLIENT))
#define TP_YTS_CLIENT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), \
    TP_TYPE_YTS_CLIENT, TpYtsClientClass))

TpYtsClient *tp_yts_client_new (const gchar *service_name,
    TpAccount *account);

gboolean tp_yts_client_register (TpYtsClient *self, GError **error);

TpYtsChannel *tp_yts_client_accept_channel (TpYtsClient *self);

void tp_yts_client_request_channel_async (TpYtsClient *self,
    TpContact *target_contact, const gchar *target_service,
    TpYtsRequestType request_type, GHashTable *request_attributes,
    const gchar *request_body, GCancellable *cancellable,
    GAsyncReadyCallback callback, gpointer user_data);

TpYtsChannel *tp_yts_client_request_channel_finish (TpYtsClient *self,
    GAsyncResult *result, GError **error);

G_END_DECLS

#endif
