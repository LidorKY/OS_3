all: stnc

stnc: stnc.o client.o server.o
	gcc -Wall -g -o stnc stnc.o client.o server.o

stnc.o: stnc.c
	gcc -Wall -g -c stnc.c

client.o: client.c
	gcc -Wall -g -c client.c

server.o: server.c
	gcc -Wall -g -c server.c

clean:
	rm -f *.o stnc