SHELL := bash
all: compile

compile:
	@echo "Compiling the source code src.c"
	mpicc  -o code src.c

