all: main


main  : cache.cpp
	g++ cache.cpp -o main

clean:
	rm main
	
