all: fiber

fiber: fiber.o fiber_init_linux.o main.o
	g++ -fno-stack-protector -o $@ fiber.o fiber_init_linux.o main.o

%.o:%.cpp
	g++ -O -fno-stack-protector -c $< -o $@
