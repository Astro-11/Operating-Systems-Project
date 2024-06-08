all: clientTest server tui

server: server.o SocketUtilities.o DatabaseHandler.o CommonUtils.o
	gcc server.o SocketUtilities.o DatabaseHandler.o CommonUtils.o -o server

server.o: server.c SocketUtilities.h
	gcc -c server.c
	

clientTest: clientTest.o SocketUtilities.o ClientProcedures.o CommonUtils.o
	gcc clientTest.c SocketUtilities.o ClientProcedures.o CommonUtils.o -o clientTest

clientTest.o: clientTest.c CommonUtils.h
	gcc -c clientTest.c

tui: tui.c SocketUtilities.o DatabaseHandler.o CommonUtils.o ClientProcedures.o 
	gcc  tui.c -lform -lmenu -lncurses SocketUtilities.o DatabaseHandler.o CommonUtils.o ClientProcedures.o -o tui

tui.o: tui.c CommonUtils.h ClientProcedures.h SocketUtilities.h DatabaseHandler.h
	gcc -c tui.c

ClientProcedures.o: ClientProcedures.c ClientProcedures.h
	gcc -c ClientProcedures.c

ClientProcedures.h: CommonDefines.h


DatabaseHandler.o: DatabaseHandler.c DatabaseHandler.h
	gcc -c DatabaseHandler.c 

DatabaseHandler.h: CommonDefines.h


SocketUtilities.o: SocketUtilities.c SocketUtilities.h 
	gcc -c SocketUtilities.c 

SocketUtilities.h: CommonDefines.h


CommonUtils.o: CommonUtils.c CommonUtils.h 
	gcc -c CommonUtils.c

CommonUtils.h: CommonDefines.h



clean:
	rm *.o