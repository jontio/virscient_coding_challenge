TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        ../common/errormsg.c \
        ../common/protocol.c \
        ../common/sio.c \
        main.c \
        parse_program_args.c \
        protocol_server.c \
        server.c

HEADERS += \
    ../common/errormsg.h \
    ../common/protocol.h \
    ../common/sio.h \
    parse_program_args.h \
    protocol_server.h \
    server.h

DISTFILES += \
    junk
