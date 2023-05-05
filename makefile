all: stnc

stnc: stnc.o client.o server.o sender.o receiver.o
	gcc -Wall -g -o stnc stnc.o client.o server.o sender.o receiver.o -lssl -lcrypto

stnc.o: stnc.c
	gcc -Wall -g -c stnc.c

client.o: client.c
	gcc -Wall -g -c client.c

server.o: server.c
	gcc -Wall -g -c server.c

sender.o: sender.c
	gcc -Wall -g -c sender.c

receiver.o: receiver.c
	gcc -Wall -g -c receiver.c

clean:
	rm -f *.o stnc gotme.txt