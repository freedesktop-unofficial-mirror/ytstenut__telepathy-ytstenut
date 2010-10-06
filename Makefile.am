ACLOCAL_AMFLAGS = -I m4

SUBDIRS = tp-qt4-tools tp-glib-tools TelepathyFooQt4 telepathy-foo-glib

GIT = git
GZIP = gzip
TAR = tar

XMLS = $(wildcard $(srcdir)/spec/*.xml)
TEMPLATES = $(wildcard $(srcdir)/doc/templates/*)
TOOLS = \
	tools/doc-generator.py \
	tools/specparser.py \
	tools/xincludator.py

GENERATED_FILES = \
	doc/spec/index.html \
	doc/telepathy-foo-spec.tar.bz2

doc/spec/index.html: $(XMLS) $(TOOLS) $(TEMPLATES)
	@install -d doc
	$(AM_V_GEN)$(PYTHON) $(top_srcdir)/tools/doc-generator.py $(top_srcdir)/spec/all.xml $(top_builddir)/doc/spec/ $(PACKAGE) com.example.Foo
	@echo '  NOTE  ' file://$$PWD/$@

doc/telepathy-foo-spec.tar.bz2: doc/spec/index.html
	$(AM_V_GEN)tar cjvf $@ --transform s,doc/spec,telepathy-foo-spec, doc/spec

if SPEC_DOCUMENTATION
all: doc/spec/index.html
else
all:
endif

doc-tarball: doc/telepathy-foo-spec.tar.bz2

EXTRA_DIST = $(XMLS) $(TEMPLATES) tools

CLEANFILES = $(GENERATED_FILES) \
	$(wildcard $(srcdir)/tools/*.pyc)

clean-local:
	-rm -rf doc/spec tmp

BRANCH = $(shell sh $(top_srcdir)/tools/git-which-branch.sh misc | tr -d '\n' | tr -C "[:alnum:]" _)
UPLOAD_BRANCH_TO = dhansak.collabora.co.uk:public_html/telepathy-foo

# Usage: make upload-branch BRANCH=discussion
upload-branch: all
	rsync -rzvP $(patsubst %.txt,%.html,$(RST)) doc/spec \
		$(UPLOAD_BRANCH_TO)-$(BRANCH)/
	@echo Your spec branch might be at:
	@echo '  ' http://people.collabora.co.uk/~$$USER/telepathy-foo-$(BRANCH)/spec/
