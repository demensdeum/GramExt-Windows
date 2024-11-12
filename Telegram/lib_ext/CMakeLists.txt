add_library(lib_ext OBJECT)
add_library(desktop-app::lib_ext ALIAS lib_ext)
init_target(lib_ext)

get_filename_component(src_loc . REALPATH)

set_target_properties(lib_ext PROPERTIES AUTOMOC ON)

nice_target_sources(lib_ext ${src_loc}
PRIVATE
    ext/Controller/js_list.h
    ext/Controller/Controller.h
    ext/Controller/Controller.cpp
    ext/Extension/Extension.h
    ext/Extension/Extension.cpp
    ext/tiny-js/TinyJS_Functions.h
    ext/tiny-js/TinyJS-Functions.cpp
    ext/tiny-js/TinyJS_MathFunctions.h
    ext/tiny-js/TinyJS-MathFunctions.cpp
    ext/tiny-js/TinyJS.h
    ext/tiny-js/TinyJS.cpp
    ext/UI/ExtensionsListWindow.h
    ext/UI/ExtensionsListWindow.cpp
)

include_directories(
    C:/tdesktop/Telegram/lib_ext/ThirdParty/curl/include
    C:/tdesktop/Telegram/lib_ext/ThirdParty/json/include
)

target_include_directories(lib_ext
PUBLIC
    ${src_loc}
)

target_link_libraries(lib_ext
PUBLIC
    C:\\Users\\Demensdeum\\Documents\\curl-8.11.0_1-win64-mingw\\lib\\libcurl.a
    C:\\Users\\Demensdeum\\Documents\\curl-8.11.0_1-win64-mingw\\lib\\libcurl.dll.a
    desktop-app::lib_base
    desktop-app::lib_ext
    desktop-app::external_qt_static_plugins
    desktop-app::external_qt
    desktop-app::external_qr_code_generator
)
