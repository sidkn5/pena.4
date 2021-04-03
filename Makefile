CC = gcc
CFLAG = -g -lm
 
all: oss user

%.o: %.c
	$(CC) $(CFLAG) -c $< -o $@

oss: oss.o
	$(CC) $(CFLAG) $< -o $@ -lm

user: user.o
	$(CC) $(CFLAG) $< -o $@ -lm

clean:
	rm -f *.o oss user *log *.txt
