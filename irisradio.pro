
load(configure)

TARGET = qtmedia_irisradio
QT += multimedia-private

PLUGIN_TYPE = mediaservice
PLUGIN_CLASS_NAME = FMRadioServicePlugin
load(qt_plugin)

SOURCES += fmradioserviceplugin.cpp \
    fmradiodatacontrol.cpp \
    fmradioservice.cpp \
    fmradiotunercontrol.cpp \
    fmradioiriscontrol.cpp

HEADERS += fmradioserviceplugin.h \
    fmradiodatacontrol.h \
    fmradioservice.h \
    fmradiotunercontrol.h \
    fmradioiriscontrol.h \
    radio-iris-commands.h
