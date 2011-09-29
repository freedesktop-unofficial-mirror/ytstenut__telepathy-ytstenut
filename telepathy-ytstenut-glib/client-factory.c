/*
 * client-factory.c - client factory for TpYtsChannel
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

#include "client-factory.h"
#include "channel.h"

#include <telepathy-glib/defs.h>
#include <telepathy-glib/interfaces.h>
#include <telepathy-glib/util.h>

#define DEBUG(msg, ...) \
    g_debug ("%s: " msg, G_STRFUNC, ##__VA_ARGS__)

/**
 * SECTION:client-factory
 * @title: TpYtsClientFactory
 * @short_description: client factory for #TpYtsChannel
 *
 * A #TpSimpleClientFactory for creating #TpYtsChannel objects. In general
 * you won't need to use this object directly because #TpYtsAccountManager creates
 * and uses it as necessary.
 */

/**
 * TpYtsClientFactory:
 *
 * A factory for creating #TpYtsChannel objects.
 */

/**
 * TpYtsClientFactoryClass:
 *
 * The class of a #TpYtsClientFactory.
 */

G_DEFINE_TYPE (TpYtsClientFactory, tp_yts_client_factory,
    TP_TYPE_AUTOMATIC_CLIENT_FACTORY)

#define chainup ((TpSimpleClientFactoryClass *) \
    tp_yts_client_factory_parent_class)

static void
tp_yts_client_factory_init (TpYtsClientFactory *self)
{

}

static TpChannel *
tp_yts_client_factory_obj_create_channel (TpSimpleClientFactory *self,
    TpConnection *conn,
    const gchar *object_path,
    const GHashTable *properties,
    GError **error)
{
  const gchar *chan_type;

  chan_type = tp_asv_get_string (properties, TP_PROP_CHANNEL_CHANNEL_TYPE);

  if (!tp_strdiff (chan_type, TP_YTS_IFACE_CHANNEL))
    {
      return tp_yts_channel_new_with_factory (self, conn, object_path,
          (GHashTable *) properties, error);
    }

  /* Chainup on parent implementation as fallback */
  return chainup->create_channel (self, conn, object_path, properties, error);
}

static void
tp_yts_client_factory_class_init (TpYtsClientFactoryClass *klass)
{
  TpSimpleClientFactoryClass *simple_class = (TpSimpleClientFactoryClass *) klass;

  simple_class->create_channel = tp_yts_client_factory_obj_create_channel;
}

/**
 * tp_yts_client_factory_new:
 *
 * Create a new #TpYtsClientFactory object. In general you won't need to
 * call this function because #TpYtsClient creates #TpYtsClientFactory
 * objects as necessary.
 *
 * Returns: A newly allocated factory object.
 */
TpSimpleClientFactory *
tp_yts_client_factory_new (TpDBusDaemon *dbus)
{
  g_return_val_if_fail (TP_IS_DBUS_DAEMON (dbus), NULL);

  return g_object_new (TP_TYPE_YTS_CLIENT_FACTORY,
      "dbus-daemon", dbus,
      NULL);
}
