CC = gcc --std=gnu99
exe_file = line_processor

$(exe_file): main.c
	$(CC) main.c -o $(exe_file)
	
clean:
	rm -f *.out *.o $(exe_file)