##########
# the project file in src/
##########

TEMPLATE = lib
equals(QT_MAJOR_VERSION, 4): TARGET = quill
equals(QT_MAJOR_VERSION, 5): TARGET = quill-qt5
INCLUDEPATH += .
DEPENDPATH += .

CONFIG += link_pkgconfig
equals(QT_MAJOR_VERSION, 4): PKGCONFIG += quillmetadata quillimagefilter
equals(QT_MAJOR_VERSION, 5): PKGCONFIG += quillmetadata-qt5 quillimagefilter-qt5

# Avoid automatic casts from QString to QUrl. Dangerous!!!
DEFINES += QT_NO_URL_CAST_FROM_STRING
MOC_DIR = .moc
QT += dbus

equals(QT_MAJOR_VERSION, 5): QT += widgets

include(../common.pri)

CONFIG += quillimagefilter quillmetadata

LIBS += -lexif -lexempi
# Generate pkg-config support by default
# Note that we HAVE TO also create prl config as QMake implementation
# mixes both of them together.
CONFIG += create_pc create_prl no_install_prl

equals(QT_MAJOR_VERSION, 4): QMAKE_PKGCONFIG_REQUIRES = quillimagefilter quillmetadata QtGui
equals(QT_MAJOR_VERSION, 5): QMAKE_PKGCONFIG_REQUIRES = quillimagefilter-qt5 quillmetadata-qt5 Qt5Gui
QMAKE_PKGCONFIG_INCDIR = $$[QT_INSTALL_HEADERS]/$$TARGET
QMAKE_PKGCONFIG_LIBDIR = $$[QT_INSTALL_LIBS]

QMAKE_CXXFLAGS += -Werror
# this is for adding coverage information while doing qmake as "qmake COV_OPTION=on"
# message is shown when 'make' is executed
for(OPTION,$$list($$lower($$COV_OPTION))){
    isEqual(OPTION, on){
        message("TEST COVERAGE IS ENABLED")
        QMAKE_CXXFLAGS += -ftest-coverage -fprofile-arcs -fno-elide-constructors
        LIBS += -lgcov
    }
}

# --- input

HEADERS += quill.h \
           quillfile.h \
           quillerror.h \
           file.h \
           core.h \
           displaylevel.h \
           tilecache.h \
           tilemap.h \
           savemap.h \
           task.h \
           scheduler.h \
           threadmanager.h \
           quillundocommand.h \
           quillundostack.h \
           imagecache.h \
           historyxml.h \
           unix_platform.h \
           logger.h \
           dbus-thumbnailer/dbusthumbnailer.h \
           dbus-thumbnailer/thumbnailer_generic.h \
           backgroundthread.h \
           regionsofinterest.h

SOURCES += quill.cpp \
           quillfile.cpp \
           quillerror.cpp \
           file.cpp \
           core.cpp \
           displaylevel.cpp \
           tilecache.cpp \
           tilemap.cpp \
           savemap.cpp \
           task.cpp \
           scheduler.cpp \
           threadmanager.cpp \
           quillundocommand.cpp \
           quillundostack.cpp \
           imagecache.cpp \
           historyxml.cpp \
           unix_platform.cpp \
           dbus-thumbnailer/dbusthumbnailer.cpp \
           dbus-thumbnailer/thumbnailer_generic.cpp \
           backgroundthread.cpp \
           regionsofinterest.cpp

debug {
    SOURCES += logger.cpp
}

INSTALL_HEADERS = \
           Quill \
           QuillFile \
           QuillError \
           quill.h \
           quillfile.h \
           quillerror.h

# --- install
headers.files = $$INSTALL_HEADERS
headers.path = $$[QT_INSTALL_HEADERS]/$$TARGET
target.path = $$[QT_INSTALL_LIBS]
target.depends += generateproxy
equals(QT_MAJOR_VERSION, 4): pkgconfig.files = quill.pc
equals(QT_MAJOR_VERSION, 5): pkgconfig.files = quill-qt5.pc
pkgconfig.path = $$[QT_INSTALL_LIBS]/pkgconfig
prf.files = quill.prf
prf.path = $$[QMAKE_MKSPECS]/features
INSTALLS += target headers pkgconfig prf

generateproxy.target = dbus-thumbnailer/thumbnailer_generic.h
generateproxy.depends = dbus-thumbnailer/tumbler-service-dbus.xml
generateproxy.commands = qdbusxml2cpp -c ThumbnailerGenericProxy -p dbus-thumbnailer/thumbnailer_generic.h:dbus-thumbnailer/thumbnailer_generic.cpp dbus-thumbnailer/tumbler-service-dbus.xml org.freedesktop.thumbnails.Thumbnailer1

# ---clean
QMAKE_CLEAN += *.gcov *.gcno *.log *.moc_* *.gcda
#               dbus-thumbnailer/thumbnailer_generic.h \
#               dbus-thumbnailer/thumbnailer_generic.cpp

QMAKE_EXTRA_TARGETS += generateproxy

QMAKE_LFLAGS += -Wl,--as-needed
