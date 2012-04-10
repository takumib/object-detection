all: object-detection

object-detection:
	gcc src/object-detection.c  -o bin/object-detection `pkg-config --cflags --libs opencv`


clean:
	rm -rf ./bin
	mkdir bin
