/*
 * status.c - proxy for Ytstenut service status
 *
 * Copyright (C) 2009 Collabora Ltd. <http://www.collabora.co.uk/>
 * Copyright (C) 2009 Nokia Corporation
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

#include "channel.h"

#include <telepathy-glib/defs.h>
#include <telepathy-glib/gtypes.h>
#include <telepathy-glib/interfaces.h>
#include <telepathy-glib/proxy-subclass.h>
#include <telepathy-glib/util.h>

#include "_gen/interfaces.h"
#include "_gen/cli-channel-body.h"

#define DEBUG(msg, ...) \
    g_debug ("%s: " msg, G_STRFUNC, ##__VA_ARGS__)

/**
 * SECTION:channel
 * @title: TpYtsChannel
 * @short_description: proxy object for Ytstenut message channel
 *
 * The #TpYtsChannel proxy object is used to communicate with a Ytstenut
 * Channel implementation.
 *
 * You can use tp_yts_client_request_channel_async() to request new outgoing
 * channels. To receive new incoming channels, use a #TpYtsClient and connect
 * the #TpYtsClient::received-channels signal.
 *
 * On an outgoing channel you should use tp_yts_channel_request_async() to send
 * off the Ytstenut request. To listen for replies or failures use the
 * tp_yts_channel_connect_to_replied() and tp_yts_channel_connect_to_failed()
 * functions respectively.
 *
 * On an incoming channel, you can use the tp_yts_channel_reply_async() or
 * tp_yts_channel_fail_async() functions to send back a Ytstenut reply or
 * error.
 */

/**
 * TpYtsChannel:
 *
 * The Ytstenut Channel holds information about a single Ytstenut request
 * and reply. It should be created using the channel dispatcher, and the
 * relevant properties.
 */

/**
 * TpYtsChannelClass:
 *
 * The class of a #TpYtsChannel.
 */

G_DEFINE_TYPE (TpYtsChannel, tp_yts_channel, TP_TYPE_PROXY);

static void
tp_yts_channel_init (TpYtsChannel *self)
{

}

static void
tp_yts_channel_class_init (TpYtsChannelClass *klass)
{
  TpProxyClass *proxy_class = (TpProxyClass *) klass;
  GType tp_type = TP_TYPE_YTS_CHANNEL;

  proxy_class->interface = TP_YTS_IFACE_QUARK_CHANNEL;

  tp_proxy_init_known_interfaces ();
  tp_proxy_or_subclass_hook_on_interface_add (tp_type,
      tp_yts_channel_add_signals);
  tp_proxy_subclass_add_error_mapping (tp_type,
      TP_ERROR_PREFIX, TP_ERRORS, TP_TYPE_ERROR);
}

/**
 * tp_yts_channel_new_from_properties:
 * @conn: The telepathy connection
 * @object_path: The DBus object path of the channel.
 * @immutable_properties: The immutable properties of the channel.
 * @error: If not %NULL, used to raise an error when %NULL is returned.
 *
 * Create a new #TpYtsChannel proxy object for a channel that exists in the
 * Ytstenut DBus implementation.
 *
 * In order to request a new outgoing channel use
 * tp_yts_client_request_channel_async() instead of this function.
 *
 * Returns: A newly allocated #TpYtsChannel object.
 */
TpChannel *
tp_yts_channel_new_from_properties (TpConnection *conn,
    const gchar *object_path,
    GHashTable *immutable_properties,
    GError **error)
{
  TpProxy *conn_proxy = TP_PROXY (conn);
  TpChannel *ret = NULL;

  g_return_val_if_fail (TP_IS_CONNECTION (conn), NULL);
  g_return_val_if_fail (object_path != NULL, NULL);
  g_return_val_if_fail (immutable_properties != NULL, NULL);

  if (!tp_dbus_check_valid_object_path (object_path, error))
    goto finally;

  ret = TP_CHANNEL (g_object_new (TP_TYPE_CHANNEL,
        "connection", conn,
        "dbus-daemon", conn_proxy->dbus_daemon,
        "bus-name", conn_proxy->bus_name,
        "object-path", object_path,
        "handle-type", (guint) TP_UNKNOWN_HANDLE_TYPE,
        "channel-properties", immutable_properties,
        NULL));

finally:
  return ret;
}

