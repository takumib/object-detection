All:cosine window object-detection invert bilateral-filter main cosine

cosine:
	gcc src/cosinesimilarity.c -Wall -c -o ./bin/cosinesimilarity.o  `pkg-config --cflags --libs opencv`
invert:
	gcc src/invert.c -Wall -c -o ./bin/invert.o `pkg-config --cflags --libs opencv`
window:
	gcc src/window.c -Wall -c -o ./bin/window.o `pkg-config --cflags --libs opencv`		
object-detection:
	gcc src/object-detection.c  -Wall -c -o bin/object-detection.o `pkg-config --cflags --libs opencv`
bilateral-filter:
	gcc src/working-bilateral-filter.c  -Wall -c -o bin/bilateral-filter.o `pkg-config --cflags --libs opencv`
main:
	gcc bin/object-detection.o ./bin/cosinesimilarity.o ./bin/window.o -o ./bin/out `pkg-config --cflags --libs opencv`
test: 
	make clean
	make
	./bin/out ./images/lenna.png ./images/Lenna_Query.png 
clean: 
	rm -rf ./bin
	mkdir ./bin
