# Windows Makefile for Embedded Sensor Hub
.PHONY: all clean build run test debug release help

all: build

build:
	@echo Building embedded sensor hub for Windows...
	@if not exist build mkdir build
	@cd build && cmake -G "MinGW Makefiles" .. && mingw32-make

debug:
	@echo Building with debug symbols...
	@if not exist build mkdir build
	@cd build && cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug .. && mingw32-make

release:
	@echo Building optimized release...
	@if not exist build mkdir build
	@cd build && cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release .. && mingw32-make

run: build
	@echo.
	@echo Running simulation...
	@echo.
	@build\sensor_hub.exe

test: build
	@echo.
	@echo Running tests...
	@echo.
	@build\sensor_hub_test.exe

clean:
	@echo Cleaning build files...
	@if exist build rmdir /s /q build
	@if exist *.exe del /q *.exe

quick:
	@echo Quick build (direct compilation)...
	@gcc -Os main.c src/kernel/*.c src/hal/*.c src/algorithms/*.c src/app/*.c src/utils/*.c simulator/*.c -o sensor_hub_quick.exe -I. -Isrc -Isimulator

help:
	@echo.
	@echo Available commands:
	@echo   make build     - Build the project
	@echo   make run       - Build and run simulation
	@echo   make test      - Build and run tests
	@echo   make clean     - Remove build files
	@echo   make quick     - Quick direct compilation
	@echo   make help      - Show this help
	@echo.