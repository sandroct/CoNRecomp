cmake_minimum_required(VERSION 3.21)

project(CoNRecomp LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# runtime/: the PS2Recomp runtime (EE/IOP/VU/GS emulation), recompiler and offline tools, kept in this repository
# (runtime/README.md). thirdparty/: PCSX2's GS and what it needs (thirdparty/README.md).
# A prebuilt recompiler can be dropped in as tools/ps2_recomp.exe.
# Pass -DCON_BUILD_RECOMP=ON to build it from source instead.
option(CON_BUILD_RECOMP "Build ps2_recomp from runtime/ps2xRecomp" OFF)
# runtime/ps2xTest holds the offline tools: vu1_recomp (VU1 microcode AOT), gs_replay, vu0_audit.
option(CON_BUILD_TOOLS "Build the offline tools in runtime/ps2xTest (vu1_recomp...)" OFF)
# In-game ImGui "Runtime Debugger" (threads, kernel, IOP/SIF, RPC history, GS, DMA, memory); F1 toggles it.
option(CON_DEBUG_UI "Build the ImGui runtime debugger into con.exe" ON)
set(PS2X_ENABLE_DEBUG_UI ${CON_DEBUG_UI} CACHE BOOL "" FORCE)
# FFmpeg MPEG decoder for the libmpeg HLE (CON_MPEG_HLE=1 only; FMVs normally go through the emulated IPU).
# A prebuilt LGPL FFmpeg is downloaded at configure time.
option(CON_FFMPEG "Decode MPEG video with FFmpeg" ON)
set(PS2X_ENABLE_FFMPEG ${CON_FFMPEG} CACHE BOOL "" FORCE)
set(PS2X_ENABLE_AGRESSIVE_LOGS OFF CACHE BOOL "" FORCE)
set(PS2X_ENABLE_SCCACHE OFF CACHE BOOL "" FORCE)

add_subdirectory(runtime EXCLUDE_FROM_ALL)
add_subdirectory(src)

# PCSX2's GS as pcsx2gs.dll (docs/GS_PCSX2.md): built from thirdparty/, loaded by the port when [Video] Renderer=pcsx2.
option(CON_PCSX2_GS "Build PCSX2's GS (pcsx2gs.dll)" ON)
if(CON_PCSX2_GS AND WIN32)
    add_subdirectory(thirdparty/pcsx2-gs EXCLUDE_FROM_ALL)
    add_dependencies(con pcsx2gs)
    target_link_libraries(ps2_runtime PRIVATE pcsx2gs_api)
    target_link_libraries(con PRIVATE pcsx2gs_api)
endif()

# CON_LTO: whole-program optimization (/GL, linked with /LTCG) of the runtime, the IOP and the port sources. The
# generated code (~14k files) stays out: LTCG over it would blow up link time and memory.
option(CON_LTO "Whole-program optimization of the runtime, the IOP and the port (not the generated code)" OFF)
if(MSVC AND CON_LTO)
    foreach(target ps2_runtime ps2_iop con)
        target_compile_options(${target} PRIVATE $<$<CONFIG:RelWithDebInfo,Release>:/GL>)
    endforeach()
    set_property(TARGET ps2_runtime ps2_iop APPEND PROPERTY STATIC_LIBRARY_OPTIONS $<$<CONFIG:RelWithDebInfo,Release>:/LTCG>)
endif()
