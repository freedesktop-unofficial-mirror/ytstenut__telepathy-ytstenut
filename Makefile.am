ACLOCAL_AMFLAGS = -I m4

SUBDIRS = \
	tp-qt4-tools \
	tp-glib-tools \
	docs

if ENABLE_QT4
SUBDIRS += TelepathyYtstenutQt4
endif

if ENABLE_GLIB
SUBDIRS += telepathy-ytstenut-glib
endif

GIT = git
GZIP = gzip
TAR = tar

XMLS = $(wildcard $(srcdir)/spec/*.xml)
TEMPLATES = $(wildcard $(srcdir)/docs/templates/*)
TOOLS = \
	tools/doc-generator.py \
	tools/specparser.py \
	tools/xincludator.py

GENERATED_FILES = \
	docs/spec/index.html \
	docs/telepathy-ytstenut-spec.tar.bz2

docs/spec/index.html: $(XMLS) $(TOOLS) $(TEMPLATES)
	@install -d doc
	$(AM_V_GEN)$(PYTHON) $(top_srcdir)/tools/doc-generator.py $(top_srcdir)/spec/all.xml $(top_builddir)/docs/spec/ $(PACKAGE) com.example.Ytstenut
	@echo '  NOTE  ' file://$$PWD/$@

docs/telepathy-ytstenut-spec.tar.bz2: docs/spec/index.html
	$(AM_V_GEN)tar cjvf $@ --transform s,docs/spec,telepathy-ytstenut-spec, docs/spec

if SPEC_DOCUMENTATION
all: docs/spec/index.html
else
all:
endif

doc-tarball: docs/telepathy-ytstenut-spec.tar.bz2

EXTRA_DIST = $(XMLS) $(TEMPLATES) tools

CLEANFILES = $(GENERATED_FILES) \
	$(wildcard $(srcdir)/tools/*.pyc)

clean-local:
	-rm -rf docs/spec tmp

BRANCH = $(shell sh $(top_srcdir)/tools/git-which-branch.sh misc | tr -d '\n' | tr -C "[:alnum:]" _)
UPLOAD_BRANCH_TO = dhansak.collabora.co.uk:public_html/telepathy-ytstenut

# Usage: make upload-branch BRANCH=discussion
upload-branch: all
	rsync -rzvP $(patsubst %.txt,%.html,$(RST)) docs/spec \
		$(UPLOAD_BRANCH_TO)-$(BRANCH)/
	@echo Your spec branch might be at:
	@echo '  ' http://people.collabora.co.uk/~$$USER/telepathy-ytstenut-$(BRANCH)/spec/
