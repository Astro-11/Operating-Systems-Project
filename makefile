all: clientTest server tuiPrototype

server: server.o DatabaseHandler.o SocketUtilities.o
	gcc server.o DatabaseHandler.o SocketUtilities.o -o server

server.o: server.c DatabaseHandler.h SocketUtilities.h
	gcc -c server.c

DatabaseHandler.o: DatabaseHandler.c DatabaseHandler.h
	gcc -c DatabaseHandler.c 

SocketUtilities.o: SocketUtilities.c SocketUtilities.h DatabaseHandler.h
	gcc -c SocketUtilities.c 

clientTest: clientTest.o SocketUtilities.o DatabaseHandler.o
	gcc clientTest.c SocketUtilities.o DatabaseHandler.o -o clientTest

clientTest.o: clientTest.c SocketUtilities.h
	gcc -c clientTest.c

tuiPrototype: tuiPrototype.c SocketUtilities.o DatabaseHandler.o
	gcc -o tuiPrototype tuiPrototype.c -lform -lmenu -lncurses SocketUtilities.o DatabaseHandler.o

clean:
	rm *.o