static void
on_channel_request_returned (TpYtsChannel *self,
    const GError *error,
    gpointer user_data,
    GObject *weak_object)
{
  GSimpleAsyncResult *res = G_SIMPLE_ASYNC_RESULT (user_data);

  if (error != NULL)
    {
      DEBUG ("Channel.Request failed: %s", error->message);
      g_simple_async_result_set_from_error (res, error);
    }

  g_simple_async_result_complete_in_idle (res);
}

/**
 * tp_yts_channel_request_async:
 * @self: The channel proxy
 * @cancellable: Not used
 * @callback: Will be called when this operation completes
 * @user_data: Data to pass to the callback
 *
 * Start an operation to send off a Ytstenut request on this newly created
 * outgoing channel. This operation will fail on an incoming channel, or where
 * the Ytstenut request has already been sent.
 */
void
tp_yts_channel_request_async (TpYtsChannel *self,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data)
{
  GSimpleAsyncResult *res;

  g_return_if_fail (TP_IS_YTS_CHANNEL (self));

  res = g_simple_async_result_new (G_OBJECT (self), callback, user_data,
      tp_yts_channel_request_async);

  tp_yts_channel_call_request (self, -1, on_channel_request_returned, res,
      g_object_unref, G_OBJECT (self));
}

/**
 * tp_yts_channel_request_finish:
 * @self: The channel proxy
 * @result: The operation result
 * @error: If not %NULL, used to raise an error when %FALSE is returned.
 *
 * Complete an operation to send off a Ytstenut request.
 *
 * Returns: %TRUE if the operation succeeded.
 */
gboolean
tp_yts_channel_request_finish (TpYtsChannel *self,
    GAsyncResult *result,
    GError **error)
{
  GSimpleAsyncResult *res;

  g_return_val_if_fail (TP_IS_YTS_CHANNEL (self), FALSE);
  g_return_val_if_fail (G_IS_SIMPLE_ASYNC_RESULT (result), FALSE);

  res = G_SIMPLE_ASYNC_RESULT (result);
  g_return_val_if_fail (g_simple_async_result_is_valid (result,
          G_OBJECT (self), tp_yts_channel_request_async), FALSE);

  if (g_simple_async_result_propagate_error (res, error))
    return FALSE;

  return TRUE;
}

static void
on_channel_reply_returned (TpYtsChannel *self,
    const GError *error,
    gpointer user_data,
    GObject *weak_object)
{
  GSimpleAsyncResult *res = G_SIMPLE_ASYNC_RESULT (user_data);

  if (error != NULL)
    {
      DEBUG ("Channel.Reply failed: %s", error->message);
      g_simple_async_result_set_from_error (res, error);
    }

  g_simple_async_result_complete_in_idle (res);
}

/**
 * tp_yts_channel_reply_async:
 * @self: The channel proxy
 * @reply_attributes: A table of Ytstenut message attributes
 * @reply_body: A UTF-8 encoded XML Ytstenut message, or %NULL
 * @cancellable: Not used
 * @callback: Will be called when this operation completes
 * @user_data: Data to pass to the callback
 *
 * Start an operation to send a Ytstenut reply on this channel. This operation
 * will fail if this is not an incoming channel, or if the already has had
 * a reply or failure sent on it.
 */
void
tp_yts_channel_reply_async (TpYtsChannel *self,
    GHashTable *reply_attributes,
    const gchar *reply_body,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data)
{
  GSimpleAsyncResult *res;

  g_return_if_fail (TP_IS_YTS_CHANNEL (self));
  g_return_if_fail (reply_attributes);

  if (!reply_body)
    reply_body = "";

  res = g_simple_async_result_new (G_OBJECT (self), callback, user_data,
      tp_yts_channel_reply_async);

  tp_yts_channel_call_reply (self, -1, reply_attributes, reply_body,
      on_channel_reply_returned, res, g_object_unref, G_OBJECT (self));
}

