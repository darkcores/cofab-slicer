QT       += core gui 3dcore 3drender 3dinput 3dextras 3dlogic 3danimation

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

QMAKE_CXXFLAGS_RELEASE = "-march=native -O3 -msse -msse2 -msse3 -mssse3"

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    src/*.cpp

HEADERS += \
    src/*.h

DESTDIR=bin #Target file directory
OBJECTS_DIR=generated_files #Intermediate object files directory
MOC_DIR=generated_files #Intermediate moc files directory

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# Dependecy for assimp
LIBS += -l assimp
