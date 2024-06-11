HEADERS = SocketUtilities.h DatabaseHandler.h CommonUtils.h ClientProcedures.h CommonDefines.h

all:  Server YellowPagesTUI ClientTest GenerateDatabase

Server: Server.o SocketUtilities.o DatabaseHandler.o CommonUtils.o
	gcc Server.o SocketUtilities.o DatabaseHandler.o CommonUtils.o -o Server

YellowPagesTUI: YellowPagesTUI.c SocketUtilities.o DatabaseHandler.o CommonUtils.o ClientProcedures.o 
	gcc  YellowPagesTUI.c -lform -lmenu -lncurses SocketUtilities.o DatabaseHandler.o CommonUtils.o ClientProcedures.o -o YellowPagesTUI

ClientTest: ClientTest.o SocketUtilities.o ClientProcedures.o CommonUtils.o
	gcc ClientTest.c SocketUtilities.o ClientProcedures.o CommonUtils.o -o ClientTest

GenerateDatabase: GenerateDatabase.o DatabaseHandler.o CommonUtils.o
	gcc GenerateDatabase.c DatabaseHandler.o CommonUtils.o -o GenerateDatabase

# https://www.gnu.org/software/make/manual/html_node/Pattern-Examples.html
# https://www.gnu.org/software/make/manual/html_node/Automatic-Variables.html
%.o: %.c $(HEADERS)
	gcc -c $< -o $@

clean:
	rm *.o Server YellowPagesTUI ClientTest GenerateDatabase