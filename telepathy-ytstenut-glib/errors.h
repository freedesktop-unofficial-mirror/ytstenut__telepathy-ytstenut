#ifndef TP_YTS_GLIB_ERRORS_H
#define TP_YTS_GLIB_ERRORS_H

#include <glib-object.h>

G_BEGIN_DECLS

#define TP_YTS_TYPE_ERROR (tp_yts_error_get_type ())
GType tp_yts_error_get_type (void);

#define TP_YTS_ERRORS (tp_yts_errors_quark ())
GQuark tp_yts_errors_quark (void);

typedef enum {
   TP_YTS_ERROR_PLACEHOLDER
} TpYtsError;

const gchar *tp_yts_error_get_dbus_name (TpYtsError error);

#define TP_YTS_ERROR_PREFIX "com.example.Ytstenut.Error"

#define TP_YTS_ERROR_STR_PLACEHOLDER \
    TP_YTS_ERROR_PREFIX "PlaceholdersNotReplacedByRealErrors"

G_END_DECLS

#endif // TP_YTS_GLIB_ERRORS_H
