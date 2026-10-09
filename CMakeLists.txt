# mdldec - GoldSrc model decompiler (MSVC-only)
# Based on DecompMDL by Toodles2You/halflife-tools (see LICENSE).
cmake_minimum_required(VERSION 3.20)

project(mdldec VERSION 1.0.0 LANGUAGES C)

# MSVC only. MinGW is explicitly not supported.
if(NOT MSVC)
  message(FATAL_ERROR "mdldec supports MSVC only. Configure with Visual Studio generator, e.g. cmake -G \"Visual Studio 17 2022\" -A x64")
endif()

add_executable(mdldec
  src/common.c
  src/math.c
  src/decompile.c
  src/studio.c
  src/model.c
  src/texture.c
  src/animation.c
  src/sprite.c
  src/wad.c
  src/info.c
)

target_include_directories(mdldec PRIVATE src)

target_compile_definitions(mdldec PRIVATE
  _CRT_SECURE_NO_WARNINGS
)

# No PCH auto-injection: each .c includes pch.h explicitly (MSVC + portable).
target_compile_options(mdldec PRIVATE
  /W3 /permissive-
)

target_link_libraries(mdldec PRIVATE winmm)

set_target_properties(mdldec PROPERTIES
  C_STANDARD 11
  OUTPUT_NAME mdldec
)

install(TARGETS mdldec DESTINATION .)
