file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(platform-basic-gaussian-acc
ethernet.cpp
display.cpp
${HEADERS})

target_include_directories(platform-basic-gaussian-acc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(platform-basic-gaussian-acc systemc)

add_executable(riscv-vp-gaussian-acc
        main.cpp)

target_link_libraries(riscv-vp-gaussian-acc rv32 platform-basic-gaussian-acc platform-common gdb-mc ${Boost_LIBRARIES} systemc pthread)

INSTALL(TARGETS riscv-vp-gaussian-acc RUNTIME DESTINATION bin)
