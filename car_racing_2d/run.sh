#!/usr/bin/env bash
set -e
cd "$(dirname "$0")"
g++ -std=c++17 -O2 main.cpp -o car_racing_2d -lglfw -lGL -lopenal -ldl -pthread
exec ./car_racing_2d
