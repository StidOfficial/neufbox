#
# Copyright (C) 2006-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=alsa-lib
PKG_VERSION:=1.0.23
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=ftp://ftp.alsa-project.org/pub/lib/ \
		http://alsa.cybermirror.org/lib/
PKG_MD5SUM:=f48b50421d8a69d2d806d9c47e534f0d

PKG_FIXUP:=libtool
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

ifeq ($(KERNEL),2.4)
DEPENDS_KMOD:=+kmod-alsa
else
DEPENDS_KMOD:=+kmod-sound-core
endif

define Package/alsa-lib
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=ALSA (Advanced Linux Sound Architecture) library
  URL:=http://www.alsa-project.org/
  DEPENDS:=@AUDIO_SUPPORT $(DEPENDS_KMOD)
endef

define Package/alsa-lib/description
 This is the library package for alsa, needed by some userspace programs.
 You must have enabled the ALSA support in the kernel.
endef

TARGET_CFLAGS += $(FPIC)

define Build/Configure
	$(call Build/Configure/Default, \
	    --enable-static \
		--disable-python \
		--disable-debug \
		--without-debug \
		$(SOFT_FLOAT_CONFIG_OPTION) \
		--with-versioned=no \
	)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/alsa \
		$(1)/usr/include/

	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libasound.{la,a,so*} \
		$(1)/usr/lib/
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/alsa.pc \
		$(1)/usr/lib/pkgconfig/

	$(INSTALL_DIR) $(1)/usr/share/aclocal
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/share/aclocal/alsa.m4 \
		$(1)/usr/share/aclocal/
endef

define Package/alsa-lib/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libasound.so.* \
		$(1)/usr/lib/

	$(INSTALL_DIR) $(1)/usr/share/alsa/{cards,pcm}
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/share/alsa/alsa.conf \
		$(1)/usr/share/alsa/
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/share/alsa/pcm/* \
		$(1)/usr/share/alsa/pcm/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/share/alsa/cards/* \
		$(1)/usr/share/alsa/cards/
endef

$(eval $(call BuildPackage,alsa-lib))
