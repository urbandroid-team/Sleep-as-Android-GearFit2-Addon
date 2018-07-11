# Add inputs and outputs from these tool invocations to the build variables

C_DEPS +=

USES_EFL = yes
USES_USR_INC = yes

SYSROOT = $(SBI_SYSROOT)

ifneq ($(strip $(SYSROOT)),)

ifeq ($(strip $(USES_EFL)),yes)
_EFL_INCS := \
     ecore-1 \
     ecore-audio-1 \
     ecore-avahi-1 \
     ecore-con-1 \
     ecore-evas-1 \
     ecore-fb-1 \
     ecore-file-1  \
     ecore-imf-1 \
     ecore-imf-evas-1 \
     ecore-input-1 \
     ecore-input-evas-1 \
     ecore-ipc-1 \
     ecore-x-1 \
     e_dbus-1 \
     edje-1 \
     eet-1 \
     efl-1 \
     efl-extension efl-extension/default efl-extension/circle \
     efreet-1 \
     eina-1 eina-1/eina \
     eio-1 \
     eldbus-1 \
     elementary-1 \
     embryo-1 \
     eo-1 \
     eom \
     ethumb-1 \
     ethumb-client-1 \
     evas-1 \
     ewebkit2-0
_EFL_INC_ROOT := $(SYSROOT)/usr/include
EFL_INCS := $(addprefix -I $(_EFL_INC_ROOT)/,$(_EFL_INCS))
endif

ifeq ($(strip $(USES_USR_INC)),yes)
_USR_INCS := \
     usr/include \
     usr/include/AL \
     usr/include/feedback \
     usr/include/ui-extension \
     usr/include/vconf \
     usr/include/weconn \
     usr/include/wipc \
     usr/include/wnoti-service \
     usr/include/appcore-agent \
     usr/include/appcore-watch \
     usr/include/appfw \
     usr/include/badge \
     usr/include/base \
     usr/include/cairo \
     usr/include/calendar-service2 \
     usr/include/ckm \
     usr/include/contacts-svc \
     usr/include/content \
     usr/include/context-manager \
     usr/include/context-service \
     usr/include/dali \
     usr/include/dali-toolkit \
     usr/include/dbus-1.0 \
     usr/include/device \
     usr/include/dlog \
     usr/include/fontconfig \
     usr/include/freetype2 \
     usr/include/geofence \
     usr/include/gio-unix-2.0 \
     usr/include/glib-2.0 \
     usr/include/json-glib-1.0 \
     usr/include/json-glib-1.0/json-glib \
     usr/include/libxml2 \
     usr/include/location \
     usr/include/maps \
     usr/include/media \
     usr/include/media-content \
     usr/include/messaging \
     usr/include/metadata-editor \
     usr/include/minicontrol \
     usr/include/minizip \
     usr/include/network \
     usr/include/notification \
     usr/include/phonenumber-utils \
     usr/include/sensor \
     usr/include/shortcut \
     usr/include/storage \
     usr/include/system \
     usr/include/telephony \
     usr/include/ui \
     usr/include/web \
     usr/include/widget_service \
     usr/include/widget_viewer_evas \
     usr/include/wifi-direct \
     usr/lib/dbus-1.0/include \
     usr/lib/glib-2.0/include 
_USR_INC_ROOT := $(SYSROOT)
USR_INCS := $(addprefix -I $(_USR_INC_ROOT)/,$(_USR_INCS))
endif

PLATFORM_INCS = $(USR_INCS) $(EFL_INCS) \
     -I"$(SDK_PATH)/library" 

endif
