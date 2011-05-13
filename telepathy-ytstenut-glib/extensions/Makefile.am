# This directory re-uses telepathy-glib's code generation mechanisms to
# generate code for interfaces that aren't stable enough for telepathy-glib

tools_dir = $(top_srcdir)/tp-glib-tools

AM_CFLAGS = \
    $(ERROR_CFLAGS) \
    $(DBUS_CFLAGS) \
    $(GLIB_CFLAGS) \
    $(TP_GLIB_CFLAGS)
AM_LDFLAGS = \
    $(ERROR_LDFLAGS) \
    $(NULL)

EXTRA_DIST = \
    all.xml \
    Connection_Future.xml

noinst_LTLIBRARIES = libfuture-extensions.la

# In an external project you'd use $(TP_GLIB_LIBS) (obtained from
# pkg-config via autoconf) instead of the .la path
libfuture_extensions_la_LIBADD = \
    $(GLIB_LIBS) \
    $(DBUS_LIBS) \
    $(TP_GLIB_LIBS)

libfuture_extensions_la_SOURCES = \
    connection-future.c \
    connection-future.h \
    extensions.c \
    extensions.h

nodist_libfuture_extensions_la_SOURCES = \
    _gen/interfaces.h \
    _gen/interfaces-body.h \
    _gen/cli-connection-future.h \
    _gen/cli-connection-future-body.h

BUILT_SOURCES = \
    _gen/all.xml \
    $(nodist_libfuture_extensions_la_SOURCES)

CLEANFILES = $(BUILT_SOURCES)

clean-local:
	rm -rf _gen

XSLTPROCFLAGS = --nonet --novalid

# Generated files which can be generated for all categories simultaneously

_gen/all.xml: all.xml $(wildcard $(srcdir)/*.xml) $(wildcard $(top_srcdir)/spec/*.xml) $(tools_dir)/xincludator.py
	$(mkdir_p) _gen
	$(AM_V_GEN)$(PYTHON) $(tools_dir)/xincludator.py $< > $@

_gen/interfaces-body.h _gen/interfaces.h: _gen/all.xml \
	$(tools_dir)/glib-interfaces-gen.py
	$(AM_V_GEN)$(PYTHON) $(tools_dir)/glib-interfaces-gen.py \
		_Tp_Yts _gen/interfaces-body.h _gen/interfaces.h $<

_gen/cli-connection-future-body.h _gen/cli-connection-future.h: _gen/all.xml \
	$(tools_dir)/glib-client-gen.py Makefile.am
	$(AM_V_GEN)$(PYTHON) $(tools_dir)/glib-client-gen.py \
		--group=connection_future \
		--subclass=TpConnection \
		--subclass-assert=TP_IS_CONNECTION \
		--iface-quark-prefix=_TP_YTS_IFACE_QUARK \
		--tp-proxy-api=0.7.6 \
		$< _Tp_Yts_Cli _gen/cli-connection-future

Android.mk: Makefile.am $(BUILT_SOURCES)
	androgenizer -:PROJECT telepathy-ytstenut \
	 -:SHARED future-extensions -:TAGS eng debug \
	 -:REL_TOP $(top_srcdir) -:ABS_TOP $(abs_top_srcdir) \
	 -:SOURCES $(libfuture_extensions_la_SOURCES) \
	  $(nodist_libfuture_extensions_la_SOURCES) \
	 -:CFLAGS $(DEFS) $(CFLAGS) $(DEFAULT_INCLUDES) $(INCLUDES) \
	  $(AM_CFLAGS) \
	 -:CPPFLAGS $(CPPFLAGS) $(AM_CPPFLAGS) \
	 -:LDFLAGS $(AM_LDFLAGS) $(libfuture_extensions_la_LIBADD) \
	> $@
