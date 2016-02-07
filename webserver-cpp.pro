TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

QMAKE_CXXFLAGS +=    -Wall -Wextra -std=c++11 -pedantic -Weffc++\
                    -Wunreachable-code -Wshadow \
                    -Wnon-virtual-dtor  \
                    -Woverloaded-virtual \
                    -Wenum-compare  \
                                        #OFF for QT
                    -Wctor-dtor-privacy	\
                    -Wsign-conversion  \
                    -Wconversion \

