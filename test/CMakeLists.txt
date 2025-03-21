set(TEST_LIST
    multithread
    quad
)

if (APPLE OR OPENGL_ES)
    list(APPEND TEST_LIST dx11)
endif ()

foreach (sample ${TEST_LIST})
    file(GLOB_RECURSE TEST_SOURCES
         ${CMAKE_CURRENT_SOURCE_DIR}/${sample}/*.cpp
         ${CMAKE_CURRENT_SOURCE_DIR}/util/*.cpp
         ${CMAKE_CURRENT_SOURCE_DIR}/util/*.h
    )

    add_executable(
        test_${sample}
        ${TEST_SOURCES}
    )

    target_link_libraries(
        test_${sample}
        PRIVATE
        ${PROJECT_NAME}
    )

    target_compile_features(
        test_${sample}
        PRIVATE
        cxx_std_23
    )

    target_compile_definitions(
        test_${sample}
        PRIVATE
        ASSETS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets/"
    )

    target_include_directories(
        test_${sample}
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/util/
        ${CMAKE_CURRENT_SOURCE_DIR}/fork/
    )

    add_custom_command(
        TARGET
        test_${sample}
        POST_BUILD
        COMMAND
        ${CMAKE_COMMAND} -E copy_if_different ${DLLS} $<TARGET_FILE_DIR:test_${sample}>
    )
endforeach ()