add_subdirectory(proxyParser)
add_subdirectory(httpClient)

add_executable(reger main.cpp)
target_link_libraries(reger PUBLIC ${CONAN_LIBS} proxyParser httpClient)

