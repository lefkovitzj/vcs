cmake_minimum_required(VERSION 4.1)
project(vcs)

set(CMAKE_CXX_STANDARD 20)

add_executable(vcs main.cpp
        utils/io.h
        utils/io.cpp
        core/config.h
        core/config.cpp
        utils/hashing.h
        utils/hashing.cpp
        utils/compress.h
        utils/compress.cpp
        core/index.cpp
        core/index.h
        core/head.cpp
        core/head.h
        commands/init.cpp
        commands/init.h
        commands/init.cpp
        core/blob.cpp
        core/blob.h
        core/vcsno.cpp
        core/vcsno.h
        commands/status.cpp
        commands/status.h)