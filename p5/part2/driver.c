#include <stdio.h>
#include <stdlib.h>
#include "fs.h"
#include "sniff.c"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <string.h>

int main(int argc, char* argv[]) {

	char a[] ="qqqq/eeee";
	ls(a,"wwww");
	return 0;
}
