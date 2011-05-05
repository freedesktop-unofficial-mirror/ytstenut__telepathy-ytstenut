/*
 * channel - proxy for Ytstenut channel
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

#ifndef TP_YTS_CHANNEL_H
#define TP_YTS_CHANNEL_H

#include "generated.h"

#include <telepathy-glib/channel.h>
#include <telepathy-glib/dbus.h>
#include <telepathy-glib/defs.h>
#include <telepathy-glib/proxy.h>

G_BEGIN_DECLS

typedef struct _TpYtsChannel TpYtsChannel;
typedef struct _TpYtsChannelClass TpYtsChannelClass;
typedef struct _TpYtsChannelPrivate TpYtsChannelPrivate;

struct _TpYtsChannel {
    /*<private>*/
    TpChannel parent;
    TpYtsChannelPrivate *priv;
};

struct _TpYtsChannelClass {
    /*<private>*/
    TpChannelClass parent_class;
    GCallback _padding[7];
};

GType tp_yts_channel_get_type (void);

#define TP_TYPE_YTS_CHANNEL (tp_yts_channel_get_type ())
#define TP_YTS_CHANNEL(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
    TP_TYPE_YTS_CHANNEL, TpYtsChannel))
#define TP_YTS_CHANNEL_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
    TP_TYPE_YTS_CHANNEL, TpYtsChannelClass))
#define TP_IS_YTS_CHANNEL(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
    TP_TYPE_YTS_CHANNEL))
#define TP_IS_YTS_CHANNEL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
    TP_TYPE_YTS_CHANNEL))
#define TP_YTS_CHANNEL_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), \
    TP_TYPE_YTS_CHANNEL, TpYtsChannelClass))

TpChannel *tp_yts_channel_new_from_properties (TpConnection *conn,
    const gchar *object_path, GHashTable *immutable_properties, GError **error);

void tp_yts_channel_request_async (TpYtsChannel *self,
    GCancellable *cancellable, GAsyncReadyCallback callback,
    gpointer user_data);

gboolean tp_yts_channel_request_finish (TpYtsChannel *self,
    GAsyncResult *result, GError **error);

void tp_yts_channel_reply_async (TpYtsChannel *self,
    GHashTable *reply_attributes, const gchar *reply_body,
    GCancellable *cancellable, GAsyncReadyCallback callback,
    gpointer user_data);

gboolean tp_yts_channel_reply_finish (TpYtsChannel *self,
    GAsyncResult *result, GError **error);

void tp_yts_channel_fail_async (TpYtsChannel *self,
    TpYtsErrorType error_type, const gchar *stanza_error_name,
    const gchar *ytstenut_error_name, const gchar *error_text,
    GCancellable *cancellable, GAsyncReadyCallback callback,
    gpointer user_data);

gboolean tp_yts_channel_fail_finish (TpYtsChannel *self,
    GAsyncResult *result, GError **error);

TpYtsRequestType tp_yts_channel_get_request_type (TpYtsChannel *self);
GHashTable * tp_yts_channel_get_request_attributes (TpYtsChannel *self);
const gchar * tp_yts_channel_get_request_body (TpYtsChannel *self);
const gchar * tp_yts_channel_get_target_service (TpYtsChannel *self);
const gchar * tp_yts_channel_get_initiator_service (TpYtsChannel *self);

G_END_DECLS

#include <telepathy-ytstenut-glib/_gen/cli-channel.h>

#endif
