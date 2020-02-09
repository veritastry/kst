objects = main.o kiss_fft.o spectraldenoise.o denoise.o WavIO.o
denoise:$(objects)
	g++ -o denoise $(objects)
main.o:main.cpp
	g++ -c main.cpp -o main.o
spectraldenoise.o:spectraldenoise.cpp
	g++ -c spectraldenoise.cpp -o spectraldenoise.o
kiss_fft.o:kiss_fft.c
	g++ -c kiss_fft.c -o kiss_fft.o
denoise.o:denoise.cpp
	g++ -c denoise.cpp -o denoise.o
WavIO.o:WavIO.cpp
	g++ -c WavIO.cpp -o WavIO.o
.PHONY:clean
clean:
	rm denoise $(objects)