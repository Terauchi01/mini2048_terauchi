cmake_minimum_required(VERSION 3.10)
project(ntn_learn)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

# zlibを見つける
find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})

include_directories(head)

# 実行ファイルの生成
add_executable(learning_program
    learning/main.cpp
)

# zlibをリンク
target_link_libraries(learning_program ${ZLIB_LIBRARIES})

# N-tupleの定義に応じたコンパイルオプション
target_compile_definitions(learning_program PRIVATE T4)  # デフォルトで4-tupleを使用