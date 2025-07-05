#!/usr/bin/env bash

wFlags="-Wall -Wextra -Werror -Wshadow -pedantic"
CC="gcc"

SRC="${pwd}test_*.c"

echo =========================

echo ACTIVATED FLAGS: $wFlags
echo SOURCE FILES: $SRC

echo =========================

$CC -o test $SRC
