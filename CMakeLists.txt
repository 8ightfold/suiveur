cmake_minimum_required(VERSION 3.23)

set(enable_tracking OFF CACHE BOOL "Tracking disabled")
set(disable_ansi OFF CACHE BOOL "ANSI enabled")

add_library(suiveur_includes STATIC
        include/suiveur/detail/allocation_registry.cpp
        include/suiveur/detail/load_file.cpp
        include/suiveur/detail/pad_with.cpp
        include/suiveur/detail/partition_data.cpp
        include/suiveur/detail/type_names.cpp)
target_include_directories(suiveur_includes PUBLIC include)

if(${enable_tracking})
    target_compile_definitions(suiveur_includes PUBLIC ENABLE_MEMORY_REGISTRY=)
endif()

if(${disable_ansi})
    target_compile_definitions(suiveur_includes PUBLIC DISABLE_ANSI_COLOR=)
endif()

add_library(suiveur::suiveur ALIAS suiveur_includes)