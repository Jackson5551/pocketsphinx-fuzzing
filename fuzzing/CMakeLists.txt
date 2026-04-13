# AFL++ Fuzzing Harnesses for PocketSphinx

# JSGF Grammar Fuzzer
add_executable(fuzz_jsgf fuzz_jsgf.c)
target_link_libraries(fuzz_jsgf pocketsphinx)
target_include_directories(fuzz_jsgf PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/include
)

# N-gram Language Model Fuzzer
add_executable(fuzz_ngram fuzz_ngram.c)
target_link_libraries(fuzz_ngram pocketsphinx)
target_include_directories(fuzz_ngram PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/include
)

# Audio File Parser Fuzzer
add_executable(fuzz_audio fuzz_audio.c)
target_link_libraries(fuzz_audio pocketsphinx)
target_include_directories(fuzz_audio PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/include
)

# Dictionary Parser Fuzzer
add_executable(fuzz_dict fuzz_dict.c)
target_link_libraries(fuzz_dict pocketsphinx)
target_include_directories(fuzz_dict PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/include
)

# FSG Parser Fuzzer
add_executable(fuzz_fsg fuzz_fsg.c)
target_link_libraries(fuzz_fsg pocketsphinx)
target_include_directories(fuzz_fsg PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/include
)

# Test crash reproducer
add_executable(test_crash test_crash.c)
target_link_libraries(test_crash pocketsphinx)
target_include_directories(test_crash PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/include
)

# JSGF crash tester (no AFL)
add_executable(test_jsgf_crash test_jsgf_crash.c)
target_link_libraries(test_jsgf_crash pocketsphinx)
target_include_directories(test_jsgf_crash PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/include
)