/**
 * tp_yts_channel_reply_finish:
 * @self: The channel proxy
 * @result: The operation result
 * @error: If not %NULL, used to raise an error when %FALSE is returned.
 *
 * Complete an operation to send a Ytstenut reply.
 *
 * Returns: %TRUE if the operation succeeded.
 */
gboolean
tp_yts_channel_reply_finish (TpYtsChannel *self,
    GAsyncResult *result,
    GError **error)
{
  GSimpleAsyncResult *res;

  g_return_val_if_fail (TP_IS_YTS_CHANNEL (self), FALSE);
  g_return_val_if_fail (G_IS_SIMPLE_ASYNC_RESULT (result), FALSE);

  res = G_SIMPLE_ASYNC_RESULT (result);
  g_return_val_if_fail (g_simple_async_result_is_valid (result,
          G_OBJECT (self), tp_yts_channel_reply_async), FALSE);

  if (g_simple_async_result_propagate_error (res, error))
    return FALSE;

  return TRUE;
}

static void
on_channel_fail_returned (TpYtsChannel *self,
    const GError *error,
    gpointer user_data,
    GObject *weak_object)
{
  GSimpleAsyncResult *res = G_SIMPLE_ASYNC_RESULT (user_data);

  if (error != NULL)
    {
      DEBUG ("Channel.Fail failed: %s", error->message);
      g_simple_async_result_set_from_error (res, error);
    }

  g_simple_async_result_complete_in_idle (res);
}

/**
 * tp_yts_channel_fail_async:
 * @self: The channel proxy
 * @error_type: The Ytstenut error type.
 * @stanza_error_name: The name of the error.
 * @ytstenut_error_name: The Ytstenut specific error name.
 * @error_text: Error message text.
 * @cancellable: Not used
 * @callback: Will be called when this operation completes
 * @user_data: Data to pass to the callback
 *
 * Start an operation to send a Ytstenut failure on this channel. This operation
 * will fail if this is not an incoming channel, or if the already has had
 * a reply or failure sent on it.
 */
void
tp_yts_channel_fail_async (TpYtsChannel *self,
    TpYtsErrorType error_type,
    const gchar *stanza_error_name,
    const gchar *ytstenut_error_name,
    const gchar *error_text,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data)
{
  GSimpleAsyncResult *res;

  g_return_if_fail (TP_IS_YTS_CHANNEL (self));
  g_return_if_fail (stanza_error_name);
  g_return_if_fail (ytstenut_error_name);
  g_return_if_fail (error_text);

  res = g_simple_async_result_new (G_OBJECT (self), callback, user_data,
      tp_yts_channel_fail_async);

  tp_yts_channel_call_fail (self, -1, error_type, stanza_error_name,
      ytstenut_error_name, error_text, on_channel_fail_returned,
      res, g_object_unref, G_OBJECT (self));
}

/**
 * tp_yts_channel_fail_finish:
 * @self: The channel proxy
 * @result: The operation result
 * @error: If not %NULL, used to raise an error when %FALSE is returned.
 *
 * Complete an operation to send a Ytstenut failure.
 *
 * Returns: %TRUE if the operation succeeded.
 */
gboolean
tp_yts_channel_fail_finish (TpYtsChannel *self,
    GAsyncResult *result,
    GError **error)
{
  GSimpleAsyncResult *res;

  g_return_val_if_fail (TP_IS_YTS_CHANNEL (self), FALSE);
  g_return_val_if_fail (G_IS_SIMPLE_ASYNC_RESULT (result), FALSE);

  res = G_SIMPLE_ASYNC_RESULT (result);
  g_return_val_if_fail (g_simple_async_result_is_valid (result,
          G_OBJECT (self), tp_yts_channel_fail_async), FALSE);

  if (g_simple_async_result_propagate_error (res, error))
    return FALSE;

  return TRUE;
}
