/*
 * client.c - client side Ytstenut implementation
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

#include "client.h"
#include "channel.h"

#include <telepathy-glib/account-channel-request.h>
#include <telepathy-glib/contact.h>
#include <telepathy-glib/interfaces.h>
#include <telepathy-glib/gtypes.h>
#include <telepathy-glib/util.h>

#include "_gen/signals-marshal.h"
#include "_gen/register-dbus-glib-marshallers-body.h"

#define DEBUG(msg, ...) \
    g_debug ("%s: " msg, G_STRFUNC, ##__VA_ARGS__)

/**
 * SECTION:client
 * @title: TpYtsClient
 * @short_description: client for managing Ytstenut channels
 *
 * A client implementation for requesting and handling Ytstenut #TpYtsChannel
 * channels and their proxies.
 *
 * To create a new client use tp_yts_client_new() and then register it with
 * tp_yts_client_register().
 *
 * You can use tp_yts_client_request_channel_async() to request new outgoing
 * channels. To receive new incoming channels, use a #TpYtsClient and connect
 * the #TpYtsClient::received-channels signal.
 */

/**
 * TpYtsClient:
 *
 * This client implementation requests and handles Ytstenut channels.
 */

/**
 * TpYtsClientClass:
 *
 * The class of a #TpYtsClient.
 */

enum {
  PROP_0,
  PROP_ACCOUNT,
  PROP_SERVICE_NAME,
};

enum {
  RECEIVED_CHANNELS,
  LAST_SIGNAL,
};

G_DEFINE_TYPE (TpYtsClient, tp_yts_client, TP_TYPE_BASE_CLIENT);

static gint signals[LAST_SIGNAL] = { 0, };

struct _TpYtsClientPrivate {
  gchar *service_name;
  TpAccount *account;
  GQueue incoming_channels;
};

static void
tp_yts_client_handle_channels (TpBaseClient *client,
    TpAccount *account,
    TpConnection *connection,
    GList *channels,
    GList *requests_satisfied,
    gint64 user_action_time,
    TpHandleChannelsContext *context)
{
  TpYtsClient *self = TP_YTS_CLIENT (client);
  GList *l;

  for (l = channels; l != NULL; l = g_list_next (l))
    {
      TpChannel *channel = l->data;

      if (!TP_IS_YTS_CHANNEL (channel))
        continue;

      g_queue_push_tail (&self->priv->incoming_channels,
          g_object_ref (channel));
    }

  tp_handle_channels_context_accept (context);
  g_signal_emit (self, signals[RECEIVED_CHANNELS], 0);
}

static void
tp_yts_client_init (TpYtsClient *self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, TP_TYPE_YTS_CLIENT,
      TpYtsClientPrivate);

  g_queue_init (&self->priv->incoming_channels);
}

static void
tp_yts_client_constructed (GObject *obj)
{
  TpYtsClient *self = TP_YTS_CLIENT (obj);
  TpBaseClient *client = TP_BASE_CLIENT (self);

  /* chain up to TpBaseClient first */
  G_OBJECT_CLASS (tp_yts_client_parent_class)->constructed (obj);

  _tp_yts_register_dbus_glib_marshallers ();

  tp_base_client_set_handler_bypass_approval (client, FALSE);

  tp_base_client_take_handler_filter (client, tp_asv_new (
      /* ChannelType */
      TP_PROP_CHANNEL_CHANNEL_TYPE, G_TYPE_STRING, TP_YTS_IFACE_CHANNEL,
      /* TargetService */
      TP_YTS_IFACE_CHANNEL ".TargetService", G_TYPE_STRING,
      self->priv->service_name,
      NULL));
}

