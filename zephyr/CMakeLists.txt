cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(em_intro)

# map .sdram_bss section to SDRAM2 memory region
zephyr_linker_sources(SECTIONS sdram.ld)

target_sources(app PRIVATE
    src/main.c
    src/graph.c
    src/font.c
    src/sprite.c
    src/fire.c
    src/floor.c
    src/stars.c
    src/logo.c
    src/scroller.c
)
