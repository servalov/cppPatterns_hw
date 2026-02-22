cmake_minimum_required(VERSION 3.26.0)
project(spider_project)

set(BOOST_ROOT "C:/Boost/boost_1_90_0")
set(BOOST_LIBRARY_DIRS "C:/Boost/boost_1_90_0/stage/lib") 
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED COMPONENTS locale url)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

set (OpenSSL_ROOT_DIR "C:/Program Files/OpenSSL-Win64")
find_package(OpenSSL REQUIRED)
set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED COMPONENTS Crypto)

file(GLOB SOURCES "source_files/*.cpp") 
file(COPY config_ini_spider.txt cacert.pem DESTINATION ${CMAKE_BINARY_DIR})


add_executable (${PROJECT_NAME} ${SOURCES})

target_compile_options(${PROJECT_NAME} PRIVATE /bigobj)

target_include_directories(${PROJECT_NAME} PRIVATE header_files)

set(PostgreSQL_ADDITIONAL_VERSIONS "17")
find_package(PostgreSQL REQUIRED)
add_subdirectory("C:/libpqxx-7.10.1" libpqxx-build) 
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17) 

target_link_libraries(${PROJECT_NAME} PRIVATE pqxx PostgreSQL::PostgreSQL)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(${PROJECT_NAME} PRIVATE Boost::locale)
target_link_libraries (${PROJECT_NAME} PRIVATE ${Boost_LIBRARIES})

if(MSVC)
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endif()






