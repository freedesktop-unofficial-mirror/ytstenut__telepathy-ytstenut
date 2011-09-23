
AM_CFLAGS = \
	$(ERROR_CFLAGS) \
	$(DBUS_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(TP_GLIB_CFLAGS) \
	-I$(top_srcdir)

AM_LDFLAGS = \
	$(DBUS_LIBS) \
	$(GLIB_LIBS) \
	$(TP_GLIB_LIBS)

LDADD = \
	$(top_builddir)/telepathy-ytstenut-glib/libtelepathy-ytstenut-glib.la

noinst_PROGRAMS = \
	diddle-account-manager \
	client-pong \
	client-ping \
	nosey-status \
	passing-service \
	passing-status \
	server-client-pong \
	server-client-ping \
	server-nosey-status \
	server-passing-service \
	server-passing-status
