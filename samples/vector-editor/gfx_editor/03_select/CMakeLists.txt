
# Add source to this project's executable.
add_executable (03_select "03_select.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET 03_select PROPERTY CXX_STANDARD 20)
endif()

find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System)

target_link_libraries(03_select PRIVATE SFML::Graphics SFML::Window SFML::System)

# Автоматически копировать SFML .dll рядом с .exe при сборке (только Windows)
if (WIN32)
  add_custom_command(TARGET 03_select POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      $<TARGET_RUNTIME_DLLS:03_select>
      $<TARGET_FILE_DIR:03_select>
    COMMAND_EXPAND_LISTS
  )
endif()
