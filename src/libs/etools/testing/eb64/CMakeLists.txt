cmake_minimum_required(VERSION 2.8.1)




ETestAddTestSet(eb64 ON
    CASES  test_encode.c
           test_decode.c
    )

