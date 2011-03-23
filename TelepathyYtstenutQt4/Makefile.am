include $(top_srcdir)/tp-qt4-tools/telepathy-qt4.mk

tpytstenutqt4includedir=$(includedir)/telepathy-ytstenut/TelepathyYtstenutQt4
genincludedir=$(tpytstenutqt4includedir)/_gen

pkgconfigdir = ${libdir}/pkgconfig
pkgconfig_DATA = TelepathyYtstenutQt4.pc

tools_dir = $(top_srcdir)/tp-qt4-tools

AM_CXXFLAGS = \
    $(ERROR_CXXFLAGS)

AM_CPPFLAGS = \
    -I$(top_srcdir) -I$(top_builddir) \
    $(QTCORE_CFLAGS) \
    $(QTDBUS_CFLAGS) \
    $(TP_QT4_CFLAGS)

lib_LTLIBRARIES = libtelepathy-ytstenut-qt4.la

libtelepathy_ytstenut_qt4_la_LIBADD = \
    $(QTCORE_LIBS) \
    $(QTDBUS_LIBS) \
    $(TP_QT4_LIBS)

libtelepathy_ytstenut_qt4_la_SOURCES = \
    cli-connection.cpp \
    cli-connection.h \
    types.cpp \
    types.h

libtelepathy_ytstenut_qt4_la_LDFLAGS = \
    -version-info "$(LT_CURRENT)":"$(LT_REVISION)":"$(LT_AGE)" -static

nodist_libtelepathy_ytstenut_qt4_la_SOURCES = \
    _gen/cli-connection-body.hpp \
    _gen/cli-connection.moc.hpp \
    _gen/types-body.hpp

tpytstenutqt4include_HEADERS = \
    cli-connection.h \
    constants.h \
    types.h

nodist_geninclude_HEADERS = \
    _gen/cli-connection.h \
    _gen/constants.h \
    _gen/types.h

EXTRA_DIST = \
    connection.xml

BUILT_SOURCES = \
    _gen/all.xml \
    _gen/connection.xml \
    $(nodist_libtelepathy_ytstenut_qt4_la_SOURCES) \
    $(nodist_geninclude_HEADERS)

CLEANFILES = \
    $(BUILT_SOURCES)

distclean-local:
	rm -rf _gen

# Generated files which can be generated for all categories simultaneously

_gen/all.xml: $(top_srcdir)/spec/all.xml $(wildcard $(top_srcdir)/spec/*.xml)
	$(mkdir_p) _gen
	$(AM_V_GEN)$(PYTHON) $(tools_dir)/xincludator.py \
		$< > $@.tmp && mv $@.tmp $@

_gen/constants.h: _gen/all.xml \
		$(tools_dir)/qt4-constants-gen.py
	$(AM_V_GEN)$(PYTHON) $(tools_dir)/qt4-constants-gen.py \
		--namespace='TpYts' \
		--str-constant-prefix='TP_YTS_' \
		--specxml=$< \
		> $@.tmp && mv $@.tmp $@

_gen/types.h _gen/types-body.hpp: _gen/all.xml \
		$(tools_dir)/qt4-types-gen.py
	$(AM_V_GEN)$(PYTHON) $(tools_dir)/qt4-types-gen.py \
		--namespace='TpYts' \
		--declfile='_gen/types.h' \
		--implfile='_gen/types-body.hpp' \
		--realinclude='types.h' \
		--specxml=$< \
		--extraincludes='<TelepathyQt4/Types>'

# Things generated per interface group

_gen/connection.xml: connection.xml $(wildcard $(top_srcdir)/spec/*.xml)
	$(mkdir_p) _gen
	$(AM_V_GEN)$(PYTHON) $(tools_dir)/xincludator.py \
		$< > $@.tmp && mv $@.tmp $@

_gen/cli-connection-body.hpp _gen/cli-connection.h: _gen/connection.xml \
	_gen/all.xml $(tools_dir)/qt4-client-gen.py
	$(AM_V_GEN)$(PYTHON) $(tools_dir)/qt4-client-gen.py \
		--namespace='TpYts::Client' \
		--typesnamespace='TpYts' \
		--headerfile=_gen/cli-connection.h \
		--implfile=_gen/cli-connection-body.hpp \
		--realinclude=$(top_srcdir)/TelepathyYtstenutQt4/cli-connection.h \
		--specxml=_gen/all.xml \
		--ifacexml=$< \
		--extraincludes='<TelepathyQt4/Connection>,<TelepathyYtstenutQt4/types.h>' \
		--mainiface='Tp::Client::ConnectionInterface'

%.moc.hpp: %.h _gen/constants.h _gen/types.h
	$(TPQT4_V_MOC)$(MOC) $(AM_CPPFLAGS) -i $< -o $@
