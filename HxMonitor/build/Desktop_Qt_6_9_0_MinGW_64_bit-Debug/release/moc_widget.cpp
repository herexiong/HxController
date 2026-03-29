/****************************************************************************
** Meta object code from reading C++ file 'widget.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../widget.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'widget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.9.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN6WidgetE_t {};
} // unnamed namespace

template <> constexpr inline auto Widget::qt_create_metaobjectdata<qt_meta_tag_ZN6WidgetE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "Widget",
        "recivedone",
        "",
        "performanceOverviewChanged",
        "componentCardsChanged",
        "historiesChanged",
        "serialPortsChanged",
        "selectedPortChanged",
        "serialConnectedChanged",
        "mediaItemsChanged",
        "activeTabIdChanged",
        "nmStatusChanged",
        "nmLogsChanged",
        "readProcessData",
        "resolvedata",
        "iconActivated",
        "QSystemTrayIcon::ActivationReason",
        "reason",
        "onNativeMessage",
        "rawJson",
        "onNmLog",
        "log",
        "onServerStatusChanged",
        "openLogViewer",
        "refreshPorts",
        "setSelectedPort",
        "portDisplay",
        "toggleSerialConnection",
        "refreshMediaList",
        "playPause",
        "tabId",
        "playNext",
        "playPrevious",
        "togglePip",
        "activateMedia",
        "performanceOverview",
        "QVariantMap",
        "componentCards",
        "QVariantList",
        "cpuHistory",
        "gpuHistory",
        "memoryHistory",
        "networkHistory",
        "debugMode",
        "serialPorts",
        "selectedPort",
        "serialConnected",
        "mediaItems",
        "activeTabId",
        "nmStatusText",
        "nmStatusTone",
        "msgSentCount",
        "msgRecvCount",
        "nmLogs"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'recivedone'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'performanceOverviewChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'componentCardsChanged'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'historiesChanged'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'serialPortsChanged'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'selectedPortChanged'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'serialConnectedChanged'
        QtMocHelpers::SignalData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'mediaItemsChanged'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'activeTabIdChanged'
        QtMocHelpers::SignalData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'nmStatusChanged'
        QtMocHelpers::SignalData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'nmLogsChanged'
        QtMocHelpers::SignalData<void()>(12, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'readProcessData'
        QtMocHelpers::SlotData<void()>(13, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'resolvedata'
        QtMocHelpers::SlotData<void()>(14, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'iconActivated'
        QtMocHelpers::SlotData<void(QSystemTrayIcon::ActivationReason)>(15, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 16, 17 },
        }}),
        // Slot 'onNativeMessage'
        QtMocHelpers::SlotData<void(const QByteArray &)>(18, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QByteArray, 19 },
        }}),
        // Slot 'onNmLog'
        QtMocHelpers::SlotData<void(const QString &)>(20, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 21 },
        }}),
        // Slot 'onServerStatusChanged'
        QtMocHelpers::SlotData<void()>(22, 2, QMC::AccessPrivate, QMetaType::Void),
        // Method 'openLogViewer'
        QtMocHelpers::MethodData<void()>(23, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'refreshPorts'
        QtMocHelpers::MethodData<void()>(24, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'setSelectedPort'
        QtMocHelpers::MethodData<void(const QString &)>(25, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 26 },
        }}),
        // Method 'toggleSerialConnection'
        QtMocHelpers::MethodData<void()>(27, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'refreshMediaList'
        QtMocHelpers::MethodData<void()>(28, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'playPause'
        QtMocHelpers::MethodData<void(int)>(29, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 30 },
        }}),
        // Method 'playPause'
        QtMocHelpers::MethodData<void()>(29, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void),
        // Method 'playNext'
        QtMocHelpers::MethodData<void(int)>(31, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 30 },
        }}),
        // Method 'playNext'
        QtMocHelpers::MethodData<void()>(31, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void),
        // Method 'playPrevious'
        QtMocHelpers::MethodData<void(int)>(32, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 30 },
        }}),
        // Method 'playPrevious'
        QtMocHelpers::MethodData<void()>(32, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void),
        // Method 'togglePip'
        QtMocHelpers::MethodData<void(int)>(33, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 30 },
        }}),
        // Method 'togglePip'
        QtMocHelpers::MethodData<void()>(33, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void),
        // Method 'activateMedia'
        QtMocHelpers::MethodData<void(int)>(34, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 30 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'performanceOverview'
        QtMocHelpers::PropertyData<QVariantMap>(35, 0x80000000 | 36, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 1),
        // property 'componentCards'
        QtMocHelpers::PropertyData<QVariantList>(37, 0x80000000 | 38, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 2),
        // property 'cpuHistory'
        QtMocHelpers::PropertyData<QVariantList>(39, 0x80000000 | 38, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 3),
        // property 'gpuHistory'
        QtMocHelpers::PropertyData<QVariantList>(40, 0x80000000 | 38, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 3),
        // property 'memoryHistory'
        QtMocHelpers::PropertyData<QVariantList>(41, 0x80000000 | 38, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 3),
        // property 'networkHistory'
        QtMocHelpers::PropertyData<QVariantList>(42, 0x80000000 | 38, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 3),
        // property 'debugMode'
        QtMocHelpers::PropertyData<bool>(43, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Constant),
        // property 'serialPorts'
        QtMocHelpers::PropertyData<QStringList>(44, QMetaType::QStringList, QMC::DefaultPropertyFlags, 4),
        // property 'selectedPort'
        QtMocHelpers::PropertyData<QString>(45, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 5),
        // property 'serialConnected'
        QtMocHelpers::PropertyData<bool>(46, QMetaType::Bool, QMC::DefaultPropertyFlags, 6),
        // property 'mediaItems'
        QtMocHelpers::PropertyData<QVariantList>(47, 0x80000000 | 38, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 7),
        // property 'activeTabId'
        QtMocHelpers::PropertyData<int>(48, QMetaType::Int, QMC::DefaultPropertyFlags, 8),
        // property 'nmStatusText'
        QtMocHelpers::PropertyData<QString>(49, QMetaType::QString, QMC::DefaultPropertyFlags, 9),
        // property 'nmStatusTone'
        QtMocHelpers::PropertyData<QString>(50, QMetaType::QString, QMC::DefaultPropertyFlags, 9),
        // property 'msgSentCount'
        QtMocHelpers::PropertyData<int>(51, QMetaType::Int, QMC::DefaultPropertyFlags, 9),
        // property 'msgRecvCount'
        QtMocHelpers::PropertyData<int>(52, QMetaType::Int, QMC::DefaultPropertyFlags, 9),
        // property 'nmLogs'
        QtMocHelpers::PropertyData<QStringList>(53, QMetaType::QStringList, QMC::DefaultPropertyFlags, 10),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<Widget, qt_meta_tag_ZN6WidgetE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject Widget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN6WidgetE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN6WidgetE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN6WidgetE_t>.metaTypes,
    nullptr
} };

void Widget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<Widget *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->recivedone(); break;
        case 1: _t->performanceOverviewChanged(); break;
        case 2: _t->componentCardsChanged(); break;
        case 3: _t->historiesChanged(); break;
        case 4: _t->serialPortsChanged(); break;
        case 5: _t->selectedPortChanged(); break;
        case 6: _t->serialConnectedChanged(); break;
        case 7: _t->mediaItemsChanged(); break;
        case 8: _t->activeTabIdChanged(); break;
        case 9: _t->nmStatusChanged(); break;
        case 10: _t->nmLogsChanged(); break;
        case 11: _t->readProcessData(); break;
        case 12: _t->resolvedata(); break;
        case 13: _t->iconActivated((*reinterpret_cast< std::add_pointer_t<QSystemTrayIcon::ActivationReason>>(_a[1]))); break;
        case 14: _t->onNativeMessage((*reinterpret_cast< std::add_pointer_t<QByteArray>>(_a[1]))); break;
        case 15: _t->onNmLog((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 16: _t->onServerStatusChanged(); break;
        case 17: _t->openLogViewer(); break;
        case 18: _t->refreshPorts(); break;
        case 19: _t->setSelectedPort((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 20: _t->toggleSerialConnection(); break;
        case 21: _t->refreshMediaList(); break;
        case 22: _t->playPause((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 23: _t->playPause(); break;
        case 24: _t->playNext((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 25: _t->playNext(); break;
        case 26: _t->playPrevious((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 27: _t->playPrevious(); break;
        case 28: _t->togglePip((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 29: _t->togglePip(); break;
        case 30: _t->activateMedia((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (Widget::*)()>(_a, &Widget::recivedone, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (Widget::*)()>(_a, &Widget::performanceOverviewChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (Widget::*)()>(_a, &Widget::componentCardsChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (Widget::*)()>(_a, &Widget::historiesChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (Widget::*)()>(_a, &Widget::serialPortsChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (Widget::*)()>(_a, &Widget::selectedPortChanged, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (Widget::*)()>(_a, &Widget::serialConnectedChanged, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (Widget::*)()>(_a, &Widget::mediaItemsChanged, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (Widget::*)()>(_a, &Widget::activeTabIdChanged, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (Widget::*)()>(_a, &Widget::nmStatusChanged, 9))
            return;
        if (QtMocHelpers::indexOfMethod<void (Widget::*)()>(_a, &Widget::nmLogsChanged, 10))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<QVariantMap*>(_v) = _t->performanceOverview(); break;
        case 1: *reinterpret_cast<QVariantList*>(_v) = _t->componentCards(); break;
        case 2: *reinterpret_cast<QVariantList*>(_v) = _t->cpuHistory(); break;
        case 3: *reinterpret_cast<QVariantList*>(_v) = _t->gpuHistory(); break;
        case 4: *reinterpret_cast<QVariantList*>(_v) = _t->memoryHistory(); break;
        case 5: *reinterpret_cast<QVariantList*>(_v) = _t->networkHistory(); break;
        case 6: *reinterpret_cast<bool*>(_v) = _t->debugMode(); break;
        case 7: *reinterpret_cast<QStringList*>(_v) = _t->serialPorts(); break;
        case 8: *reinterpret_cast<QString*>(_v) = _t->selectedPort(); break;
        case 9: *reinterpret_cast<bool*>(_v) = _t->serialConnected(); break;
        case 10: *reinterpret_cast<QVariantList*>(_v) = _t->mediaItems(); break;
        case 11: *reinterpret_cast<int*>(_v) = _t->activeTabIdProperty(); break;
        case 12: *reinterpret_cast<QString*>(_v) = _t->nmStatusText(); break;
        case 13: *reinterpret_cast<QString*>(_v) = _t->nmStatusTone(); break;
        case 14: *reinterpret_cast<int*>(_v) = _t->msgSentCountProperty(); break;
        case 15: *reinterpret_cast<int*>(_v) = _t->msgRecvCountProperty(); break;
        case 16: *reinterpret_cast<QStringList*>(_v) = _t->nmLogs(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 8: _t->setSelectedPort(*reinterpret_cast<QString*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *Widget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Widget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN6WidgetE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int Widget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 31)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 31;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 31)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 31;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 17;
    }
    return _id;
}

// SIGNAL 0
void Widget::recivedone()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void Widget::performanceOverviewChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void Widget::componentCardsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void Widget::historiesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void Widget::serialPortsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void Widget::selectedPortChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void Widget::serialConnectedChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void Widget::mediaItemsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void Widget::activeTabIdChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}

// SIGNAL 9
void Widget::nmStatusChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 9, nullptr);
}

// SIGNAL 10
void Widget::nmLogsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 10, nullptr);
}
QT_WARNING_POP
