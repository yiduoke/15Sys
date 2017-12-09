all: control main

control: control.c
	gcc -o control control.c

main: main.c
	gcc -o main main.c

clean:
	rm *~
	rm a.out