## Process this file with automake to produce Makefile.in

abs_top_builddir = @abs_top_builddir@

# We require automake 1.6 at least.
AUTOMAKE_OPTIONS = 1.6

# The name of the module, e.g. 'glib'.
DOC_MODULE=telepathy-ytstenut-glib

# The top-level SGML file. You can change this if you want to.
DOC_MAIN_SGML_FILE=$(DOC_MODULE)-docs.sgml

# The directory containing the source code. Relative to $(srcdir).
# gtk-doc will search all .c & .h files beneath here for inline comments
# documenting the functions and macros.
# e.g. DOC_SOURCE_DIR=../../../gtk
DOC_SOURCE_DIR=../../telepathy-ytstenut-glib

# Extra options to pass to gtkdoc-scangobj. Not normally needed.
SCANGOBJ_OPTIONS=

# Extra options to supply to gtkdoc-scan.
# e.g. SCAN_OPTIONS=--deprecated-guards="GTK_DISABLE_DEPRECATED"
SCAN_OPTIONS=\
    --deprecated-guards=TP_DISABLE_DEPRECATED \
    --ignore-decorators=_TP_GNUC_DEPRECATED \
    --rebuild-types

# Extra options to supply to gtkdoc-mkdb.
# e.g. MKDB_OPTIONS=--sgml-mode --output-format=xml
MKDB_OPTIONS=--sgml-mode --output-format=xml

# If _gen is a subdirectory of $(top_srcdir)/telepathy-ytstenut-glib then we
# already get it scanned by virtue of setting DOC_SOURCE_DIR. If
# it's not a subdirectory, we'll have to add a second --source-dir
# so both get scanned (we want the declarations and doc-comments in
# the generated source to take effect).
if OUT_OF_TREE_BUILD
SCAN_OPTIONS += --source-dir=$(abs_top_builddir)/telepathy-ytstenut-glib/_gen
MKDB_OPTIONS += --source-dir=$(abs_top_builddir)/telepathy-ytstenut-glib/_gen
endif

# Extra options to supply to gtkdoc-mktmpl
# e.g. MKTMPL_OPTIONS=--only-section-tmpl
MKTMPL_OPTIONS=

# Extra options to supply to gtkdoc-fixref. Not normally needed.
# e.g. FIXXREF_OPTIONS=--extra-dir=../gdk-pixbuf/html --extra-dir=../gdk/html
FIXXREF_OPTIONS=

# Used for dependencies. The docs will be rebuilt if any of these change.
# e.g. HFILE_GLOB=$(top_srcdir)/gtk/*.h
# e.g. CFILE_GLOB=$(top_srcdir)/gtk/*.c
HFILE_GLOB=$(top_srcdir)/telepathy-ytstenut-glib/*.h $(top_builddir)/telepathy-ytstenut-glib/_gen/*.h
CFILE_GLOB=$(top_srcdir)/telepathy-ytstenut-glib/*.c $(top_builddir)/telepathy-ytstenut-glib/_gen/*.c

# Header files to ignore when scanning.
# e.g. IGNORE_HFILES=gtkdebug.h gtkintl.h
IGNORE_HFILES=

# Images to copy into HTML directory.
# e.g. HTML_IMAGES=$(top_srcdir)/gtk/stock-icons/stock_about_24.png
HTML_IMAGES=

# Extra SGML files that are included by $(DOC_MAIN_SGML_FILE).
# e.g. content_files=running.sgml building.sgml changes-2.0.sgml
content_files=tp-svc.xml

# SGML files where gtk-doc abbrevations (#GtkWidget) are expanded
# These files must be listed here *and* in content_files
# e.g. expand_content_files=running.sgml
expand_content_files=

# CFLAGS and LDFLAGS for compiling gtkdoc-scangobj with your library.
# Only needed if you are using gtkdoc-scangobj to dynamically query widget
# signals and properties.
# e.g. INCLUDES=-I$(top_srcdir) -I$(top_builddir) $(GTK_DEBUG_FLAGS)
# e.g. GTKDOC_LIBS=$(top_builddir)/gtk/$(gtktargetlib)
INCLUDES=-I$(top_srcdir) -I$(top_builddir) \
     @TP_GLIB_CFLAGS@ @DBUS_CFLAGS@ @GLIB_CFLAGS@
GTKDOC_LIBS=$(top_builddir)/telepathy-ytstenut-glib/libtelepathy-ytstenut-glib.la \
     @TP_GLIB_LIBS@ @DBUS_LIBS@ @GLIB_LIBS@

# This includes the standard gtk-doc make rules, copied by gtkdocize.
include $(top_srcdir)/gtk-doc.make

if ENABLE_GTK_DOC
check-local:
	@err=0; \
	if grep '^0 symbols incomplete' \
		$(srcdir)/telepathy-ytstenut-glib-undocumented.txt; then\
		:; \
	else \
		cat $(srcdir)/telepathy-ytstenut-glib-undocumented.txt; \
		err=1; \
	fi; \
	if grep '^0 not documented' \
		$(srcdir)/telepathy-ytstenut-glib-undocumented.txt; then\
		:; \
	else \
		cat $(srcdir)/telepathy-ytstenut-glib-undocumented.txt; \
		err=1; \
	fi; \
	if grep . $(srcdir)/telepathy-ytstenut-glib-unused.txt; then\
		echo "^^^ Unused symbols" >&2; \
		err=1; \
	fi; \
	if test -e $(srcdir)/telepathy-ytstenut-glib-undeclared.txt &&\
		grep . $(srcdir)/telepathy-ytstenut-glib-undeclared.txt; then\
		echo "^^^ Undeclared symbols" >&2; \
		err=1; \
	fi; \
	sed -n -e 's,<FILE>\(.*\)</FILE>,\1,p' $(srcdir)/telepathy-ytstenut-glib-sections.txt |\
	while read file; do \
		if grep -F "xml/$$file.xml" $(srcdir)/telepathy-ytstenut-glib-docs.sgml \
			>/dev/null;\
		then \
		:; \
		else \
			echo "$$file missing from telepathy-ytstenut-glib-docs.sgml"; \
			err=1; \
		fi; \
	done; \
	case @ENABLE_DOC_CHECKS@/$$err in \
		(yes/1) \
			echo "Documentation check failed" >&2; \
			exit $$err; \
			;; \
		(*) \
			exit 0; \
			;; \
	esac
endif
