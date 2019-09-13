cmake_minimum_required(VERSION 3.5)
add_library(hccd util.c counts.c tokens.c bitset.c lnbetacache.c minheap.c)
target_link_libraries(hccd ${GSL_LIBRARIES})