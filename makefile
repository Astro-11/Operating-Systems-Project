all: socketClientTest socketServerTest

socketServerTest: socketServerTest.o DatabaseHandler.o SocketUtilities.o
	gcc socketServerTest.o DatabaseHandler.o SocketUtilities.o -o socketServerTest

socketServerTest.o: socketServerTest.c DatabaseHandler.h SocketUtilities.h
	gcc -c socketServerTest.c

DatabaseHandler.o: DatabaseHandler.c DatabaseHandler.h
	gcc -c DatabaseHandler.c 

SocketUtilities.o: SocketUtilities.c SocketUtilities.h DatabaseHandler.h
	gcc -c SocketUtilities.c 

socketClientTest: socketClientTest.o SocketUtilities.o
	gcc socketClientTest.c SocketUtilities.o -o socketClientTest

socketClientTest.o: socketClientTest.c SocketUtilities.h
	gcc -c socketClientTest.c

clean:
	rm *.o
