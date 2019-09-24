cmake_minimum_required(VERSION 3.5)

add_executable(test_bitset_hash test_bitset_hash.c)
target_link_libraries(test_bitset_hash libbhcd hccd)

add_test(NAME bitset_hash COMMAND test_bitset_hash)