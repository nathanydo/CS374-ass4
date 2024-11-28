CC = gcc --std=gnu99
exe_file = main

$(exe_file): main.c
	$(CC) main.c -o $(exe_file)
	
clean:
	rm -f *.out *.o $(exe_file)