#-------------------------------------------------
#
# Project created by QtCreator 2011-10-12T16:18:57
#
#-------------------------------------------------

QT       -= core gui

TARGET = common
TEMPLATE = lib

DEFINES += COMMON_LIBRARY

SOURCES += \
    Common.cpp \
    Registry.cpp \
    segvcatch.cpp \
    Variant.cpp \
    Timer.cpp \
    Thread.cpp \
    Lockable.cpp \
    PasswordKeeper.cpp \
    ComPort.cpp \
    PasswordArray.cpp \
    MLString.cpp \
    Logger.cpp \
    Error.cpp

HEADERS +=\
        common_global.h \
    Common.h \
    Registry.h \
    segvcatch.h \
    x86_64-signal.h \
    Variant.h \
    tstring.h \
    Timer.h \
    Thread.h \
    smart_ptr.h \
    Queue.h \
    Pool.h \
    PasswordKeeper.h \
    PasswordArray.h \
    MLString.h \
    Logger.h \
    Lockable.h \
    i386-signal.h \
    Error.h \
    DataCache.h \
    ComPort.h \
    auto_array.h

symbian {
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xE52CF902
    TARGET.CAPABILITY = 
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = common.dll
    addFiles.path = !:/sys/bin
    DEPLOYMENT += addFiles
}

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

OTHER_FILES += \
    makefile











