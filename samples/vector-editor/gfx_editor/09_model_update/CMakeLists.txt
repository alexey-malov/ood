find_package(Boost REQUIRED)
find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System)

add_executable(09_model_update
    "09_model_update.cpp"
    "domain_model.h"
    "command_history.h"
    "editor_model.h"
    "canvas_view.h"
    "editor_window.h"
    "drawing_document.h"
)

set_target_properties(09_model_update PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_link_libraries(09_model_update PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
    Boost::headers
)

# Автоматически копировать SFML DLL рядом с EXE (Windows)
if(WIN32)
    add_custom_command(TARGET 09_model_update POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:09_model_update>
            $<TARGET_FILE_DIR:09_model_update>
        COMMAND_EXPAND_LISTS
        VERBATIM
    )
endif()