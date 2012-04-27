all: object-detection

object-detection:
	gcc src/object-detection.c src/matrix.c src/steeringKernel.c src/window.c src/cosinesimilarity.c -o bin/object-detection `pkg-config --cflags --libs opencv`

test:
	make clean
	make 
	./bin/object-detection images/eye.png images/lenna.png

clean:
	rm -rf ./bin
	mkdir bin
