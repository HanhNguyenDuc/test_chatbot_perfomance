cmake_minimum_required(VERSION 3.19.2)
project(AIF3_CLI CXX)
file(GLOB SOURCES "src/*.cpp")
add_executable(aif3_client ${SOURCES})
include(FetchContent)

FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/whoshuu/cpr.git GIT_TAG c8d33915dbd88ad6c92b258869b03aba06587ff9) # the commit hash for 1.5.0
FetchContent_MakeAvailable(cpr)

FetchContent_Declare(json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.7.3)

FetchContent_GetProperties(json)
if(NOT json_POPULATED)
  FetchContent_Populate(json)
  add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

target_link_libraries(aif3_client PRIVATE cpr::cpr nlohmann_json::nlohmann_json)

target_include_directories(aif3_client PUBLIC include)
