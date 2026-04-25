QT += quick quickcontrols2 network
CONFIG += c++17

SOURCES += \
    main.cpp \
    cppsrc/engine.cpp \
    cppsrc/winjudge.cpp \
    cppsrc/network.cpp \
    cppsrc/protocol.cpp \
    cppsrc/fancalc.cpp \
    cppsrc/gamecontroller.cpp

HEADERS += \
    cppsrc/engine.h \
    cppsrc/winjudge.h \
    cppsrc/network.h \
    cppsrc/protocol.h \
    cppsrc/fancalc.h \
    cppsrc/gamecontroller.h

RESOURCES += resources.qrc

android {
    QT += androidextras
    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
}