#include <QtQml/qqmlprivate.h>
#include <QtCore/qdir.h>
#include <QtCore/qurl.h>
#include <QtCore/qhash.h>
#include <QtCore/qstring.h>

namespace QmlCacheGeneratedCode {
namespace _qt_qml_TrueSight_ui_AppHeader_qml { 
    extern const unsigned char qmlData[];
    extern const QQmlPrivate::AOTCompiledFunction aotBuiltFunctions[];
    const QQmlPrivate::CachedQmlUnit unit = {
        reinterpret_cast<const QV4::CompiledData::Unit*>(&qmlData), &aotBuiltFunctions[0], nullptr
    };
}
namespace _qt_qml_TrueSight_ui_ChatHeader_qml { 
    extern const unsigned char qmlData[];
    extern const QQmlPrivate::AOTCompiledFunction aotBuiltFunctions[];
    const QQmlPrivate::CachedQmlUnit unit = {
        reinterpret_cast<const QV4::CompiledData::Unit*>(&qmlData), &aotBuiltFunctions[0], nullptr
    };
}
namespace _qt_qml_TrueSight_ui_ConversationEntry_qml { 
    extern const unsigned char qmlData[];
    extern const QQmlPrivate::AOTCompiledFunction aotBuiltFunctions[];
    const QQmlPrivate::CachedQmlUnit unit = {
        reinterpret_cast<const QV4::CompiledData::Unit*>(&qmlData), &aotBuiltFunctions[0], nullptr
    };
}
namespace _qt_qml_TrueSight_ui_ConversationFilterInput_qml { 
    extern const unsigned char qmlData[];
    extern const QQmlPrivate::AOTCompiledFunction aotBuiltFunctions[];
    const QQmlPrivate::CachedQmlUnit unit = {
        reinterpret_cast<const QV4::CompiledData::Unit*>(&qmlData), &aotBuiltFunctions[0], nullptr
    };
}
namespace _qt_qml_TrueSight_ui_ConversationList_qml { 
    extern const unsigned char qmlData[];
    extern const QQmlPrivate::AOTCompiledFunction aotBuiltFunctions[];
    const QQmlPrivate::CachedQmlUnit unit = {
        reinterpret_cast<const QV4::CompiledData::Unit*>(&qmlData), &aotBuiltFunctions[0], nullptr
    };
}
namespace _qt_qml_TrueSight_ui_ConversationView_qml { 
    extern const unsigned char qmlData[];
    extern const QQmlPrivate::AOTCompiledFunction aotBuiltFunctions[];
    const QQmlPrivate::CachedQmlUnit unit = {
        reinterpret_cast<const QV4::CompiledData::Unit*>(&qmlData), &aotBuiltFunctions[0], nullptr
    };
}
namespace _qt_qml_TrueSight_ui_Main_qml { 
    extern const unsigned char qmlData[];
    extern const QQmlPrivate::AOTCompiledFunction aotBuiltFunctions[];
    const QQmlPrivate::CachedQmlUnit unit = {
        reinterpret_cast<const QV4::CompiledData::Unit*>(&qmlData), &aotBuiltFunctions[0], nullptr
    };
}
namespace _qt_qml_TrueSight_ui_MessageBubble_qml { 
    extern const unsigned char qmlData[];
    extern const QQmlPrivate::AOTCompiledFunction aotBuiltFunctions[];
    const QQmlPrivate::CachedQmlUnit unit = {
        reinterpret_cast<const QV4::CompiledData::Unit*>(&qmlData), &aotBuiltFunctions[0], nullptr
    };
}
namespace _qt_qml_TrueSight_ui_MessageComposer_qml { 
    extern const unsigned char qmlData[];
    extern const QQmlPrivate::AOTCompiledFunction aotBuiltFunctions[];
    const QQmlPrivate::CachedQmlUnit unit = {
        reinterpret_cast<const QV4::CompiledData::Unit*>(&qmlData), &aotBuiltFunctions[0], nullptr
    };
}
namespace _qt_qml_TrueSight_ui_SearchField_qml { 
    extern const unsigned char qmlData[];
    extern const QQmlPrivate::AOTCompiledFunction aotBuiltFunctions[];
    const QQmlPrivate::CachedQmlUnit unit = {
        reinterpret_cast<const QV4::CompiledData::Unit*>(&qmlData), &aotBuiltFunctions[0], nullptr
    };
}

}
namespace {
struct Registry {
    Registry();
    ~Registry();
    QHash<QString, const QQmlPrivate::CachedQmlUnit*> resourcePathToCachedUnit;
    static const QQmlPrivate::CachedQmlUnit *lookupCachedUnit(const QUrl &url);
};

Q_GLOBAL_STATIC(Registry, unitRegistry)


Registry::Registry() {
    resourcePathToCachedUnit.insert(QStringLiteral("/qt/qml/TrueSight/ui/AppHeader.qml"), &QmlCacheGeneratedCode::_qt_qml_TrueSight_ui_AppHeader_qml::unit);
    resourcePathToCachedUnit.insert(QStringLiteral("/qt/qml/TrueSight/ui/ChatHeader.qml"), &QmlCacheGeneratedCode::_qt_qml_TrueSight_ui_ChatHeader_qml::unit);
    resourcePathToCachedUnit.insert(QStringLiteral("/qt/qml/TrueSight/ui/ConversationEntry.qml"), &QmlCacheGeneratedCode::_qt_qml_TrueSight_ui_ConversationEntry_qml::unit);
    resourcePathToCachedUnit.insert(QStringLiteral("/qt/qml/TrueSight/ui/ConversationFilterInput.qml"), &QmlCacheGeneratedCode::_qt_qml_TrueSight_ui_ConversationFilterInput_qml::unit);
    resourcePathToCachedUnit.insert(QStringLiteral("/qt/qml/TrueSight/ui/ConversationList.qml"), &QmlCacheGeneratedCode::_qt_qml_TrueSight_ui_ConversationList_qml::unit);
    resourcePathToCachedUnit.insert(QStringLiteral("/qt/qml/TrueSight/ui/ConversationView.qml"), &QmlCacheGeneratedCode::_qt_qml_TrueSight_ui_ConversationView_qml::unit);
    resourcePathToCachedUnit.insert(QStringLiteral("/qt/qml/TrueSight/ui/Main.qml"), &QmlCacheGeneratedCode::_qt_qml_TrueSight_ui_Main_qml::unit);
    resourcePathToCachedUnit.insert(QStringLiteral("/qt/qml/TrueSight/ui/MessageBubble.qml"), &QmlCacheGeneratedCode::_qt_qml_TrueSight_ui_MessageBubble_qml::unit);
    resourcePathToCachedUnit.insert(QStringLiteral("/qt/qml/TrueSight/ui/MessageComposer.qml"), &QmlCacheGeneratedCode::_qt_qml_TrueSight_ui_MessageComposer_qml::unit);
    resourcePathToCachedUnit.insert(QStringLiteral("/qt/qml/TrueSight/ui/SearchField.qml"), &QmlCacheGeneratedCode::_qt_qml_TrueSight_ui_SearchField_qml::unit);
    QQmlPrivate::RegisterQmlUnitCacheHook registration;
    registration.structVersion = 0;
    registration.lookupCachedQmlUnit = &lookupCachedUnit;
    QQmlPrivate::qmlregister(QQmlPrivate::QmlUnitCacheHookRegistration, &registration);
}

Registry::~Registry() {
    QQmlPrivate::qmlunregister(QQmlPrivate::QmlUnitCacheHookRegistration, quintptr(&lookupCachedUnit));
}

const QQmlPrivate::CachedQmlUnit *Registry::lookupCachedUnit(const QUrl &url) {
    if (url.scheme() != QLatin1String("qrc"))
        return nullptr;
    QString resourcePath = QDir::cleanPath(url.path());
    if (resourcePath.isEmpty())
        return nullptr;
    if (!resourcePath.startsWith(QLatin1Char('/')))
        resourcePath.prepend(QLatin1Char('/'));
    return unitRegistry()->resourcePathToCachedUnit.value(resourcePath, nullptr);
}
}
int QT_MANGLE_NAMESPACE(qInitResources_qmlcache_TrueSightClient)() {
    ::unitRegistry();
    return 1;
}
Q_CONSTRUCTOR_FUNCTION(QT_MANGLE_NAMESPACE(qInitResources_qmlcache_TrueSightClient))
int QT_MANGLE_NAMESPACE(qCleanupResources_qmlcache_TrueSightClient)() {
    return 1;
}
