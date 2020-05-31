message("adding teamspeak-plugin-qt-common")

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS Core Sql REQUIRED)
link_libraries(Qt5::Core Qt5::Sql)

include_directories(
    "${CMAKE_CURRENT_LIST_DIR}/ts3client-pluginsdk/include"
    "${CMAKE_CURRENT_LIST_DIR}/core"
    "${CMAKE_CURRENT_LIST_DIR}/deps/gsl/include"
)

set (TS_QT_CORE
    "${CMAKE_CURRENT_LIST_DIR}/core/core/definitions.h"
    "${CMAKE_CURRENT_LIST_DIR}/core/core/client_storage.h"
    "${CMAKE_CURRENT_LIST_DIR}/core/core/plugin_base.h"
    "${CMAKE_CURRENT_LIST_DIR}/core/core/translator.h"
    "${CMAKE_CURRENT_LIST_DIR}/core/core/module.h"
    "${CMAKE_CURRENT_LIST_DIR}/core/core/module_qt.h"
    "${CMAKE_CURRENT_LIST_DIR}/core/core/ts_helpers_qt.h"
    "${CMAKE_CURRENT_LIST_DIR}/core/core/ts_settings_qt.h"
    "${CMAKE_CURRENT_LIST_DIR}/core/core/ts_logging_qt.h"
    "${CMAKE_CURRENT_LIST_DIR}/core/core/ts_context_menu_qt.h"
    "${CMAKE_CURRENT_LIST_DIR}/core/core/ts_infodata_qt.h"
    "${CMAKE_CURRENT_LIST_DIR}/core/core/ts_servergroups.h"
    "${CMAKE_CURRENT_LIST_DIR}/core/core/ts_serversinfo.h"
    "${CMAKE_CURRENT_LIST_DIR}/core/core/ts_serverinfo_qt.h"
    "${CMAKE_CURRENT_LIST_DIR}/core/core/talkers.h"
    "${CMAKE_CURRENT_LIST_DIR}/core/plugin_base.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/core/translator.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/core/module.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/core/module_qt.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/core/ts_helpers_qt.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/core/ts_settings_qt.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/core/ts_logging_qt.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/core/ts_context_menu_qt.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/core/ts_infodata_qt.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/core/ts_servergroups.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/core/ts_serversinfo.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/core/ts_serverinfo_qt.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/core/talkers.cpp"
)

# Create named folders for the sources within the .vcproj
# Empty name lists them directly under the .vcproj
source_group("ts_qt_core" FILES ${TS_QT_CORE})

if (WITH_VOLUME OR WITH_VOLUME_WIDGETS)
    message("adding volume")
    set (TS_QT_VOLUME
        "${CMAKE_CURRENT_LIST_DIR}/volume/volume/dsp_volume.h"
        "${CMAKE_CURRENT_LIST_DIR}/volume/dsp_volume.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/volume/volume/dsp_volume_agmu.h"
        "${CMAKE_CURRENT_LIST_DIR}/volume/dsp_volume_agmu.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/volume/volume/dsp_volume_ducker.h"
        "${CMAKE_CURRENT_LIST_DIR}/volume/dsp_volume_ducker.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/volume/volume/volumes.h"
    )
    
    include_directories(
        "${CMAKE_CURRENT_LIST_DIR}/volume"
    )
    if (WITH_VOLUME_WIDGETS)
        message("adding volume widgets")
        set(CMAKE_AUTOUIC ON)
        find_package(Qt5 COMPONENTS Gui Widgets REQUIRED)
        link_libraries(Qt5::Gui Qt5::Widgets)
        set (TS_QT_VOLUME
            ${TS_QT_VOLUME}
            "${CMAKE_CURRENT_LIST_DIR}/volume_widgets/volume_widgets/fader_vertical.h"
            "${CMAKE_CURRENT_LIST_DIR}/volume_widgets/fader_vertical.cpp"
            "${CMAKE_CURRENT_LIST_DIR}/volume_widgets/fader_vertical.ui"
        )
        include_directories("${CMAKE_CURRENT_LIST_DIR}/volume_widgets")
    endif (WITH_VOLUME_WIDGETS)
    source_group("ts_qt_volume" FILES ${TS_QT_VOLUME})
endif (WITH_VOLUME OR WITH_VOLUME_WIDGETS)
