add_subdirectory(src)

# create Painting shared library
add_library(Painting SHARED ${PAINTING_SRC})

# find_package for these external lib should already be called at this point
target_link_libraries(Painting PUBLIC ${OpenCV_LIBS})
target_link_libraries(Painting PRIVATE spdlog::spdlog_header_only)

set_target_properties(Painting PROPERTIES 
                        VERSION ${PROJECT_VERSION} 
                        SOVERSION ${PROJECT_VERSION_MAJOR}
                        PUBLIC_HEADER include/engine.h
)

target_include_directories(Painting PUBLIC include)
target_include_directories(Painting PUBLIC src)
#target_compile_definitions(Painting PRIVATE DEV_GUI=1)

set (CATCH2_TEST_SRC ${CATCH2_TEST_SRC} PARENT_SCOPE)
