cmake_minimum_required(VERSION 3.10)

project(LLMBasic)

set(LibCurlHome "${CMAKE_CURRENT_SOURCE_DIR}/libcurl")
set(CURL_INCLUDE_DIR "${LibCurlHome}/include/")

if(WIN32)
	set(CURL_LIB ${LibCurlHome}/lib/Win64/Release/libcurl.lib)
	set(OPENSSL_LIB ${LibCurlHome}/lib/Win64/Release/libssl.lib)
	set(OPENSSL_CRYPTO_LIB ${LibCurlHome}/lib/Win64/Release/libcrypto.lib)
	set(ZLIB_LIB ${LibCurlHome}/lib/Win64/Release/zlibstatic.lib)
	set(NGHTTP2_LIB ${LibCurlHome}/lib/Win64/Release/nghttp2.lib)
	
elseif(UNIX)
	set(CURL_LIB "${LibCurlHome}/lib/Unix/x86_64-unknown-linux-gnu/Release/libcurl.a")
	
endif()

set(CACertificate ${LibCurlHome}/CA/cacert-2024-07-02.pem)

include_directories(${CURL_INCLUDE_DIR})

add_executable(LLMBasicTest src/main.cpp)

target_compile_definitions(LLMBasicTest PRIVATE CURL_STATICLIB)
target_compile_definitions(LLMBasicTest PRIVATE CA_Certificate_Path=\"${CACertificate}\")

target_link_libraries(LLMBasicTest PRIVATE ${CURL_LIB} ${NGHTTP2_LIB} ${ZLIB_LIB} ${OPENSSL_LIB} ${OPENSSL_CRYPTO_LIB} Ws2_32.lib Wldap32.lib Crypt32.lib)

