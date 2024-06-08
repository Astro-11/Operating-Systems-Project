HEADERS = SocketUtilities.h DatabaseHandler.h CommonUtils.h ClientProcedures.h CommonDefines.h

all:  server tui #clientTest

server: server.o SocketUtilities.o DatabaseHandler.o CommonUtils.o
	gcc server.o SocketUtilities.o DatabaseHandler.o CommonUtils.o -o server

tui: tui.c SocketUtilities.o DatabaseHandler.o CommonUtils.o ClientProcedures.o 
	gcc  tui.c -lform -lmenu -lncurses SocketUtilities.o DatabaseHandler.o CommonUtils.o ClientProcedures.o -o tui

clientTest: clientTest.o SocketUtilities.o ClientProcedures.o CommonUtils.o
	gcc clientTest.c SocketUtilities.o ClientProcedures.o CommonUtils.o -o clientTest

%.o: %.c $(HEADERS)
	gcc -c $< -o $@

clean:
	rm *.o server tui clientTest