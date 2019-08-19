#!/bin/bash

g++ main.cpp chip8.h chip8.cpp -o ../c8 $(pkg-config --cflags --libs sdl2)  