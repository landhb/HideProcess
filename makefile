32bit: main.c tools/errorhandling.c tools/process.c tools/privilages.c 
	i686-w64-mingw32-gcc -Iddk -o dkom.exe main.c tools/errorhandling.c tools/process.c tools/privilages.c main.h

64bit: main.c tools/errorhandling.c tools/process.c tools/privilages.c 
	x86_64-w64-mingw32-gcc -Iddk -o dkom.exe main.c tools/errorhandling.c tools/process.c tools/privilages.c main.h