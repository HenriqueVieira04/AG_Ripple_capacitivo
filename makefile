all:
	g++ -o evolutives evolutives.cpp -lGL -lglut -lGLU

run:
	./evolutives

clean:
	rm -f evolutives