All:cosine window object-detection

object-detection:
	gcc src/object-detection.c -o bin/object-detection `pkg-config --cflags --libs opencv`
cosine:
	gcc src/cosinesimilarity.c -Wall -c -o ./bin/cosinesimilarity.o  `pkg-config --cflags --libs opencv`
invert:
	gcc src/invert.c -Wall -c -o bin/invert.o `pkg-config --cflags --libs opencv`
window:
	gcc src/window.c -Wall -c -o ./bin/window.o `pkg-config --cflags --libs opencv`		

object-detection:
	gcc src/object-detection.c  -Wall -c -o bin/object-detion.o `pkg-config --cflags --libs opencv`

bilateral-filter:
	gcc src/bilateral-filter.c  -Wall -c -o bin/bilateral-filter.o `pkg-config --cflags --libs opencv`

#cosinesimilarity.c  cosinesimilarity.h	invert.c  object-detection.c  working-bilateral-filter.c

clean: 
	rm -rf ./bin
	mkdir ./bin
