all: sample2D

sample2D: 3d.cpp glad.c
	g++ -o sample2D 3d.cpp glad.c -lGL -lglfw -ldl

clean:
	rm sample2D
