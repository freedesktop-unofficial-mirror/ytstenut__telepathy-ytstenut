/*
 * channel-fectory.c - channel factory for TpYtsChannel
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

#include "channel-factory.h"
#include "channel.h"

#include <telepathy-glib/defs.h>

#define DEBUG(msg, ...) \
    g_debug ("%s: " msg, G_STRFUNC, ##__VA_ARGS__)

/**
 * SECTION:channel-factory
 * @title: TpYtsChannelFactory
 * @short_description: channel factory for #TpYtsStatus
 *
 * A #TpClientChannelFactory for creating #TpYtsStatus objects.
 */

/**
 * TpYtsChannelFactory:
 *
 * The #TpYtsChannelFactory is used with
 * tp_channel_request_set_channel_factory() so that #TpChannelRequest
 * will create #TpYtsChannel objects.
 */

/**
 * TpYtsChannelFactoryClass:
 *
 * The class of a #TpYtsChannelFactory.
 */

static void factory_iface_init (TpClientChannelFactoryInterface *iface,
    gpointer iface_data);

G_DEFINE_TYPE_WITH_CODE (TpYtsChannelFactory, tp_yts_channel_factory,
    G_TYPE_OBJECT,
    G_IMPLEMENT_INTERFACE (TP_TYPE_CLIENT_CHANNEL_FACTORY, factory_iface_init)
);

static void
tp_yts_channel_factory_init (TpYtsChannelFactory *self)
{

}

static void
tp_yts_channel_factory_class_init (TpYtsChannelFactoryClass *klass)
{

}

static TpChannel *
tp_yts_channel_factory_obj_create_channel (TpClientChannelFactory *factory,
    TpConnection *conn,
    const gchar *path,
    GHashTable *properties,
    GError **error)
{
  return tp_yts_channel_new_from_properties (conn, path, properties, error);
}

static void factory_iface_init (TpClientChannelFactoryInterface *iface,
    gpointer iface_data)
{
#define IMPLEMENT(x) iface->x = tp_yts_channel_factory_##x;
  IMPLEMENT (obj_create_channel);
#undef IMPLEMENT
}

TpClientChannelFactory *
tp_yts_channel_factory_new (void)
{
  return g_object_new (TP_TYPE_YTS_CHANNEL_FACTORY, NULL);
}
