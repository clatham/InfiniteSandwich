cmake_minimum_required(VERSION 3.0)


add_executable(disneyapp
    DisneyWindow.cpp
    Font.cpp
    Image.cpp
    main.cpp
    Rectangle.cpp
    Texture.cpp
    VideoDecoder.cpp
    WebServices.cpp
    WebSupplicant.cpp
    Window.cpp
    WindowServices.cpp)

target_include_directories(disneyapp PRIVATE
    curl/include
    ${STB_INCLUDE_DIRS}
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include)

target_link_directories(disneyapp PRIVATE
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib)

target_link_libraries(disneyapp PRIVATE
    ${CMAKE_STATIC_LIBRARY_PREFIX}avcodec${CMAKE_STATIC_LIBRARY_SUFFIX}
    ${CMAKE_STATIC_LIBRARY_PREFIX}avformat${CMAKE_STATIC_LIBRARY_SUFFIX}
    ${CMAKE_STATIC_LIBRARY_PREFIX}avutil${CMAKE_STATIC_LIBRARY_SUFFIX}
    ${CMAKE_STATIC_LIBRARY_PREFIX}swscale${CMAKE_STATIC_LIBRARY_SUFFIX}
    CURL::libcurl
    glad::glad
    glfw
    nlohmann_json::nlohmann_json
    opengl32)

if(WIN32)
    target_link_options(disneyapp PRIVATE
        "/subsystem:windows"
        "/entry:mainCRTStartup")
endif()


add_custom_command(TARGET disneyapp
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/DisneyPlusLogo.png" $<TARGET_FILE_DIR:disneyapp>/)
