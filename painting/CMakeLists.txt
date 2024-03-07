cmake_minimum_required(VERSION 3.2)
project(Painter VERSION 1.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

#set(CMAKE_VERBOSE_MAKEFILE ON)
message("CMAKE standard ${CMAKE_CXX_STANDARD}")

# Add external dependencies
add_subdirectory(dep) #dep installed path
find_package(OpenCV REQUIRED)

if(NOT TARGET spdlog)
    # Stand-alone build
    find_package(spdlog REQUIRED)
endif()

add_subdirectory(painting)

# Build PaintByNumbers App
add_executable(PBN pbnMain.cpp)

target_link_libraries(PBN PUBLIC ${OpenCV_LIBS} )
target_link_libraries(PBN PRIVATE spdlog::spdlog_header_only)
target_link_libraries(PBN PUBLIC Painting)


##### Add Test Files
if( WITH_TESTS)
    find_package(Catch2 3 REQUIRED)
    add_executable(TestRunner ${CATCH2_TEST_SRC})
    target_link_libraries(TestRunner PRIVATE Catch2::Catch2WithMain)
    target_link_libraries(TestRunner PUBLIC ${OpenCV_LIBS})
    target_link_libraries(TestRunner PRIVATE spdlog::spdlog_header_only)
    target_link_libraries(TestRunner PUBLIC Painting)
endif()
