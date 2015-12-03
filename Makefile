otherport.so: otherport.c
	gcc -Wall -nostartfiles -fpic -shared otherport.c -o otherport.so -ldl -D_GNU_SOURCE
