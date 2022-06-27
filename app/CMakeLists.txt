cmake_minimum_required(VERSION 3.0)


add_executable(disneyapp
    DisneyWindow.cpp
    Font.cpp
    Image.cpp
    main.cpp
    Rectangle.cpp
    Texture.cpp
    WebServices.cpp
    WebSupplicant.cpp
    Window.cpp
    WindowServices.cpp)

target_include_directories(disneyapp PRIVATE
    curl/include
    ${STB_INCLUDE_DIRS})

target_link_libraries(disneyapp PRIVATE
    CURL::libcurl
    glad::glad
    glfw
    nlohmann_json::nlohmann_json
    opengl32)


add_custom_command(TARGET disneyapp
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/DisneyPlusLogo.png" $<TARGET_FILE_DIR:disneyapp>/)
