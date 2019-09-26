cmake_minimum_required(VERSION 3.5)

add_executable(test_bitset_hash test_bitset_hash.c)
target_link_libraries(test_bitset_hash libbhcd hccd)

add_executable(test_util test_util.c)
target_link_libraries(test_util libbhcd hccd)

add_executable(test_pipeio test_pipeio.c)
target_link_libraries(test_pipeio libbhcd hccd)

add_test(NAME bitset_hash COMMAND test_bitset_hash)
add_test(NAME util COMMAND test_util)
add_test(NAME pipeio COMMAND test_pipeio)
