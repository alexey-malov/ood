
# Add source to this project's executable.
add_executable (02_draw "02_draw.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET 02_draw PROPERTY CXX_STANDARD 20)
endif()

find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System)

target_link_libraries(02_draw PRIVATE SFML::Graphics SFML::Window SFML::System)

# Автоматически копировать SFML .dll рядом с .exe при сборке (только Windows)
if (WIN32)
  add_custom_command(TARGET 02_draw POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      $<TARGET_RUNTIME_DLLS:02_draw>
      $<TARGET_FILE_DIR:02_draw>
    COMMAND_EXPAND_LISTS
  )
endif()
