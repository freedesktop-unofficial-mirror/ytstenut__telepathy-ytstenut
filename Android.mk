LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

TELEPATHY_YTSTENUT_BUILT_SOURCES := \
	telepathy-ytstenut-glib/Android.mk \
	telepathy-ytstenut-glib/extensions/Android.mk \
	telepathy-ytstenut-glib/tests/Android.mk

telepathy-ytstenut-configure-real:
	cd $(TELEPATHY_YTSTENUT_TOP) ; \
	CXX="$(CONFIGURE_CXX)" \
	CC="$(CONFIGURE_CC)" \
	CFLAGS="$(CONFIGURE_CFLAGS)" \
	LD=$(TARGET_LD) \
	LDFLAGS="$(CONFIGURE_LDFLAGS)" \
	CPP=$(CONFIGURE_CPP) \
	CPPFLAGS="$(CONFIGURE_CPPFLAGS)" \
	PKG_CONFIG_LIBDIR=$(CONFIGURE_PKG_CONFIG_LIBDIR) \
	PKG_CONFIG_TOP_BUILD_DIR=$(PKG_CONFIG_TOP_BUILD_DIR) \
	$(TELEPATHY_YTSTENUT_TOP)/$(CONFIGURE) --host=arm-linux-androideabi \
		--disable-spec-documentation --disable-qt4 \
		--disable-Werror && \
	for file in $(TELEPATHY_YTSTENUT_BUILT_SOURCES); do \
		rm -f $$file && \
		make -C $$(dirname $$file) $$(basename $$file) ; \
	done

telepathy-ytstenut-configure: telepathy-ytstenut-configure-real

.PHONY: telepathy-ytstenut-configure

CONFIGURE_TARGETS += telepathy-ytstenut-configure

#include all the subdirs...
-include $(TELEPATHY_YTSTENUT_TOP)/telepathy-ytstenut-glib/Android.mk
-include $(TELEPATHY_YTSTENUT_TOP)/telepathy-ytstenut-glib/extensions/Android.mk
-include $(TELEPATHY_YTSTENUT_TOP)/telepathy-ytstenut-glib/tests/Android.mk
