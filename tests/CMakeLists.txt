add_executable(result_tests
        test_result.cpp
)

target_link_libraries(result_tests
        PRIVATE
        lsr::result
)

target_compile_features(result_tests
        PRIVATE
        cxx_std_17
)

if(RESULT_ENABLE_CLANG_TIDY)
    set_target_properties(result_tests PROPERTIES
            CXX_CLANG_TIDY "${CLANG_TIDY_EXE}"
    )
endif()

if(RESULT_ENABLE_SANITIZERS)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(result_tests
                PRIVATE
                -fsanitize=address,undefined
                -fno-omit-frame-pointer
        )

        target_link_options(result_tests
                PRIVATE
                -fsanitize=address,undefined
        )
    else()
        message(WARNING "RESULT_ENABLE_SANITIZERS is only configured for Clang/GCC")
    endif()
endif()

add_test(NAME result_tests
        COMMAND result_tests
)