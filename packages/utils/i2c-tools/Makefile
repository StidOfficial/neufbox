#
# Copyright (C) 2007-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=i2c-tools
PKG_VERSION:=3.0.2
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=http://dl.lm-sensors.org/i2c-tools/releases/
PKG_MD5SUM:=b546345ac19db56719dea6b8199f11e0

PKG_BUILD_DEPENDS:=python

include $(INCLUDE_DIR)/package.mk
$(call include_mk, python-package.mk)

define Package/i2c-tools
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=I2C tools for Linux
  DEPENDS:=@!LINUX_2_4
  URL:=http://www.lm-sensors.org/wiki/I2CTools
endef

define Package/python-smbus
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Python bindings for the SMBUS
  URL:=http://www.lm-sensors.org/wiki/I2CTools
  DEPENDS:= +python-mini +i2c-tools
endef

define Package/i2c-tools/description
 This package contains an heterogeneous set of I2C tools for Linux. These tools
 were originally part of the lm-sensors package.
 Includes:
 - i2cdetect
 - i2cdump
 - i2cget
 - i2cset
endef

define Package/python-smbus/description
 This package contains an heterogeneous set of I2C tools for Linux. 
 
 This package contain the python bindings for Linux SMBus access through i2c-dev.
endef

TARGET_CPPFLAGS += -I$(LINUX_DIR)/include

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		LINUX="$(LINUX_DIR)" \
		CC="$(TARGET_CC)" \
		STAGING_DIR="$(STAGING_DIR)"
	$(if $(Build/Compile/PyMod),,@echo Python packaging code not found.; false)
	$(call Build/Compile/PyMod,./py-smbus/, \
		install --prefix="$(PKG_INSTALL_DIR)/usr", \
	)
endef

define Package/i2c-tools/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tools/i2cdetect $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tools/i2cdump $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tools/i2cset $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tools/i2cget $(1)/usr/sbin/
endef

define PyPackage/python-smbus/filespec
+|$(PYTHON_PKG_DIR)/smbus.so
endef

$(eval $(call BuildPackage,i2c-tools))
$(eval $(call PyPackage,python-smbus))
$(eval $(call BuildPackage,python-smbus))
