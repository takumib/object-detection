all: object-detection

object-detection:
	gcc src/object-detection.c  -o bin/object-detection `pkg-config --cflags --libs opencv`

test:
	make clean
	make 
	./bin/object-detection images/lenna.png

clean:
	rm -rf ./bin
	mkdir bin
