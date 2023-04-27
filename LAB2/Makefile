C = gcc
CFLAGS = -W -Wall -Wno-unused-parameter -Wno-unused-variable -std=c11 -pedantic -o

parent:
	$(C) parent.c $(CFLAGS) parent

child:
	$(C) child.c $(CFLAGS) child


all:
	export CHILD_PATH=/home/max/CLionProjects/LAB2OSiSPFinal/child
	export LC_COLLATE=C
	$(C) parent.c $(CFLAGS) parent
	$(C) child.c $(CFLAGS) child

cleanAll:
	rm -rf *.o parent
	rm -rf *.o child

cleanParent:
	rm -rf *.o parent

cleanChild:
	rm -rf *.o child