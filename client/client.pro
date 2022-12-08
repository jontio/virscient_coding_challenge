TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

#can override default setting in pro file. eg
#DEFINES += PARSE_PROGRAM_ARGS_DEFAULT_SERVER_HOST_ADDRESS=\\"\"localhost\\"\"

SOURCES += \
        ../common/errormsg.c \
        ../common/protocol.c \
        ../common/sio.c \
        client.c \
        main.c \
        parse_program_args.c \
        protocol_client.c

HEADERS += \
    ../common/errormsg.h \
    ../common/protocol.h \
    ../common/sio.h \
    client.h \
    parse_program_args.h \
    protocol_client.h


