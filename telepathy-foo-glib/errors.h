#ifndef TP_FOO_GLIB_ERRORS_H
#define TP_FOO_GLIB_ERRORS_H

#include <glib-object.h>

G_BEGIN_DECLS

#define TP_FOO_TYPE_ERROR (tp_foo_error_get_type ())
GType tp_foo_error_get_type (void);

#define TP_FOO_ERRORS (tp_foo_errors_quark ())
GQuark tp_foo_errors_quark (void);

typedef enum {
   TP_FOO_ERROR_PLACEHOLDER
} TpFooError;

const gchar *tp_foo_error_get_dbus_name (TpFooError error);

#define TP_FOO_ERROR_PREFIX "com.example.Foo.Error"

#define TP_FOO_ERROR_STR_PLACEHOLDER \
    TP_FOO_ERROR_PREFIX "PlaceholdersNotReplacedByRealErrors"

G_END_DECLS

#endif // TP_FOO_GLIB_ERRORS_H
