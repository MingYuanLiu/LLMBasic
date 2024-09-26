cmake_minimum_required(VERSION 3.10)

project(LLMBasic)

# setup libcurl
set(LibCurlHome "${CMAKE_CURRENT_SOURCE_DIR}/libcurl")
set(CurlIncludeDir "${LibCurlHome}/include/")
if(WIN32)
	set(CURL_LIB ${LibCurlHome}/lib/Win64/Release/libcurl.lib)
	set(OPENSSL_LIB ${LibCurlHome}/lib/Win64/Release/libssl.lib)
	set(OPENSSL_CRYPTO_LIB ${LibCurlHome}/lib/Win64/Release/libcrypto.lib)
	set(ZLIB_LIB ${LibCurlHome}/lib/Win64/Release/zlibstatic.lib)
	set(NGHTTP2_LIB ${LibCurlHome}/lib/Win64/Release/nghttp2.lib)
elseif(UNIX)
	set(CURL_LIB "${LibCurlHome}/lib/Unix/x86_64-unknown-linux-gnu/Release/libcurl.a")
endif()

set(JsonInclude "${CMAKE_CURRENT_SOURCE_DIR}/json")
add_library(Json11 ${JsonInclude}/json11.cpp)

file(GLOB_RECURSE HeaderFiles "src/*.h")
file(GLOB_RECURSE SourceFiles "src/*.cpp")

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${HeaderFiles} ${SourceFiles})

include_directories(${JsonInclude})
include_directories(${CurlIncludeDir})

add_executable(LLMBasicTest ${HeaderFiles} ${SourceFiles})

set(CACertificate ${LibCurlHome}/CA/cacert-2024-07-02.pem)
target_compile_definitions(LLMBasicTest PRIVATE CURL_STATICLIB)
target_compile_definitions(LLMBasicTest PRIVATE CA_Certificate_Path=\"${CACertificate}\")

target_link_libraries(LLMBasicTest PRIVATE ${CURL_LIB} ${NGHTTP2_LIB} ${ZLIB_LIB} ${OPENSSL_LIB} ${OPENSSL_CRYPTO_LIB} Ws2_32.lib Wldap32.lib Crypt32.lib)
target_link_libraries(LLMBasicTest PRIVATE $<BUILD_INTERFACE:Json11>)
