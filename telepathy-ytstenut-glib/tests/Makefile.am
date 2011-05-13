
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
	server-passing-status \
	server-file-transfer

Android.mk: Makefile.am
	androgenizer -:PROJECT telepathy-ytstenut-glib \
	 -:REL_TOP $(top_srcdir) -:ABS_TOP $(abs_top_srcdir) \
	 -:EXECUTABLE nosey-status -:TAGS eng debug \
	 -:SOURCES nosey-status.c \
	 -:CFLAGS $(DEFS) $(CFLAGS) $(DEFAULT_INCLUDES) $(INCLUDES) \
	  $(AM_CFLAGS) \
	 -:CPPFLAGS $(CPPFLAGS) $(AM_CPPFLAGS) \
	 -:LDFLAGS $(AM_LDFLAGS) $(LDADD) libfuture-extensions.la \
	 -:EXECUTABLE passing-service -:TAGS eng debug \
	 -:SOURCES passing-service.c \
	 -:CFLAGS $(DEFS) $(CFLAGS) $(DEFAULT_INCLUDES) $(INCLUDES) \
	  $(AM_CFLAGS) \
	 -:CPPFLAGS $(CPPFLAGS) $(AM_CPPFLAGS) \
	 -:LDFLAGS $(AM_LDFLAGS) $(LDADD) libfuture-extensions.la \
	 -:EXECUTABLE passing-status -:TAGS eng debug \
	 -:SOURCES passing-status.c \
	 -:CFLAGS $(DEFS) $(CFLAGS) $(DEFAULT_INCLUDES) $(INCLUDES) \
	  $(AM_CFLAGS) \
	 -:CPPFLAGS $(CPPFLAGS) $(AM_CPPFLAGS) \
	 -:LDFLAGS $(AM_LDFLAGS) $(LDADD) libfuture-extensions.la \
	> $@
