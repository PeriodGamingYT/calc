make:
	make clean
	gcc main.c -o main
	./main

debug:
	make clean
	gcc main.c -g -o main
	valgrind ./main

clean:
	clear
	rm -f main
