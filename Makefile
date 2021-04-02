CC = gcc
CFLAG = -g

oss: oss.o
	$(CC) $(CFLAG) $< -o $@

clean:
	rm -f *.o oss