cmake_minimum_required(VERSION 3.5)

add_library(libbhcd dataset.c params.c tree.c merge.c build.c dataset_gml.c tree_io.c sscache.c labelset.c dataset_gen.c islands.c)

if(WIN32)
    find_package(unofficial-glib REQUIRED)
    target_link_libraries(libbhcd INTERFACE unofficial::glib::glib)
elseif(UNIX)
    target_link_libraries(libbhcd INTERFACE ${GLIB_LDFLAGS})
endif()
    
add_executable(bhcd bhcd.c)





target_link_libraries(bhcd GSL::gsl GSL::gslcblas)

target_link_libraries(bhcd libbhcd)
target_link_libraries(bhcd hccd)
install(TARGETS bhcd RUNTIME DESTINATION bin)
