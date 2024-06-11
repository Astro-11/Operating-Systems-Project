HEADERS = SocketUtilities.h DatabaseHandler.h CommonUtils.h ClientProcedures.h CommonDefines.h

all:  server YellowPagesTUI clientTest generateDatabase

server: server.o SocketUtilities.o DatabaseHandler.o CommonUtils.o
	gcc server.o SocketUtilities.o DatabaseHandler.o CommonUtils.o -o server

YellowPagesTUI: YellowPagesTUI.c SocketUtilities.o DatabaseHandler.o CommonUtils.o ClientProcedures.o 
	gcc  YellowPagesTUI.c -lform -lmenu -lncurses SocketUtilities.o DatabaseHandler.o CommonUtils.o ClientProcedures.o -o YellowPagesTUI

clientTest: clientTest.o SocketUtilities.o ClientProcedures.o CommonUtils.o
	gcc clientTest.c SocketUtilities.o ClientProcedures.o CommonUtils.o -o clientTest

generateDatabase: generateDatabase.o DatabaseHandler.o CommonUtils.o
	gcc generateDatabase.c DatabaseHandler.o CommonUtils.o -o generateDatabase

# https://www.gnu.org/software/make/manual/html_node/Pattern-Examples.html
# https://www.gnu.org/software/make/manual/html_node/Automatic-Variables.html
%.o: %.c $(HEADERS)
	gcc -c $< -o $@

clean:
	rm *.o server YellowPagesTUI clientTest generateDatabase