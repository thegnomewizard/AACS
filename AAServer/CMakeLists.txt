cmake_minimum_required(VERSION 2.8)
project (AAServer)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-g -rdynamic -O3)

find_package(Boost 1.67 REQUIRED COMPONENTS filesystem program_options)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Protobuf REQUIRED)
find_package(fmt REQUIRED)
pkg_check_modules (LIBUSBGX REQUIRED IMPORTED_TARGET libusbgx)
pkg_check_modules (LIBPCAP REQUIRED IMPORTED_TARGET libpcap)
pkg_check_modules (LIBAVDEVICE REQUIRED IMPORTED_TARGET libavdevice)
pkg_check_modules (LIBAVCODEC REQUIRED IMPORTED_TARGET libavcodec)
pkg_check_modules (LIBAVFORMAT REQUIRED IMPORTED_TARGET libavformat)
pkg_check_modules (LIBAVUTIL REQUIRED IMPORTED_TARGET libavutil)
pkg_check_modules (LIBAVFILTER REQUIRED IMPORTED_TARGET libavfilter)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(include)
include_directories(${gst_INCLUDE_DIRS})

include(${CMAKE_CURRENT_SOURCE_DIR}/../proto/CMakeLists.txt)

add_executable(AAServer main.cpp 
    ../src/utils.cpp 
    src/ServerUtils.cpp 
    src/descriptors.cpp
    src/Library.cpp
    src/Function.cpp
    src/MassStorageFunction.cpp
    src/FfsFunction.cpp
    src/Configuration.cpp
    src/Gadget.cpp
    src/Udc.cpp
    src/Message.cpp
    src/ModeSwitcher.cpp
    src/AaCommunicator.cpp
    src/SocketClient.cpp
    src/SocketCommunicator.cpp
    src/ManualResetEvent.cpp
    src/ChannelHandler.cpp
    src/DefaultChannelHandler.cpp
    src/VideoChannelHandler.cpp
    src/InputChannelHandler.cpp
    src/VidCapture.cpp
    src/InputSender.cpp
    src/Uhid.cpp
    ${PROTO_SRCS}
    ${PROTO_HDRS}
    ${BACKWARD_ENABLE}
)

add_backward(AAServer)
target_link_libraries(AAServer Threads::Threads)
target_link_libraries(AAServer Boost::filesystem)
target_link_libraries(AAServer Boost::program_options)
target_link_libraries(AAServer PkgConfig::LIBUSBGX)
target_link_libraries(AAServer OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(AAServer protobuf::libprotobuf)
target_link_libraries(AAServer PkgConfig::LIBAVFORMAT)
target_link_libraries(AAServer PkgConfig::LIBAVUTIL)
target_link_libraries(AAServer PkgConfig::LIBAVDEVICE)
target_link_libraries(AAServer PkgConfig::LIBAVCODEC)
target_link_libraries(AAServer PkgConfig::LIBAVFILTER)
target_link_libraries(AAServer PkgConfig::LIBPCAP)
target_link_libraries(AAServer fmt::fmt)
include_directories(${LIBUSBGX_INCLUDE_DIRS})
include_directories(${LIBPCAP_INCLUDE_DIRS})

add_custom_command(OUTPUT dhparam.pem COMMAND openssl dhparam -out dhparam.pem 2048 > /dev/null 2>&1)
add_custom_target(dhparam DEPENDS dhparam.pem)
add_dependencies(AAServer dhparam)
add_custom_command(TARGET AAServer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/ssl/android_auto.crt
            ${CMAKE_CURRENT_BINARY_DIR}/android_auto.crt)
add_custom_command(TARGET AAServer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/ssl/android_auto.key
            ${CMAKE_CURRENT_BINARY_DIR}/android_auto.key)
