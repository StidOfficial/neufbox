#
# Copyright (C) 2009-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=umurmur
PKG_VERSION:=0.2.4
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://umurmur.googlecode.com/files/
PKG_MD5SUM:=ab5cc15b4b1d827443f7950cd935bc6a

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/umurmur/Default
  SECTION:=net
  CATEGORY:=Network
  TITLE:=uMurmur
  DEPENDS:=+libconfig
  URL:=http://code.google.com/p/umurmur
  MAINTAINER:=Martin Johansson <martin@fatbob.nu>
endef

define Package/umurmur/Default/description
  Minimalistic Mumble server daemon.
endef

define Package/umurmur-openssl
  $(call Package/umurmur/Default)
  TITLE+= (with OpenSSL support)
  DEPENDS+= +libopenssl
  VARIANT:=openssl
endef

define Package/umurmur-openssl/description
  $(call Package/umurmur/Default/description)
  Uses OpenSSL library for SSL and crypto.
endef

define Package/umurmur-polarssl
  $(call Package/umurmur/Default)
  TITLE+= (with PolarSSL support)
  DEPENDS+= +libpolarssl
  VARIANT:=polarssl
endef

define Package/umurmur-polarssl/description
  $(call Package/umurmur/Default/description)
  Uses the PolarSSL library for SSL and crypto.
endef

define Build/Compile
	CC="$(TARGET_CC)" \
	CFLAGS="$(TARGET_CFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)" \
	$(MAKE) -C $(PKG_BUILD_DIR)/src all
endef

define Package/umurmur-openssl/conffiles
/etc/umurmur.conf
endef

Package/umurmur-polarssl/conffiles = $(Package/umurmur-openssl/conffiles)

define Package/umurmur-openssl/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/umurmurd $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/openwrt/files/umurmur.conf $(1)/etc/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/openwrt/files/umurmur.init $(1)/etc/init.d/umurmur
	$(INSTALL_DIR) $(1)/etc/umurmur
endef

Package/umurmur-polarssl/install = $(Package/umurmur-openssl/install)

ifeq ($(BUILD_VARIANT),openssl)
  TARGET_CFLAGS += -DWRT_TARGET -I$(STAGING_DIR)/usr/include
  TARGET_LDFLAGS += -lcrypto -lssl
endif

ifeq ($(BUILD_VARIANT),polarssl)
  TARGET_CFLAGS += -DUSE_POLARSSL -DWRT_TARGET -I$(STAGING_DIR)/usr/include
  TARGET_LDFLAGS += -lpolarssl
endif

$(eval $(call BuildPackage,umurmur-openssl))
$(eval $(call BuildPackage,umurmur-polarssl))
