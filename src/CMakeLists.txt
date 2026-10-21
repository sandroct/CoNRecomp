# CON_ARCH: instruction set the compiler may use in the generated code (game code + VU1/VU0 AOT, the bulk of
# the build). SSE41 (default): the PCSX2 minimum; one con.exe for every CPU. AVX2 only for comparison.
set(CON_ARCH "SSE41" CACHE STRING "Instruction set of the generated code: SSE41 (default) or AVX2 (comparison only)")
set_property(CACHE CON_ARCH PROPERTY STRINGS SSE41 AVX2)
if(CON_ARCH STREQUAL "AVX2")
    set(CON_ARCH_FLAG /arch:AVX2)
else()
    set(CON_ARCH_FLAG "")
endif()

set(CON_GENERATED_DIR "${CMAKE_SOURCE_DIR}/generated")
set(CON_RUNTIME_INCLUDE "${CMAKE_SOURCE_DIR}/runtime/ps2xRuntime/include")

if(NOT EXISTS "${CON_GENERATED_DIR}/register_functions.cpp")
    message(FATAL_ERROR
        "Recompiled game code not found in ${CON_GENERATED_DIR}.\n"
        "Put your own disc image in game/ and run: python build.py generate")
endif()

file(GLOB CON_GENERATED_SOURCES CONFIGURE_DEPENDS "${CON_GENERATED_DIR}/*.cpp")

set(CON_HOST_SOURCES
    main.cpp
    port/log.cpp
    port/sha256.cpp
)

# Reuse an already-built game code library when only host sources change (the full EE build is long).
set(CON_GAME_CODE_LIBRARY "" CACHE FILEPATH "Reuse an unchanged generated EE library for host-only builds")
if(CON_GAME_CODE_LIBRARY)
    if(NOT EXISTS "${CON_GAME_CODE_LIBRARY}")
        message(FATAL_ERROR "Cached generated EE library does not exist: ${CON_GAME_CODE_LIBRARY}")
    endif()
    add_library(con_game_code STATIC IMPORTED)
    set_target_properties(con_game_code PROPERTIES IMPORTED_LOCATION "${CON_GAME_CODE_LIBRARY}")
    target_include_directories(con_game_code INTERFACE "${CON_GENERATED_DIR}")
    target_link_libraries(con_game_code INTERFACE ps2_runtime)
    get_filename_component(CON_GAME_CODE_LIBRARY_DIR "${CON_GAME_CODE_LIBRARY}" DIRECTORY)
    target_link_libraries(con_game_code INTERFACE "${CON_GAME_CODE_LIBRARY_DIR}/con_game_code_huge.lib")
else()
    # MSVC skips /d2OptimizeHugeFunctions' full optimization on huge functions; the biggest ones would run
    # unoptimized. Files above the threshold go into con_game_code_huge, which keeps MSVC's default
    # huge-function handling, and the known-good ones get full optimization by name below.
    set(CON_HUGE_SOURCE_BYTES 900000 CACHE STRING "Generated files larger than this skip /d2OptimizeHugeFunctions")
    set(CON_HUGE_SOURCES "")
    foreach(source IN LISTS CON_GENERATED_SOURCES)
        file(SIZE "${source}" sourceBytes)
        if(sourceBytes GREATER CON_HUGE_SOURCE_BYTES AND NOT source MATCHES "register_functions\.cpp$")
            list(APPEND CON_HUGE_SOURCES "${source}")
        endif()
    endforeach()
    list(REMOVE_ITEM CON_GENERATED_SOURCES ${CON_HUGE_SOURCES})

    add_library(con_game_code_huge STATIC ${CON_HUGE_SOURCES})
    target_include_directories(con_game_code_huge PUBLIC "${CON_GENERATED_DIR}")
    target_link_libraries(con_game_code_huge PUBLIC ps2_runtime)
    # Each huge file needs several GB in MSVC's pass 2; compiling them all at once runs out of heap (C1002).
    set(CON_HUGE_JOBS 2 CACHE STRING "Concurrent compiles of the huge generated sources (Ninja)")
    set_property(GLOBAL APPEND PROPERTY JOB_POOLS con_huge=${CON_HUGE_JOBS})
    set_property(TARGET con_game_code_huge PROPERTY JOB_POOL_COMPILE con_huge)
    if(MSVC)
        target_compile_options(con_game_code_huge PRIVATE /bigobj /fp:precise ${CON_ARCH_FLAG} /GS- /W0)
        # The huge files that do compile in reasonable time get full optimization back; the worst ones do not.
        set(CON_HUGE_OPTIMIZED "" CACHE STRING "Huge generated files still compiled with /d2OptimizeHugeFunctions")
        foreach(source IN LISTS CON_HUGE_SOURCES)
            get_filename_component(sourceName "${source}" NAME)
            if(sourceName IN_LIST CON_HUGE_OPTIMIZED)
                set_source_files_properties("${source}" PROPERTIES COMPILE_OPTIONS "/d2OptimizeHugeFunctions")
            endif()
        endforeach()
    endif()

    add_library(con_game_code STATIC ${CON_GENERATED_SOURCES})
    target_link_libraries(con_game_code PUBLIC con_game_code_huge)
    target_include_directories(con_game_code PUBLIC "${CON_GENERATED_DIR}")
    target_link_libraries(con_game_code PUBLIC ps2_runtime)
    set_target_properties(con_game_code PROPERTIES UNITY_BUILD OFF)
    target_precompile_headers(con_game_code PRIVATE
        "${CON_RUNTIME_INCLUDE}/ps2_runtime_macros.h"
        "${CON_RUNTIME_INCLUDE}/ps2_runtime.h"
        "${CON_GENERATED_DIR}/ps2_recompiled_functions.h"
        "${CON_GENERATED_DIR}/ps2_recompiled_stubs.h"
        "${CON_RUNTIME_INCLUDE}/ps2_syscalls.h"
        "${CON_RUNTIME_INCLUDE}/ps2_stubs.h"
    )
    if(MSVC)
        target_compile_options(con_game_code PRIVATE /bigobj /fp:precise ${CON_ARCH_FLAG} /GS- /d2OptimizeHugeFunctions /W0 /MP $<$<CONFIG:RelWithDebInfo>:/Ob2>)
        # The one ~48 MB table initializer that runs once at startup: compiles in seconds without
        # PCH/optimizer, hours with them.
        set_source_files_properties("${CON_GENERATED_DIR}/register_functions.cpp" PROPERTIES
            COMPILE_OPTIONS "/Od" SKIP_PRECOMPILE_HEADERS ON)
    endif()
