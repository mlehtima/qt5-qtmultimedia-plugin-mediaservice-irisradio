
load(configure)

TARGET = qtmedia_fmdrv
QT += multimedia-private

PLUGIN_TYPE = mediaservice
PLUGIN_CLASS_NAME = FMRadioServicePlugin
load(qt_plugin)

SOURCES += fmradioserviceplugin.cpp \
    fmradiodatacontrol.cpp \
    fmradioservice.cpp \
    fmradiotunercontrol.cpp \
    fmradiofmdrvcontrol.cpp

HEADERS += fmradioserviceplugin.h \
    fmradiodatacontrol.h \
    fmradioservice.h \
    fmradiotunercontrol.h \
    fmradiofmdrvcontrol.h \
    radio-fmdrv-commands.h