static void
tp_yts_client_set_property (GObject *object,
    guint property_id,
    const GValue *value,
    GParamSpec *pspec)
{
  TpYtsClient *self = TP_YTS_CLIENT (object);

  switch (property_id)
    {
      case PROP_SERVICE_NAME:
        self->priv->service_name = g_value_dup_string (value);
        break;
      case PROP_ACCOUNT:
        self->priv->account = g_value_dup_object (value);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

static void
tp_yts_client_get_property (GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *pspec)
{
  TpYtsClient *self = TP_YTS_CLIENT (object);

  switch (property_id)
    {
      case PROP_SERVICE_NAME:
        g_value_set_string (value, self->priv->service_name);
        break;
      case PROP_ACCOUNT:
        g_value_set_object (value, self->priv->account);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

static void
on_channel_close_returned (GObject *source,
    GAsyncResult *result,
    gpointer user_data)
{
  TpChannel *channel = TP_CHANNEL (user_data);
  GError *error = NULL;

  tp_channel_close_finish (channel, result, &error);
  if (error != NULL)
    {
      DEBUG ("Channel.Close() failed: %s", error->message);
      g_clear_error (&error);
    }

  g_object_unref (channel);
}

static void
tp_yts_client_dispose (GObject *object)
{
  TpYtsClient *self = TP_YTS_CLIENT (object);
  TpChannel *channel;

  while (!g_queue_is_empty (&self->priv->incoming_channels))
    {
      channel = g_queue_pop_head (&self->priv->incoming_channels);
      tp_channel_close_async (channel, on_channel_close_returned, channel);
    }

  G_OBJECT_CLASS (tp_yts_client_parent_class)->dispose(object);
}


static void
tp_yts_client_finalize (GObject *object)
{
  TpYtsClient *self = TP_YTS_CLIENT (object);

  g_free (self->priv->service_name);

  G_OBJECT_CLASS (tp_yts_client_parent_class)->finalize(object);
}

static void
tp_yts_client_class_init (TpYtsClientClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  TpBaseClientClass *base_class = TP_BASE_CLIENT_CLASS (klass);

  object_class->constructed = tp_yts_client_constructed;
  object_class->dispose = tp_yts_client_dispose;
  object_class->finalize = tp_yts_client_finalize;
  object_class->get_property = tp_yts_client_get_property;
  object_class->set_property = tp_yts_client_set_property;

  base_class->handle_channels = tp_yts_client_handle_channels;

  g_type_class_add_private (klass, sizeof (TpYtsClientPrivate));

  /**
   * TpYtsClient:service-name:
   *
   * The local Ytstenut service name.
   */
  g_object_class_install_property (object_class, PROP_SERVICE_NAME,
      g_param_spec_string ("service-name", "Service Name",
          "Ytstenut Service Name", NULL,
          G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  /**
   * TpYtsClient:account:
   *
   * The local Ytstenut account to use for accepting and creating channels.
   */
  g_object_class_install_property (object_class, PROP_ACCOUNT,
      g_param_spec_object ("account", "Account", "Local Ytstenut Account",
          TP_TYPE_ACCOUNT,
          G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  /**
   * TpYtsClient::received-channels:
   *
   * This signal is emitted when one or more new incoming channels are
   * available. Use tp_yts_client_accept_channel() to access the new incoming
   * channel(s).
   */
  signals[RECEIVED_CHANNELS] =
    g_signal_new ("received-channels",
      G_TYPE_FROM_CLASS (klass),
      G_SIGNAL_RUN_LAST, 0,
      NULL, NULL,
      g_cclosure_marshal_VOID__VOID,
      G_TYPE_NONE,
      0);
}

/**
 * tp_yts_client_new:
 * @service_name: The Ytstenut service name for this client.
 * @account: The Ytstenut account for this client.
 *
 * Create a new #TpYtsClient object for the given Ytstenut name and
 * account. After getting the new client object, you should add
 * localized names, capabilities and interests with
 * tp_yts_client_add_names(), tp_yts_client_add_capabilities() and
 * tp_yts_client_add_interests() respectively.

 * After setting these values, You should call
 * tp_yts_client_register() to register the client. g_object_unref()
 * should be used to free the object.
 *
 * Returns: A newly allocated client object.
 */
TpYtsClient *
tp_yts_client_new (const gchar *service_name,
    TpAccount *account)
{
  TpYtsClient *out = NULL;
  TpDBusDaemon *bus;
  gchar *name, *tmp;

  g_return_val_if_fail (tp_dbus_check_valid_interface_name (service_name, NULL),
      NULL);

  name = g_strdup_printf ("Ytsenut.Client.%s", service_name);
  bus = tp_dbus_daemon_dup (NULL);

  out = g_object_new (TP_TYPE_YTS_CLIENT,
      "dbus-daemon", bus,
      "name", name,
      "service-name", service_name,
      "uniquify-name", TRUE,
      "account", account,
      NULL);

  g_object_unref (bus);
  g_free (name);

  /* add uid handler capability token */
  tmp = g_strdup_printf (TP_YTS_IFACE_CHANNEL "/uid/%s", service_name);
  tp_base_client_add_handler_capability (TP_BASE_CLIENT (out), tmp);
  g_free (tmp);

  return out;
}

/**
 * tp_yts_client_add_names:
 * @client: a #TpYtsClient
 * @lang: the IETF language tag of the first name
 * @...: the value of the first name and subsequent names, terminated
 *   by %NULL
 *
 * Adds a list of localized names to @client. See
 * tp_yts_client_add_name() for more details on the singular version
 * of this function.
 *
 * <example>
 *   <title>using tp_yts_client_add_names</title>
 *   <programlisting>
 * tp_yts_client_add_names (client,
 *     "en_GB", "banana",
 *     "fr", "la banane",
 *     "de", "banane",
 *     NULL);
 *   </programlisting>
 * </example>
 */
void
tp_yts_client_add_names (TpYtsClient *client,
    const gchar *lang,
    ...)
{
  const gchar *key;
  va_list ap;

  va_start (ap, lang);

  for (key = lang; key != NULL; key = va_arg (ap, const gchar *))
    {
      const gchar *val;

      val = va_arg (ap, const gchar *);

      if (val == NULL)
        break;

      tp_yts_client_add_name (client, key, val);
    }

  va_end (ap);

}

/**
 * tp_yts_client_add_name:
 * @client: a #TpYtsClient
 * @lang: the IETF language tag
 * @name: the localized name
 *
 * Adds @name as a localized name to @client. This will be advertised
 * to other Ytstenut clients when @client is registered.
 *
 * This must be called before tp_yts_client_register() is called.
 */
void
tp_yts_client_add_name (TpYtsClient *client,
    const gchar *lang,
    const gchar *name)
{
  gchar *tmp;

  tmp = g_strdup_printf (TP_YTS_IFACE_CHANNEL "/name/%s/%s",
      lang, name);

  tp_base_client_add_handler_capability (TP_BASE_CLIENT (client), tmp);

  g_free (tmp);
}

/**
 * tp_yts_client_add_capabilities:
 * @client: a #TpYtsClient
 * @cap: the first capability to add
 * @...: the subsequenet capabilities to add, followed by %NULL
 *
 * The plural version of tp_yts_client_add_capability().
 *
 * <example>
 *   <title>using tp_yts_client_add_capabilities</title>
 *   <programlisting>
 * tp_yts_client_add_capabilities (client,
 *     "urn:ytstenut:capabilities:yts-caps-video",
 *     "urn:ytstenut:capabilities:yts-caps-audio",
 *     NULL);
 *   </programlisting>
 * </example>
 */
void
tp_yts_client_add_capabilities (TpYtsClient *client,
    const gchar *first_cap,
    ...)
{
  va_list ap;
  const gchar *cap;

  va_start (ap, first_cap);

  for (cap = first_cap; cap != NULL; cap = va_arg (ap, const gchar *))
    {
      tp_yts_client_add_capability (client, cap);
    }

  va_end (ap);
}

/**
 * tp_yts_client_add_capability:
 * @client: a #TpYtsClient
 * @cap: a capability to add
 *
 * Adds @cap as capability to @client. This will be advertised to
 * other Ytstenut clients when @client is registered.
 *
 * This must be called before tp_yts_client_register() is called.
 */
void
tp_yts_client_add_capability (TpYtsClient *client,
    const gchar *cap)
{
  gchar *tmp;

  tmp = g_strdup_printf (TP_YTS_IFACE_CHANNEL "/caps/%s", cap);

  tp_base_client_add_handler_capability (TP_BASE_CLIENT (client), tmp);

  g_free (tmp);
}

/**
 * tp_yts_client_add_interests:
 * @client: a #TpYtsClient
 * @interest: the first interest to add
 * @...: the subsequent interests to add, followed by %NULL
 *
 * The plural version of tp_yts_client_add_interest().
 *
 * <example>
 *   <title>using tp_yts_client_add_interests</title>
 *   <programlisting>
 * tp_yts_client_add_interests (client,
 *     "urn:ytstenut:capabilities:falling-over",
 *     "urn:ytstenut:capabilities:taking-pictures-of-cats",
 *     NULL);
 *   </programlisting>
 * </example>
 */
void
tp_yts_client_add_interests (TpYtsClient *client,
    const gchar *first_interest,
    ...)
{
  va_list ap;
  const gchar *interest;

  va_start (ap, first_interest);

  for (interest = first_interest;
       interest != NULL;
       interest = va_arg (ap, const gchar *))
    {
      tp_yts_client_add_interest (client, interest);
    }

  va_end (ap);
}

/**
 * tp_yts_client_add_interest:
 * @client: a #TpYtsClient
 * @interest: an interest to add
 *
 * Adds @interest as an interest to @client. This will be advertised
 * to other Ytstenut clients when @client is registered and as a
 * result will mean that StatusChanged will be emitted for this
 * interest from other services.
 *
 * This must be called before tp_yts_client_register() is called.
 */
void
tp_yts_client_add_interest (TpYtsClient *client,
    const gchar *interest)
{
  gchar *tmp;

  tmp = g_strdup_printf (TP_YTS_IFACE_CHANNEL "/interested/%s", interest);

  tp_base_client_add_handler_capability (TP_BASE_CLIENT (client), tmp);

  g_free (tmp);
}

/**
 * tp_yts_client_register:
 * @self: The client object.
 * @error: If not %NULL, raise an error here when %FALSE is returned.
 *
 * Register this client with telepathy. This must be done before new channels
 * are created or received.
 *
 * Returns: %TRUE if registering was successful.
 */
gboolean
tp_yts_client_register (TpYtsClient *self,
    GError **error)
{
  return tp_base_client_register (TP_BASE_CLIENT (self), error);
}

/**
 * tp_yts_client_accept_channel:
 * @self: The client object.
 *
 * Accept a new incoming Ytstenut channel. This function never blocks, and will
 * return %NULL if no incoming channels are waiting.
 *
 * After the #TpYtsClient::received-channels signal is emitted, at least one
 * new incoming channel will be available to be returned by this function.
 *
 * Returns: The new incoming Ytstenut channel or %NULL. The ownership of the
 *      channel is transferred to the caller, who should close and release it as
 *      expected.
 */
TpYtsChannel *
tp_yts_client_accept_channel (TpYtsClient *self)
{
  g_return_val_if_fail (TP_IS_YTS_CLIENT (self), NULL);
  return g_queue_pop_head (&self->priv->incoming_channels);
}

static void
on_channel_request_create_and_handle_channel_returned (GObject *source,
    GAsyncResult *result, gpointer user_data)
{
  GSimpleAsyncResult *res = G_SIMPLE_ASYNC_RESULT (user_data);
  TpAccountChannelRequest *channel_request =
      TP_ACCOUNT_CHANNEL_REQUEST (source);
  TpChannel *channel;
  GError *error = NULL;

  channel = tp_account_channel_request_create_and_handle_channel_finish (
      channel_request, result, NULL, &error);
  if (error == NULL)
    {
      g_simple_async_result_set_op_res_gpointer (res, channel, g_object_unref);
    }
  else
    {
      g_simple_async_result_set_from_error (res, error);
      g_clear_error (&error);
    }

  g_simple_async_result_complete (res);
  g_object_unref (channel_request);
}

/**
 * tp_yts_client_request_channel_async:
 * @self: The client object
 * @target_contact: The contact to open the channel to
 * @target_service: The Ytstenut service to open the channel to
 * @request_type: The type of request to send
 * @request_attributes: A table of Ytstenut attributes, or %NULL
 * @request_body: A UTF-8 encoded XML Ytstenut message, or %NULL
 * @cancellable: Used to cancel this operation
 * @callback: Called when the operation completes
 * @user_data: Data to pass to the callback
 *
 * Start an operation to request a new Ytstenut channel.
 *
 * The new channel will have a Ytstenut request message ready to send. The
 * message is assembled from the arguments specified here.
 */
void
tp_yts_client_request_channel_async (TpYtsClient *self,
    TpContact *target_contact,
    const gchar *target_service,
    TpYtsRequestType request_type,
    GHashTable *request_attributes,
    const gchar *request_body,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data)
{
  TpAccountChannelRequest *channel_request;
  GSimpleAsyncResult *res;
  GHashTable *request_properties;
  TpHandle target_handle;

  g_return_if_fail (TP_IS_YTS_CLIENT (self));
  g_return_if_fail (TP_IS_CONTACT (target_contact));
  g_return_if_fail (tp_dbus_check_valid_interface_name (target_service, NULL));

  if (!request_body)
    request_body = "";

  g_return_if_fail (g_utf8_validate (request_body, -1, NULL));

  target_handle = tp_contact_get_handle (target_contact);
  g_return_if_fail (target_handle);

  res = g_simple_async_result_new (G_OBJECT (self), callback, user_data,
      tp_yts_client_request_channel_async);

  request_properties = tp_asv_new (
      TP_IFACE_CHANNEL ".ChannelType", G_TYPE_STRING, TP_YTS_IFACE_CHANNEL,
      TP_IFACE_CHANNEL ".TargetHandleType", G_TYPE_UINT, TP_HANDLE_TYPE_CONTACT,
      TP_IFACE_CHANNEL ".TargetHandle", G_TYPE_UINT, target_handle,
      TP_YTS_IFACE_CHANNEL ".TargetService", G_TYPE_STRING, target_service,
      TP_YTS_IFACE_CHANNEL ".InitiatorService", G_TYPE_STRING,
          self->priv->service_name,
      TP_YTS_IFACE_CHANNEL ".RequestType", G_TYPE_UINT, request_type,
      TP_YTS_IFACE_CHANNEL ".RequestBody", G_TYPE_STRING, request_body,
      NULL);

  if (request_attributes)
    tp_asv_set_boxed (request_properties,
        TP_YTS_IFACE_CHANNEL ".RequestAttributes",
        TP_HASH_TYPE_STRING_STRING_MAP, request_attributes);

  channel_request = tp_account_channel_request_new (self->priv->account,
      request_properties, 0);

  tp_account_channel_request_create_and_handle_channel_async (channel_request,
      cancellable, on_channel_request_create_and_handle_channel_returned, res);

  g_hash_table_unref (request_properties);
}

/**
 * tp_yts_client_request_channel_finish:
 * @self: A client object
 * @result: The operation result
 * @error: If not %NULL, then raise an error here when returning %NULL.
 *
 * Get the result of an operation to create a new Ytstenut channel.
 *
 * Returns: A newly created Ytstenut channel, or %NULL if failed. The caller
 *      is responsible to close and release the channel.
 */
TpYtsChannel *
tp_yts_client_request_channel_finish (TpYtsClient *self,
    GAsyncResult *result,
    GError **error)
{
  GSimpleAsyncResult *res;

  g_return_val_if_fail (TP_IS_YTS_CLIENT (self), NULL);
  g_return_val_if_fail (g_simple_async_result_is_valid (result, G_OBJECT (self),
      tp_yts_client_request_channel_async), NULL);

  res = G_SIMPLE_ASYNC_RESULT (result);
  if (g_simple_async_result_propagate_error (res, error))
    return NULL;

  return g_object_ref (g_simple_async_result_get_op_res_gpointer (res));
}
