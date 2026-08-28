cmake_minimum_required(VERSION 4.3)
project(Wev C)

set(CMAKE_C_STANDARD 11)

add_executable(Wev src/main.c
        src/driver/driver.h
        src/driver/driver.c
        src/driver/driver.c
        src/driver/driver.h
        src/driver/console.c
        src/driver/console.h
        src/core/string.c
        src/core/string.h
        src/core/file.c
        src/core/file.h
        src/driver/driver_args.c
        src/driver/driver_args.h)
