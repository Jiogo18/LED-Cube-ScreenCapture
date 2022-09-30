QT += widgets websockets

INCLUDEPATH += Bcrypt.cpp/include

SOURCES += \
    Bcrypt.cpp/src/bcrypt.cpp \
    Bcrypt.cpp/src/blowfish.cpp \
    Sender.cpp \
    fenetre.cpp \
    main.cpp

HEADERS += \
    Bcrypt.cpp/include/bcrypt.h \
    Sender.h \
    fenetre.h

FORMS += \
    fenetre.ui
