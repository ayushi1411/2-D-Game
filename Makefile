all: angrybirds2D

#sample3D: Sample_GL3_3D.cpp glad.c
#	g++ -o sample3D Sample_GL3.cpp glad.c -lGL -lglfw

sample2D: angrybirds.cpp glad.c
	g++ -o angrybirds2D angrybirds.cpp glad.c -lGL -lglfw -ldl

clean:
	rm angrybirds2D
