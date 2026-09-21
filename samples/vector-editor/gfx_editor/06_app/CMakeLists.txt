
# Add source to this project's executable.
add_executable (06_app "06_app.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET 06_app PROPERTY CXX_STANDARD 20)
endif()

find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System)

target_link_libraries(06_app PRIVATE SFML::Graphics SFML::Window SFML::System)

# Автоматически копировать SFML .dll рядом с .exe при сборке (только Windows)
if (WIN32)
  add_custom_command(TARGET 06_app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      $<TARGET_RUNTIME_DLLS:06_app>
      $<TARGET_FILE_DIR:06_app>
    COMMAND_EXPAND_LISTS
  )
endif()
