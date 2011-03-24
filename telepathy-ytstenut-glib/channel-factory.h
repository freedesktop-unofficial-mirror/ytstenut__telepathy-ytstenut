/*
 * channel-factory.h - channel factory for TpYtsChannel
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

#ifndef TP_YTS_CHANNEL_FACTORY_H
#define TP_YTS_CHANNEL_FACTORY_H

#include <telepathy-glib/client-channel-factory.h>

G_BEGIN_DECLS

typedef struct _TpYtsChannelFactory TpYtsChannelFactory;
typedef struct _TpYtsChannelFactoryClass TpYtsChannelFactoryClass;
typedef struct _TpYtsChannelFactoryPrivate TpYtsChannelFactoryPrivate;

struct _TpYtsChannelFactory {
    /*<private>*/
    GObject parent;
    TpYtsChannelFactoryPrivate *priv;
};

struct _TpYtsChannelFactoryClass {
    /*<private>*/
    GObjectClass parent_class;
    GCallback _padding[7];
};

GType tp_yts_channel_factory_get_type (void);

#define TP_TYPE_YTS_CHANNEL_FACTORY (tp_yts_channel_factory_get_type ())
#define TP_YTS_CHANNEL_FACTORY(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
    TP_TYPE_YTS_CHANNEL_FACTORY, TpYtsChannelFactory))
#define TP_YTS_CHANNEL_FACTORY_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
    TP_TYPE_YTS_CHANNEL_FACTORY, TpYtsChannelFactoryClass))
#define TP_IS_YTS_CHANNEL_FACTORY(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
    TP_TYPE_YTS_CHANNEL_FACTORY))
#define TP_IS_YTS_CHANNEL_FACTORY_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), TP_TYPE_YTS_CHANNEL_FACTORY))
#define TP_YTS_CHANNEL_FACTORY_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), TP_TYPE_YTS_CHANNEL_FACTORY, \
        TpYtsChannelFactoryClass))

TpClientChannelFactory *tp_yts_channel_factory_new (void);

G_END_DECLS

#endif
