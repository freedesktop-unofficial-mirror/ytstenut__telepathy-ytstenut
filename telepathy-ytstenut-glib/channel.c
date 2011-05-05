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

/* properties */
enum {
  PROP_0,
  PROP_REQUEST_TYPE,
  PROP_REQUEST_ATTRIBUTES,
  PROP_REQUEST_BODY,
  PROP_TARGET_SERVICE,
  PROP_INITIATOR_SERVICE,
};

struct _TpYtsChannelPrivate
{
  TpYtsRequestType request_type;
  GHashTable *request_attributes;
  gchar *request_body;
  gchar *target_service;
  gchar *initiator_service;
};

G_DEFINE_TYPE (TpYtsChannel, tp_yts_channel, TP_TYPE_CHANNEL);

static void
tp_yts_channel_init (TpYtsChannel *self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, TP_TYPE_YTS_CHANNEL,
      TpYtsChannelPrivate);
}

static void
tp_yts_channel_constructed (GObject *object)
{
  TpYtsChannel *self = TP_YTS_CHANNEL (object);
  TpYtsChannelPrivate *priv = self->priv;
  GHashTable *props;

  g_object_get (object,
      "channel-properties", &props,
      NULL);

  /* RequestType */
  priv->request_type = tp_asv_get_uint32 (props,
      TP_YTS_IFACE_CHANNEL ".RequestType", NULL);

  /* RequestAttributes */
  priv->request_attributes = tp_asv_get_boxed (props,
          TP_YTS_IFACE_CHANNEL ".RequestAttributes",
          TP_HASH_TYPE_STRING_STRING_MAP);

  /* only ref it if we get one out */
  if (priv->request_attributes != NULL)
    g_hash_table_ref (priv->request_attributes);

  /* RequestBody */
  priv->request_body = g_strdup (tp_asv_get_string (props,
          TP_YTS_IFACE_CHANNEL ".RequestBody"));

  /* TargetService */
  priv->target_service = g_strdup (tp_asv_get_string (props,
          TP_YTS_IFACE_CHANNEL ".TargetService"));

  /* InitiatorService */
  priv->initiator_service = g_strdup (tp_asv_get_string (props,
          TP_YTS_IFACE_CHANNEL ".InitiatorService"));

  g_hash_table_unref (props);

  if (G_OBJECT_CLASS (tp_yts_channel_parent_class)->constructed != NULL)
    G_OBJECT_CLASS (tp_yts_channel_parent_class)->constructed (object);
}

static void
tp_yts_channel_finalize (GObject *object)
{
  TpYtsChannel *self = TP_YTS_CHANNEL (object);
  TpYtsChannelPrivate *priv = self->priv;

  tp_clear_pointer (&priv->request_attributes, g_hash_table_unref);
  tp_clear_pointer (&priv->request_body, g_free);
  tp_clear_pointer (&priv->target_service, g_free);
  tp_clear_pointer (&priv->initiator_service, g_free);

  if (G_OBJECT_CLASS (tp_yts_channel_parent_class)->finalize != NULL)
    G_OBJECT_CLASS (tp_yts_channel_parent_class)->finalize (object);
}

