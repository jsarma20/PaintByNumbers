set(PAINTING_SRC
${CMAKE_CURRENT_SOURCE_DIR}/image.cpp
${CMAKE_CURRENT_SOURCE_DIR}/painting.cpp
PARENT_SCOPE
)

configure_file(version.h.in version.h)

add_subdirectory(test)
set (CATCH2_TEST_SRC ${CATCH2_TEST_SRC} PARENT_SCOPE)

