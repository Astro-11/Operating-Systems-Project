all: clientTest server tui

server: server.o DatabaseHandler.o SocketUtilities.o
	gcc server.o DatabaseHandler.o SocketUtilities.o -o server

server.o: server.c DatabaseHandler.h SocketUtilities.h
	gcc -c server.c

clientTest: clientTest.o clientFunctions.o
	gcc clientTest.c clientFunctions.o -o clientTest

clientTest.o: clientTest.c clientFunctions.h
	gcc -c clientTest.c

tui: tui.c clientFunctions.o
	gcc  tui.c -lform -lmenu -lncurses clientFunctions.o -o tui

tui.o: tui.c clientFunctions.h
	gcc -c tui.c

DatabaseHandler.o: DatabaseHandler.c DatabaseHandler.h
	gcc -c DatabaseHandler.c 

SocketUtilities.o: SocketUtilities.c SocketUtilities.h DatabaseHandler.h
	gcc -c SocketUtilities.c 

clientFunctions.o: clientFunctions.c clientFunctions.h SocketUtilities.h
	gcc -c clientFunctions.c 

clean:
	rm *.o
