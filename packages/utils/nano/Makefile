#
# Copyright (C) 2007-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=nano
PKG_VERSION:=2.2.5
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://www.nano-editor.org/dist/v2.2
PKG_MD5SUM:=77a10a49589f975ce98350a4527a2ebf

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/nano
  SUBMENU:=Editors
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=An enhanced clone of the Pico text editor
  URL:=http://www.nano-editor.org/
  MAINTAINER:=Bernhard Loos <bernhardloos@googlemail.com>
  DEPENDS:=+libncurses
endef

define Package/nano/description
  GNU nano (Nano's ANOther editor, or Not ANOther editor) is an enhanced clone
  of the Pico text editor.
endef

CONFIGURE_ARGS += \
	--enable-tiny \
	--disable-glibtest \
	--disable-utf8 \
	--without-slang \

CONFIGURE_VARS += \
	ac_cv_header_regex_h=no \

define Package/nano/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/$(PKG_NAME) $(1)/usr/bin/
endef

$(eval $(call BuildPackage,nano))
