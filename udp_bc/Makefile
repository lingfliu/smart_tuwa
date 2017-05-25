#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=udp_bc
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/udp_bc
  SECTION:=udp_bc
  CATEGORY:=Twrt_IP
  TITLE:=Twrt-ip-broadcast
  DEPENDS:=+libncurses +libpthread
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

TARGET_LDFLAGS :=

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS) -Wall" \
		LDFLAGS="$(TARGET_LDFLAGS)"
endef

define Package/udp_bc/install
	$(INSTALL_DIR) $(1)/usr/sbin $(1)/etc/init.d/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/udp_bc $(1)/usr/sbin/
	$(INSTALL_BIN) ./files/udp_bc $(1)/etc/init.d/
endef

$(eval $(call BuildPackage,udp_bc))
