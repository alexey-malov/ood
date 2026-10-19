# Add source to this project's executable.
add_executable (08_undo "08_undo.cpp" "domain_model.h" "command_history.h" "editor_model.h"  "canvas_view.h" "editor_window.h" "drawing_document.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET 08_undo PROPERTY CXX_STANDARD 20)
endif()

find_package(Boost REQUIRED)
find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System)

target_link_libraries(08_undo PRIVATE SFML::Graphics SFML::Window SFML::System Boost::headers)

# Автоматически копировать SFML .dll рядом с .exe при сборке (только Windows)
if (WIN32)
  add_custom_command(TARGET 08_undo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      $<TARGET_RUNTIME_DLLS:08_undo>
      $<TARGET_FILE_DIR:08_undo>
    COMMAND_EXPAND_LISTS
  )
endif()
