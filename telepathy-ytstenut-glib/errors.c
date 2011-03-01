#include <telepathy-ytstenut-glib/errors.h>

#include <dbus/dbus-glib.h>

GQuark
tp_ytstenut_errors_quark (void)
{
  static gsize quark = 0;

  if (g_once_init_enter (&quark))
    {
      GQuark domain = g_quark_from_static_string ("tp_ytstenut_errors");

      dbus_g_error_domain_register (domain, TP_YTSTENUT_ERROR_PREFIX,
          TP_YTSTENUT_TYPE_ERROR);
      g_once_init_leave (&quark, domain);
    }

  return (GQuark) quark;
}

GType
tp_ytstenut_error_get_type (void)
{
  static GType etype = 0;

  if (g_once_init_enter (&etype))
    {
      static const GEnumValue values[] = {
        { TP_YTSTENUT_ERROR_PLACEHOLDER, "TP_YTSTENUT_ERROR_PLACEHOLDER", "Placeholder" },
        { 0 }
      };

      g_once_init_leave (&etype, g_enum_register_static ("TpYtstenutError", values));
    }

  return etype;
}

const gchar *
tp_ytstenut_error_get_dbus_name (TpYtstenutError error)
{
  switch (error)
    {
    case TP_YTSTENUT_ERROR_PLACEHOLDER:
      return TP_YTSTENUT_ERROR_STR_PLACEHOLDER;
    default:
      g_return_val_if_reached (NULL);
    }
}
