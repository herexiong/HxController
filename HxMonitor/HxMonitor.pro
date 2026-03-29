QT += core gui widgets serialport network qml quick quickwidgets quickcontrols2

CONFIG += c++17

SOURCES += \
    localserverhost.cpp \
    main.cpp \
    monitorlabel.cpp \
    widget.cpp

HEADERS += \
    localserverhost.h \
    monitorlabel.h \
    widget.h

RESOURCES += \
    qml.qrc

QML_IMPORT_PATH += $$PWD/third_party/FluentUI/dist

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
