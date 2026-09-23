#!/usr/bin/env bash
# 音楽理論基盤のホストユニットテスト（ハード不要）。
# picotofu の note/scale/chord 計算値と一致するかを検証する。
set -euo pipefail
cd "$(dirname "$0")/.."

TMP="$(mktemp -d)"

echo "== music_theory =="
g++ -std=c++14 -Wall -Wextra -o "$TMP/music_test" \
    test/test_music_theory.cpp \
    src/music/music_theory.cpp \
    src/music/music_state.cpp
"$TMP/music_test"

echo "== chord_engine =="
g++ -std=c++14 -Wall -Wextra -o "$TMP/chord_test" \
    test/test_chord_engine.cpp \
    src/music/chord_engine.cpp \
    src/music/music_theory.cpp \
    src/music/music_state.cpp
"$TMP/chord_test"
