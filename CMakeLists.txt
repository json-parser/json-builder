
cmake_minimum_required (VERSION 2.6)
project (json-builder)

set (CMAKE_C_FLAGS "-std=c89 -ansi -pedantic -Wall -I../json-parser ${CMAKE_C_FLAGS}")

set (SOURCES ${SOURCES} json-builder.c)

add_library (jsonbuilder ${SOURCES})

set_target_properties (jsonbuilder PROPERTIES VERSION 0.1.0
                                              SOVERSION 0.1)

target_link_libraries (jsonbuilder ${LIBS})

install (TARGETS jsonbuilder DESTINATION lib)
install (FILES json-builder.h DESTINATION include)


