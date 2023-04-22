set(TARGET_NAME online_http)
find_package(CURL)

NewTargetSource()
AddSourceFolder(INCLUDE PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/public")
AddSourceFolder("${CMAKE_CURRENT_SOURCE_DIR}/private")

if(CURL_FOUND)
    AddSourceFolder(INCLUDE PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/public/curl")
    AddSourceFolder("${CMAKE_CURRENT_SOURCE_DIR}/private/curl")
endif()

source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${SourceFiles})
add_library(${TARGET_NAME} SHARED ${SourceFiles})

target_compile_features(${TARGET_NAME} PRIVATE cxx_std_20)
set_target_properties(${TARGET_NAME} PROPERTIES CXX_STANDARD_REQUIRED ON)

AddTargetInclude(${TARGET_NAME})

target_link_libraries(${TARGET_NAME} PUBLIC sutils)

if(CURL_FOUND)
    target_link_libraries(${TARGET_NAME} PUBLIC CURL::libcurl)
endif()
