SUBDIRS = . tests

pkgconfigdir = ${libdir}/pkgconfig
pkgconfig_DATA = telepathy-ytstenut-glib.pc

tpytstenutglibincludedir=$(includedir)/telepathy-ytstenut/telepathy-ytstenut-glib
genincludedir=$(tpytstenutglibincludedir)/_gen
tools_dir = $(top_srcdir)/tp-glib-tools

lib_LTLIBRARIES = libtelepathy-ytstenut-glib.la

libtelepathy_ytstenut_glib_la_SOURCES = \
    account-manager.c \
    account-manager.h \
    errors.c \
    errors.h \
    telepathy-ytstenut-glib.c \
    telepathy-ytstenut-glib.h

nodist_libtelepathy_ytstenut_glib_la_SOURCES = \
    _gen/signals-marshal.c \
    _gen/signals-marshal.h \
    _gen/signals-marshal.list \
    _gen/enums.h \
    _gen/gtypes.h \
    _gen/gtypes-body.h \
    _gen/interfaces.h \
    _gen/interfaces-body.h \
    _gen/cli-account-manager.h \
    _gen/cli-account-manager-body.h \
    _gen/svc.h \
    _gen/svc.c

libtelepathy_ytstenut_glib_la_LDFLAGS = \
    -version-info "$(LT_CURRENT)":"$(LT_REVISION)":"$(LT_AGE)"

BUILT_SOURCES = \
    _gen/all.xml \
    $(nodist_libtelepathy_ytstenut_glib_la_SOURCES)

tpytstenutglibinclude_HEADERS = \
    account-manager.h \
    errors.h \
    telepathy-ytstenut-glib.h

nodist_geninclude_HEADERS = \
    _gen/enums.h \
    _gen/gtypes.h \
    _gen/interfaces.h \
    _gen/cli-account-manager.h \
    _gen/svc.h

EXTRA_DIST = \
    all.xml

CLEANFILES = \
    $(BUILT_SOURCES)

AM_CFLAGS = $(ERROR_CFLAGS) @DBUS_CFLAGS@ @GLIB_CFLAGS@ @TP_GLIB_CFLAGS@ \
	-I$(top_srcdir)
AM_LDFLAGS = @DBUS_LIBS@ @GLIB_LIBS@ @TP_GLIB_LIBS@

# Generated stuff

_gen/all.xml: all.xml $(wildcard $(top_srcdir)/spec/*.xml)
	$(mkdir_p) _gen
	$(AM_V_GEN)$(PYTHON) $(tools_dir)/xincludator.py \
		$< > $@.tmp && mv $@.tmp $@

_gen/cli-account-manager-body.h _gen/cli-account-manager.h: _gen/all.xml \
	$(tools_dir)/glib-client-gen.py
	$(AM_V_GEN)$(PYTHON) $(tools_dir)/glib-client-gen.py \
		--group=account_manager \
		--subclass=TpYtsAccountManager \
		--subclass-assert=TP_IS_YTS_ACCOUNT_MANAGER \
		--iface-quark-prefix=TP_YTS_IFACE_QUARK \
		--tp-proxy-api=0.10.0 \
		$< Tp_Yts _gen/cli-account-manager

_gen/svc.c _gen/svc.h: _gen/all.xml $(tools_dir)/glib-ginterface-gen.py \
	Makefile.am
	$(AM_V_GEN)$(PYTHON) $(tools_dir)/glib-ginterface-gen.py \
		--filename=_gen/svc --signal-marshal-prefix=_tp_yts \
		--include='<telepathy-glib/dbus.h>' \
		--include='"_gen/signals-marshal.h"' \
		--allow-unstable \
		--not-implemented-func='tp_dbus_g_method_return_not_implemented' \
		$< Tp_Yts_Svc_

_gen/signals-marshal.list: _gen/all.xml \
	$(tools_dir)/glib-signals-marshal-gen.py \
	Makefile.am
	$(AM_V_GEN)$(PYTHON) $(tools_dir)/glib-signals-marshal-gen.py $< > $@

_gen/signals-marshal.h: _gen/signals-marshal.list Makefile.am
	$(AM_V_GEN)$(GLIB_GENMARSHAL) --header --prefix=_tp_yts_marshal $< > $@

_gen/signals-marshal.c: _gen/signals-marshal.list Makefile.am
	$(AM_V_GEN){ echo '#include "_gen/signals-marshal.h"' && \
	$(GLIB_GENMARSHAL) --body --prefix=_tp_yts_marshal $< ; } > $@

_gen/enums.h: _gen/all.xml $(tools_dir)/c-constants-gen.py \
	Makefile.am
	$(AM_V_GEN)$(PYTHON) $(tools_dir)/c-constants-gen.py Tp_Yts $< > $@

_gen/interfaces.h _gen/interfaces-body.h: _gen/all.xml \
	$(tools_dir)/glib-interfaces-gen.py \
	Makefile.am
	$(AM_V_GEN)$(PYTHON) $(tools_dir)/glib-interfaces-gen.py \
		Tp_Yts _gen/interfaces-body.h _gen/interfaces.h $<

_gen/gtypes.h _gen/gtypes-body.h: _gen/all.xml \
	$(tools_dir)/glib-gtypes-generator.py Makefile.am
	$(AM_V_GEN)$(PYTHON) $(tools_dir)/glib-gtypes-generator.py \
		$< _gen/gtypes Tp_Yts

