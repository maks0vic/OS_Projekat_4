all: domaci4.c
	gcc domaci4.c -o domaci4 -pthread

clean:
	rm -f domaci4