endif()

add_executable(con ${CON_HOST_SOURCES})
target_include_directories(con PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${CON_GENERATED_DIR}")
target_link_libraries(con PRIVATE con_game_code ps2_runtime)

if(MSVC)
    target_compile_options(con PRIVATE /fp:precise /W3 /utf-8)
    target_compile_definitions(con PRIVATE _CRT_SECURE_NO_WARNINGS CON_ARCH_NAME="${CON_ARCH}")
    # /FORCE:MULTIPLE: WinAPI vs raylib CloseWindow clash. Large stack: deep guest call chains run on the
    # host stack.
    target_link_options(con PRIVATE /FORCE:MULTIPLE /STACK:16777216 $<$<CONFIG:RelWithDebInfo>:/LTCG>)
    target_link_libraries(con PRIVATE dbghelp winmm)
endif()

if(CON_DEBUG_UI AND TARGET rlImGui)
    target_sources(con PRIVATE "${CMAKE_SOURCE_DIR}/runtime/ps2xRuntime/src/lib/ps2_debug_panel.cpp")
    target_include_directories(con PRIVATE "${CMAKE_SOURCE_DIR}/runtime/ps2xRuntime/src/lib")
    target_link_libraries(con PRIVATE rlImGui imgui)
    target_compile_definitions(con PRIVATE PS2X_ENABLE_DEBUG_UI=1)
endif()

if(COMMAND ps2x_stage_ffmpeg_runtime_dlls)
    ps2x_stage_ffmpeg_runtime_dlls(con)
endif()

# VU1 microcode compiled ahead of time: capture the code images with PS2X_VU1_CAPTURE, then run vu1_recomp
# (configure with -DCON_BUILD_TOOLS=ON) into generated/vu1. Images not compiled stay interpreted.
file(GLOB CON_VU1_SOURCES CONFIGURE_DEPENDS "${CON_GENERATED_DIR}/vu1/*.cpp")
if(CON_VU1_SOURCES)
    add_library(con_vu1_code STATIC ${CON_VU1_SOURCES})
    target_include_directories(con_vu1_code PRIVATE "${CMAKE_SOURCE_DIR}/runtime/ps2xRuntime/src/lib/vu")
    target_link_libraries(con_vu1_code PUBLIC ps2_runtime)
    target_link_libraries(con PRIVATE con_vu1_code)
    target_compile_definitions(con PRIVATE CON_HAS_VU1_PROGRAMS=1)
    if(MSVC)
        target_compile_options(con_vu1_code PRIVATE /bigobj /fp:precise ${CON_ARCH_FLAG} /W0 /MP /d2OptimizeHugeFunctions $<$<CONFIG:RelWithDebInfo>:/Ob2>)
    endif()
endif()

set_target_properties(con PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
