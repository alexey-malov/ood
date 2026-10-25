find_package(Boost REQUIRED)
find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System)

add_executable(10_grouping
    "10_grouping.cpp"
    "domain_model.h"
    "command_history.h"
    "editor_model.h"
    "canvas_view.h"
    "editor_window.h"
    "drawing_document.h"
    "group_shape.h"
    "shape_collection.h"
    "ishape.h"
    "types.h"
    "signals.h"
    "ishapevisitor.h")

set_target_properties(10_grouping PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_link_libraries(10_grouping PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
    Boost::headers
)

# Автоматически копировать SFML DLL рядом с EXE (Windows)
if(WIN32)
    add_custom_command(TARGET 10_grouping POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:10_grouping>
            $<TARGET_FILE_DIR:10_grouping>
        COMMAND_EXPAND_LISTS
        VERBATIM
    )
endif()