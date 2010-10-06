#include <telepathy-foo-glib/errors.h>

#include <dbus/dbus-glib.h>

GQuark
tp_foo_errors_quark (void)
{
  static gsize quark = 0;

  if (g_once_init_enter (&quark))
    {
      GQuark domain = g_quark_from_static_string ("tp_foo_errors");

      dbus_g_error_domain_register (domain, TP_FOO_ERROR_PREFIX,
          TP_FOO_TYPE_ERROR);
      g_once_init_leave (&quark, domain);
    }

  return (GQuark) quark;
}

GType
tp_foo_error_get_type (void)
{
  static GType etype = 0;

  if (g_once_init_enter (&etype))
    {
      static const GEnumValue values[] = {
        { TP_FOO_ERROR_PLACEHOLDER, "TP_FOO_ERROR_PLACEHOLDER", "PlaceholdersNotReplacedByRealErrors" },
        { 0 }
      };

      g_once_init_leave (&etype, g_enum_register_static ("TpFooError", values));
    }

  return etype;
}

const gchar *
tp_foo_error_get_dbus_name (TpFooError error)
{
  switch (error)
    {
    case TP_FOO_ERROR_PLACEHOLDER:
      return TP_FOO_ERROR_STR_PLACEHOLDER;
    default:
      g_return_val_if_reached (NULL);
    }
}
