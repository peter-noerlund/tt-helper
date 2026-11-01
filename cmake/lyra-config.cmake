if(NOT lyra_FOUND)
    set(lyra_FOUND FALSE)

    include(FetchContent)

    FetchContent_Declare(
        lyra
        GIT_REPOSITORY https://github.com/bfgroup/Lyra.git
        GIT_TAG        1.6.1
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(lyra)
    add_library(lyra::lyra INTERFACE IMPORTED)
    target_include_directories(lyra::lyra INTERFACE ${lyra_SOURCE_DIR}/include)

    set(lyra_FOUND TRUE)
endif()
