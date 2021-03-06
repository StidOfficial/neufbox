#
# Copyright (C) 2006-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mtr
PKG_VERSION:=0.80
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=ftp://ftp.bitwizard.nl/mtr
PKG_MD5SUM:=fa68528eaec1757f52bacf9fea8c68a9

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/mtr
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+libncurses
  TITLE:=Full screen ncurses traceroute tool
  URL:=http://www.bitwizard.nl/mtr/
endef

define Package/mtr/description
	mtr combines the functionality of the 'traceroute' and 'ping' programs
	in a single network diagnostic tool.
	As mtr starts, it investigates the network connection between the host
	mtr runs on and a user-specified destination host. After it
	determines the address of each network hop between the machines,
	it sends a sequence ICMP ECHO requests to each one to determine the
	quality of the link to each machine. As it does this, it prints
	running statistics about each machine.
endef

CONFIGURE_ARGS += \
	--without-gtk \

CONFIGURE_VARS += \
	ac_cv_lib_resolv_res_mkquery=yes \

define Build/Configure
	(cd $(PKG_BUILD_DIR); touch \
		configure.in \
		aclocal.m4 \
		Makefile.in \
		img/Makefile.in \
		stamp-h.in \
		config.h.in \
		configure \
	);
	$(call Build/Configure/Default)
endef

define Package/mtr/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/mtr $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,mtr))
