
# Add source to this project's executable.
add_executable (07_visitor "07_visitor.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET 07_visitor PROPERTY CXX_STANDARD 20)
endif()

find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System)

target_link_libraries(07_visitor PRIVATE SFML::Graphics SFML::Window SFML::System)

# Автоматически копировать SFML .dll рядом с .exe при сборке (только Windows)
if (WIN32)
  add_custom_command(TARGET 07_visitor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      $<TARGET_RUNTIME_DLLS:07_visitor>
      $<TARGET_FILE_DIR:07_visitor>
    COMMAND_EXPAND_LISTS
  )
endif()
