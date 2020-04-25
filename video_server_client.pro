ROOT_DIR=../

TEMPLATE = app
#TEMPLATE = lib
TARGET = video_server_client
CONFIG += plugin

include($${ROOT_DIR}pri/common.pri)

#CONFIG += release

QMAKE_CXXFLAGS += -Wno-unused-parameter
QMAKE_CXXFLAGS += -Wno-unused-variable

QT += network

# TODO: add defines to logger, system monitor, restbed webserver, database, etc...
DEFINES += \
#    SWITCH_LOGGER_SIMPLE \
    SWITCH_LOGGER_ASTRA \
    OBJREPR_LIBRARY_EXIST \

# NOTE: paths for dev environment ( all projects sources in one dir )
INCLUDEPATH +=  \
    $${PWD}/from_ms_common/ \

LIBS += \
    -ljsoncpp \
    -lrabbitmq \

contains( DEFINES, OBJREPR_LIBRARY_EXIST ){
    message("connect 'unilog' and 'objrepr' libraries")
LIBS += \
    -lunilog \
    -lobjrepr
}

SOURCES += \
        analyze_handler.cpp \
        archive_handler.cpp \
        commands/cmd_analyze_start.cpp \
        commands/cmd_analyze_status.cpp \
        commands/cmd_analyze_stop.cpp \
        commands/cmd_archive_start.cpp \
        commands/cmd_archive_stop.cpp \
        commands/cmd_archiving_status.cpp \
        commands/cmd_ping.cpp \
        commands/cmd_source_connect.cpp \
        commands/cmd_source_disconnect.cpp \
        commands/i_command.cpp \
        from_ms_common/communication/amqp_client_c.cpp \
        from_ms_common/communication/amqp_controller.cpp \
        from_ms_common/communication/network_interface.cpp \
        from_ms_common/system/logger_astra.cpp \
        from_ms_common/system/logger_normal.cpp \
        from_ms_common/system/logger_simple.cpp \
        main.cpp \
        video_server_client.cpp \
        video_server_handler.cpp \
    from_ms_common/system/object_pool.cpp

HEADERS += \
    analyze_handler.h \
    archive_handler.h \
    commands/cmd_analyze_start.h \
    commands/cmd_analyze_status.h \
    commands/cmd_analyze_stop.h \
    commands/cmd_archive_start.h \
    commands/cmd_archive_stop.h \
    commands/cmd_archiving_status.h \
    commands/cmd_ping.h \
    commands/cmd_source_connect.h \
    commands/cmd_source_disconnect.h \
    commands/i_command.h \
    common_types.h \
    common_types_private.h \
    common_vars.h \
    from_ms_common/common/ms_common_types.h \
    from_ms_common/common/ms_common_utils.h \
    from_ms_common/communication/amqp_client_c.h \
    from_ms_common/communication/amqp_controller.h \
    from_ms_common/communication/network_interface.h \
    from_ms_common/system/logger.h \
    from_ms_common/system/logger_astra.h \
    from_ms_common/system/logger_common.h \
    from_ms_common/system/logger_normal.h \
    from_ms_common/system/logger_simple.h \
    video_server_client.h \
    video_server_handler.h \
    common_utils.h \
    from_ms_common/system/object_pool.h

include($${ROOT_DIR}pri/install.pri)
