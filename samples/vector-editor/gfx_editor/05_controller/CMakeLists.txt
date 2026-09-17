
# Add source to this project's executable.
add_executable (05_controller "05_controller.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET 05_controller PROPERTY CXX_STANDARD 20)
endif()

find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System)

target_link_libraries(05_controller PRIVATE SFML::Graphics SFML::Window SFML::System)

# Автоматически копировать SFML .dll рядом с .exe при сборке (только Windows)
if (WIN32)
  add_custom_command(TARGET 05_controller POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      $<TARGET_RUNTIME_DLLS:05_controller>
      $<TARGET_FILE_DIR:05_controller>
    COMMAND_EXPAND_LISTS
  )
endif()
