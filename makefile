all: socketClientTest socketServerTest

socketServerTest: socketServerTest.o DatabaseHandler.o
	gcc socketServerTest.o DatabaseHandler.o -o socketServerTest

socketServerTest.o: socketServerTest.c DatabaseHandler.h
	gcc -c socketServerTest.c

DatabaseHandler.o: DatabaseHandler.c
	gcc -c DatabaseHandler.c 

socketClientTest: socketClientTest.c
	gcc socketClientTest.c -o socketClientTest
