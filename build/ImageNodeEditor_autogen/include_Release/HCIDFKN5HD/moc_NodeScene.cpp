/****************************************************************************
** Meta object code from reading C++ file 'NodeScene.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../main/gui/NodeScene.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NodeScene.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_NodeScene_t {
    QByteArrayData data[12];
    char stringdata0[133];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_NodeScene_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_NodeScene_t qt_meta_stringdata_NodeScene = {
    {
QT_MOC_LITERAL(0, 0, 9), // "NodeScene"
QT_MOC_LITERAL(1, 10, 12), // "nodeSelected"
QT_MOC_LITERAL(2, 23, 0), // ""
QT_MOC_LITERAL(3, 24, 5), // "Node*"
QT_MOC_LITERAL(4, 30, 4), // "node"
QT_MOC_LITERAL(5, 35, 14), // "nodeDeselected"
QT_MOC_LITERAL(6, 50, 15), // "connectionAdded"
QT_MOC_LITERAL(7, 66, 17), // "connectionRemoved"
QT_MOC_LITERAL(8, 84, 16), // "workflowModified"
QT_MOC_LITERAL(9, 101, 11), // "showPreview"
QT_MOC_LITERAL(10, 113, 15), // "connectionError"
QT_MOC_LITERAL(11, 129, 3) // "msg"

    },
    "NodeScene\0nodeSelected\0\0Node*\0node\0"
    "nodeDeselected\0connectionAdded\0"
    "connectionRemoved\0workflowModified\0"
    "showPreview\0connectionError\0msg"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_NodeScene[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       7,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   49,    2, 0x06 /* Public */,
       5,    0,   52,    2, 0x06 /* Public */,
       6,    0,   53,    2, 0x06 /* Public */,
       7,    0,   54,    2, 0x06 /* Public */,
       8,    0,   55,    2, 0x06 /* Public */,
       9,    1,   56,    2, 0x06 /* Public */,
      10,    1,   59,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, QMetaType::QString,   11,

       0        // eod
};

void NodeScene::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<NodeScene *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->nodeSelected((*reinterpret_cast< Node*(*)>(_a[1]))); break;
        case 1: _t->nodeDeselected(); break;
        case 2: _t->connectionAdded(); break;
        case 3: _t->connectionRemoved(); break;
        case 4: _t->workflowModified(); break;
        case 5: _t->showPreview((*reinterpret_cast< Node*(*)>(_a[1]))); break;
        case 6: _t->connectionError((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (NodeScene::*)(Node * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&NodeScene::nodeSelected)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (NodeScene::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&NodeScene::nodeDeselected)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (NodeScene::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&NodeScene::connectionAdded)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (NodeScene::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&NodeScene::connectionRemoved)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (NodeScene::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&NodeScene::workflowModified)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (NodeScene::*)(Node * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&NodeScene::showPreview)) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (NodeScene::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&NodeScene::connectionError)) {
                *result = 6;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject NodeScene::staticMetaObject = { {
    QMetaObject::SuperData::link<QGraphicsScene::staticMetaObject>(),
    qt_meta_stringdata_NodeScene.data,
    qt_meta_data_NodeScene,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *NodeScene::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *NodeScene::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_NodeScene.stringdata0))
        return static_cast<void*>(this);
    return QGraphicsScene::qt_metacast(_clname);
}

int NodeScene::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGraphicsScene::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void NodeScene::nodeSelected(Node * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void NodeScene::nodeDeselected()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void NodeScene::connectionAdded()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void NodeScene::connectionRemoved()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void NodeScene::workflowModified()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void NodeScene::showPreview(Node * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void NodeScene::connectionError(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
