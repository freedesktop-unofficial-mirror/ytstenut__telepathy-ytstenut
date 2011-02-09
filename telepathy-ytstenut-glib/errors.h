#ifndef TP_YTSTENUT_GLIB_ERRORS_H
#define TP_YTSTENUT_GLIB_ERRORS_H

#include <glib-object.h>

G_BEGIN_DECLS

#define TP_YTSTENUT_TYPE_ERROR (tp_ytstenut_error_get_type ())
GType tp_ytstenut_error_get_type (void);

#define TP_YTSTENUT_ERRORS (tp_ytstenut_errors_quark ())
GQuark tp_ytstenut_errors_quark (void);

typedef enum {
   TP_YTSTENUT_ERROR_PLACEHOLDER
} TpYtstenutError;

const gchar *tp_ytstenut_error_get_dbus_name (TpYtstenutError error);

#define TP_YTSTENUT_ERROR_PREFIX "com.example.Ytstenut.Error"

#define TP_YTSTENUT_ERROR_STR_PLACEHOLDER \
    TP_YTSTENUT_ERROR_PREFIX "PlaceholdersNotReplacedByRealErrors"

G_END_DECLS

#endif // TP_YTSTENUT_GLIB_ERRORS_H
