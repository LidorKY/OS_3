all: stnc

stnc: stnc.o client.o server.o sender.o receiver.o
	gcc -Wall -g -o stnc stnc.o client.o server.o sender.o receiver.o -lssl -lcrypto

stnc.o: stnc.c sender.h receiver.h client.h server.h
	gcc -Wall -g -c stnc.c

client.o: client.c sender.h receiver.h client.h
	gcc -Wall -g -c client.c

server.o: server.c sender.h receiver.h server.h
	gcc -Wall -g -c server.c

sender.o: sender.c sender.h
	gcc -Wall -g -c sender.c

receiver.o: receiver.c receiver.h
	gcc -Wall -g -c receiver.c

clean:
	rm -f *.o stnc