static void
tp_yts_channel_get_property (GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *pspec)
{
  TpYtsChannel *self = TP_YTS_CHANNEL (object);

  switch (property_id)
    {
      case PROP_REQUEST_TYPE:
        g_value_set_uint (value, self->priv->request_type);
        break;
      case PROP_REQUEST_ATTRIBUTES:
        g_value_set_boxed (value, self->priv->request_attributes);
        break;
      case PROP_REQUEST_BODY:
        g_value_set_string (value, self->priv->request_body);
        break;
      case PROP_TARGET_SERVICE:
        g_value_set_string (value, self->priv->target_service);
        break;
      case PROP_INITIATOR_SERVICE:
        g_value_set_string (value, self->priv->initiator_service);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

static void
tp_yts_channel_class_init (TpYtsChannelClass *klass)
{
  GObjectClass *object_class = (GObjectClass *) klass;
  TpProxyClass *proxy_class = (TpProxyClass *) klass;
  GType tp_type = TP_TYPE_YTS_CHANNEL;

  object_class->constructed = tp_yts_channel_constructed;
  object_class->finalize = tp_yts_channel_finalize;
  object_class->get_property = tp_yts_channel_get_property;

  proxy_class->interface = TP_YTS_IFACE_QUARK_CHANNEL;

  g_type_class_add_private (klass, sizeof (TpYtsChannelPrivate));

  tp_proxy_init_known_interfaces ();
  tp_proxy_or_subclass_hook_on_interface_add (tp_type,
      tp_yts_channel_add_signals);
  tp_proxy_subclass_add_error_mapping (tp_type,
      TP_ERROR_PREFIX, TP_ERRORS, TP_TYPE_ERROR);

  /**
   * TpYtsChannel:request-type:
   *
   * The IQ type of the request message.
   */
  g_object_class_install_property (object_class, PROP_REQUEST_TYPE,
      g_param_spec_uint ("request-type", "Request type",
          "Ytstenut Request Type", 0, NUM_TP_YTS_REQUEST_TYPES, 0,
          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  /**
   * TpYtsChannel:request-attributes:
   *
   * The attributes present on the Ytstenut request message.
   */
  g_object_class_install_property (object_class, PROP_REQUEST_ATTRIBUTES,
      g_param_spec_boxed ("request-attributes", "Request attributes",
          "Ytstenut Request Attributes",
          TP_HASH_TYPE_STRING_STRING_MAP,
          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  /**
   * TpYtsChannel:request-body:
   *
   * The UTF-8 encoded XML body of the Ytstenut request message.
   */
  g_object_class_install_property (object_class, PROP_REQUEST_BODY,
      g_param_spec_string ("request-body", "Request body",
          "Ytstenut Request Body", NULL,
          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  /**
   * TpYtsChannel:target-service:
   *
   * The name of the target Ytstenut service that this channel is
   * targetted at.
   */
  g_object_class_install_property (object_class, PROP_TARGET_SERVICE,
      g_param_spec_string ("target-service", "Target service",
          "Ytstenut Target Service", NULL,
          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  /**
   * TpYtsChannel:initiator-service:
   *
   * The name of the Ytstenut service which initiated this channel.
   */
  g_object_class_install_property (object_class, PROP_INITIATOR_SERVICE,
      g_param_spec_string ("initiator-service", "Initiator service",
          "Ytstenut Initiator Service", NULL,
          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

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

  ret = TP_CHANNEL (g_object_new (TP_TYPE_YTS_CHANNEL,
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
  g_return_if_fail (reply_attributes != NULL);

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
  g_return_if_fail (stanza_error_name != NULL);
  g_return_if_fail (ytstenut_error_name != NULL);
  g_return_if_fail (error_text != NULL);

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

/**
 * tp_yts_channel_get_request_type:
 * @self: a #TpYtsChannel
 *
 * <!-- -->
 *
 * Returns: the same as the #TpYtsChannel:request-type property
 */
TpYtsRequestType
tp_yts_channel_get_request_type (TpYtsChannel *self)
{
  TpYtsChannelPrivate *priv;

  g_return_val_if_fail (TP_IS_YTS_CHANNEL (self), 0);

  priv = self->priv;

  return priv->request_type;
}

/**
 * tp_yts_channel_get_request_attributes:
 * @self: a #TpYtsChannel
 *
 * <!-- -->
 *
 * Returns: the same as the #TpYtsChannel:request-attributes property,
 *   use g_hash_table_ref() to keep a reference to the returned hash
 *   table
 */
GHashTable *
tp_yts_channel_get_request_attributes (TpYtsChannel *self)
{
  TpYtsChannelPrivate *priv;

  g_return_val_if_fail (TP_IS_YTS_CHANNEL (self), NULL);

  priv = self->priv;

  return priv->request_attributes;
}

/**
 * tp_yts_channel_get_request_body:
 * @self: a #TpYtsChannel
 *
 * <!-- -->
 *
 * Returns: the same as the #TpYtsChannel:request-body property
 */
const gchar *
tp_yts_channel_get_request_body (TpYtsChannel *self)
{
  TpYtsChannelPrivate *priv;

  g_return_val_if_fail (TP_IS_YTS_CHANNEL (self), NULL);

  priv = self->priv;

  return priv->request_body;
}

/**
 * tp_yts_channel_get_target_service:
 * @self: a #TpYtsChannel
 *
 * <!-- -->
 *
 * Returns: the same as the #TpYtsChannel:target-service property
 */
const gchar *
tp_yts_channel_get_target_service (TpYtsChannel *self)
{
  TpYtsChannelPrivate *priv;

  g_return_val_if_fail (TP_IS_YTS_CHANNEL (self), NULL);

  priv = self->priv;

  return priv->target_service;
}

/**
 * tp_yts_channel_get_initiator-service:
 * @self: a #TpYtsChannel
 *
 * <!-- -->
 *
 * Returns: the same as the #TpYtsChannel:initiator-service property
 */
const gchar *
tp_yts_channel_get_initiator_service (TpYtsChannel *self)
{
  TpYtsChannelPrivate *priv;

  g_return_val_if_fail (TP_IS_YTS_CHANNEL (self), 0);

  priv = self->priv;

  return priv->initiator_service;
}
