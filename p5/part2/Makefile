all:
	gcc -o myLib myLib.c -Wall
	gcc -c -fpic myLib.c -Wall -Werror
	gcc -shared -o lfsreader myLib.o
	#gcc -lmyLib -L. -o srg driver.c -Wall -Werror

clean:
	rm -f myLib
