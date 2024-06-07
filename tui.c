#include <curses.h>
#include <ncurses.h>
#include <menu.h>
#include <form.h>

#include <stdlib.h>
#include <string.h>

#include <stdio.h>


#include <stdio.h>
#include <unistd.h>
// #include <sys/types.h>
#include <sys/socket.h>
// #include <netinet/in.h>

// #include "DatabaseHandler.h"
// #include "SocketUtilities.h"
// #include "clientTest.c"
#include "clientFunctions.h"

#define MENU_CHOICES 6
#define MAX_FIELD_LEN 30
#define BUFFER_SIZE 256



int loginFormWindow();
int admin_client_routine(int client_socket);
int base_client_routine(int client_socket);

int open_form_to_add_new_entry(int clientSocket);
int open_form_to_delete_an_entry(int clientSocket);
int open_form_to_edit_an_entry(int clientSocket);

void server_option(int clientSocket, int option);



int main(int argc, char *argv[]) {
    
    // We create the connection and attempt to login if given a password
    int userType; // <- used to choose the interface of the application
    int client_socket = create_client_socket(SERVER_IP, PORT);
    
    if (argc == 1) {
        userType = BASE;
        no_login(client_socket);
    } else {
        userType = ADMIN;
        char response[MSG_LENGHT];
        login(client_socket, argv[1], response);
        if(strcmp(response, ACCESS_GRANTED) != 0 ){
            printf( "\n%s\nACCESS DENIED.\n",response);
            return 0;
        }
    }

    if ( userType == ADMIN ){
        admin_client_routine(client_socket);
    } else {
        base_client_routine(client_socket);
    }

    system("stty sane");
    system("clear");

}

int admin_client_routine(int clientSocket){
    ITEM **menuItems; // Holds the elements that will be part of the menu
    MENU *adminMenu;
    WINDOW *adminMenuWindow; 

    // ################################################################
    // ### Initialize ncurses and the options that we need for this interface
    initscr(); 
    cbreak();
    noecho();
    curs_set(0);
    start_color();
    keypad(stdscr, TRUE);
    
    // ################################################################
    // ### Create the list of items (The user options) for the menu from the array of strings 
    // ### and assembles them in the menu `adminMenu`
    char *choices[MENU_CHOICES] = {
    "Search the database",
    "Add new entry",
    "Edit an entry",
    "Remove an entry",
    "Exit and save",
    "Exit without saving" };

    menuItems = (ITEM **)calloc(MENU_CHOICES + 1, sizeof(ITEM *));
    for (int i = 0; i < MENU_CHOICES; i++) {
        menuItems[i] = new_item(choices[i], "");
        set_item_userptr(menuItems[i], choices[i]); // Set user pointer to the choice string
    }
    menuItems[MENU_CHOICES] = (ITEM *)NULL; // The array of items needs to have the last item set to NULL by specification

    adminMenu = new_menu(menuItems);

    // ################################################################
    // ### Create the window where the menu lives and the associated subwindow containing the items

    // Calculate window dimensions and positions
    // The top left corner is (0,0) and right and down from it are positive coordinates
    // LINES and COLS are defined by the ncurses library and hold informations about the terminal size
    int height = MENU_CHOICES + 4 + 6; // Total height of the menu window (including border and title)
    int width = 40;                    // Total width of the menu window
    int starty = (LINES - height) / 2; // Center the window vertically
    int startx = (COLS - width) / 2;   // Center the window horizontally

    adminMenuWindow = newwin(height, width, starty, startx);
    keypad(adminMenuWindow, TRUE); // Allows the use of special terminal keys in the given window. Like F1, Arrows, Home, PgDown...

    // Create the sub-window for the menu items with padding
    int xOffset = width / 3 - 1;
    int yOffset = height / 3 + 1;
    WINDOW *menuSubwin = derwin(adminMenuWindow, MENU_CHOICES, width - 2 - xOffset, yOffset, xOffset);
    set_menu_win(adminMenu, adminMenuWindow);
    set_menu_sub(adminMenu, menuSubwin);

    // Set up the menu (format and mark) and load it in the window (post)
    set_menu_format(adminMenu, MENU_CHOICES, 1); 
    set_menu_mark(adminMenu, "");
    post_menu(adminMenu);

    // Print a title and border around the menu window  
    box(adminMenuWindow, 0, 0);
    char header[] = "YellowPages TUI client";
    init_pair(1, COLOR_YELLOW, COLOR_BLACK); 
    wattron(adminMenuWindow, COLOR_PAIR(1));
    mvwprintw(adminMenuWindow, 2, (width - strlen(header)) / 2, "%s", header);
    wattroff(adminMenuWindow, COLOR_PAIR(1));
    
    // We print at the bottom of the screen a line with usefull info for the user
    // This is not printed on `adminMenuWindow` but on ncurses `stdscr`
    char * reminderText = "Press F1 to exit the program.";
    mvprintw(LINES - 2, 3, "%s", reminderText);

    refresh();                 // updates the visuals of stdscr
    wrefresh(adminMenuWindow); // updates the visuals of `adminMenuWindow`

    // ################################################################
    // ### Loop that handles the user input
    int choice;
    while ((choice = wgetch(adminMenuWindow)) != KEY_F(1)) {
        
        switch (choice) {
            case KEY_DOWN:
                menu_driver(adminMenu, REQ_DOWN_ITEM);
                break;
            case KEY_UP:
                menu_driver(adminMenu, REQ_UP_ITEM);
                break;
            case 10: // Enter key
                {
                    char *selectedOption = (char *)item_userptr(current_item(adminMenu));
                    
                    // ### SEARCH THE DATABASE
                    if (strcmp(selectedOption, "Search the database") == 0){

                        // We communicate to the server what we want to do
                        server_option(clientSocket, SEARCH_DB);

                        char server_response[BUFFER_SIZE];

                        int valread = recv(clientSocket, server_response, BUFFER_SIZE - 1, 0);
                        if (valread == -1) {
                            // perror("Receive error");
                            init_pair(4, COLOR_RED, COLOR_BLACK);
                            attron(COLOR_PAIR(4));
                            printw("IMPOSSIBLE TO REACH THE SERVER");
                            attroff( COLOR_PAIR(4));
                        } else {
                            server_response[valread] = '\0'; // Null-terminate the received data

                            

                            FILE *fp = fopen("serverResponses.txt", "a");
                            if (fp != NULL) {
                                fprintf(fp, "TUI          : %s\n", server_response);
                                fprintf(fp, "----------------\n");
                                fclose(fp);
                            }

                            move(LINES - 4, 3);
                            clrtoeol();
                            init_pair(3, COLOR_BLUE, COLOR_BLACK);
                            
                            printw("The server said: ");
                            attron(COLOR_PAIR(3));
                            printw("%s", server_response);
                            attroff( COLOR_PAIR(3));
                            refresh();
                        }
                        close(clientSocket);
                        
                    }
                    // ### ADD NEW ENTRY
                    else if (strcmp(selectedOption, "Add new entry") == 0) {
                        
                        clear();
                        mvprintw(LINES - 2, 3, "Press F1 to exit the program.");
                        refresh();
                        unpost_menu(adminMenu);

                        open_form_to_add_new_entry(clientSocket);
                        clear();
                        refresh();

                    } 
                    // ### EDIT ENTRY
                    else if (strcmp(selectedOption, "Edit an entry") == 0) {
                        
                        // # # # # # # # # # # # # 
                        // # # # # # # # # # # # # 
                        // # # # # # # # # # # # # 
                        // # # # # # # # # # # # # 

                    } 
                    // ### REMOVE ENTRY
                    else if (strcmp(selectedOption, "Remove an entry") == 0) {
                        
                        clear();
                        mvprintw(LINES - 2, 3, "Press F1 to exit the program.");
                        refresh();
                        unpost_menu(adminMenu);

                        open_form_to_delete_an_entry(clientSocket);
                        clear();
                        refresh();

                    } 
                    // ### EXIT WITHOUT SAVING
                    else if (strcmp(selectedOption, "Exit without saving") == 0) {
                        
                        // # # # # # # # # # # # # 
                        // # # # # # # # # # # # # 
                        // # # # # # # # # # # # # 
                        // # # # # # # # # # # # # 

                    } 
                    // ### EXIT AND SAVE
                    else if (strcmp(selectedOption, "Exit and save") == 0) {
                        unpost_menu(adminMenu);
                        free_menu(adminMenu);
                        for (int i = 0; i < MENU_CHOICES; ++i)
                            free_item(menuItems[i]);
                        endwin();
                        system("stty sane");
                        system("clear");
                        return 0;
                    }
                    // ### DEFAULT
                    else {
                        // Handle other menu choices here
                        // wrefresh(my_menu_win);
                        move(LINES - 4, 3);
                        clrtoeol();
                        printw("You selected: %s", selectedOption);
                        refresh();
                    }
                }
                break;
        }
        
        mvprintw(LINES - 2, 3, "Press F1 to exit the program.");
        refresh();
        box(adminMenuWindow, 0, 0);
        char header[] = "YellowPages TUI client";
        wattron(adminMenuWindow, COLOR_PAIR(1));
        mvwprintw(adminMenuWindow, 2, (width - strlen(header)) / 2, "%s", header);
        wattroff(adminMenuWindow, COLOR_PAIR(1));
        touchwin(adminMenuWindow);
        post_menu(adminMenu); 
        wrefresh(adminMenuWindow);
    }

    system("stty sane");
    system("clear");
    
    // Clean up
    unpost_menu(adminMenu);

    if(adminMenu != NULL){
        for (int i = 0; i < item_count(adminMenu); ++i)
            free_item(menu_items(adminMenu)[i]);
        free_menu(adminMenu);
        endwin();
    }
    
    return 0;
};

int base_client_routine(int client_socket){
    printf("\nunimplemented\n");
    return -1;
};

// Function to display a window with a form and save input to a file
int open_form_to_add_new_entry(int clientSocket) {

    curs_set(1); // Turns the cursor visibility on

    server_option(clientSocket, ADD_RECORD);

    // Create a new window for the form
    // Calculate window dimensions and positions
    
    int height = 12;                   // Total height of the menu window (including border and title)
    int width = 60;                    // Total width of the menu window
    int starty = (LINES - height) / 2; // Center the window vertically
    int startx = (COLS - width) / 2;   // Center the window horizontally

    WINDOW *newEntryFormWindow = newwin(height, width, starty, startx);
    box(newEntryFormWindow, 0, 0);
    char header[] = "Please submit the new entry:";
    mvwprintw(newEntryFormWindow, 1, (width - strlen(header)) / 2, "%s", header);
    mvwprintw(newEntryFormWindow, 3, 1, "%8s:","Name");
    mvwprintw(newEntryFormWindow, 4, 1, "%8s:","Address");
    mvwprintw(newEntryFormWindow, 5, 1, "%8s:","Phone");

    // Create form fields
    FIELD *fields[4];
    int gap = 2;
    fields[0] = new_field(1, MAX_FIELD_LEN, 0, gap, 0, 0);
    fields[1] = new_field(1, MAX_FIELD_LEN, 1, gap, 0, 0);
    fields[2] = new_field(1, MAX_FIELD_LEN, 2, gap, 0, 0);
    fields[3] = NULL;

    // Highlits the fields with an underscore character 
    set_field_back(fields[0], A_UNDERLINE);
    set_field_back(fields[1], A_UNDERLINE);
    set_field_back(fields[2], A_UNDERLINE);


    // Create the form
    FORM *my_form = new_form(fields);
    set_form_win(my_form, newEntryFormWindow);

    scale_form(my_form, &height, &width);

    WINDOW* formSubWin = derwin(newEntryFormWindow, height, width, 3, 9);
    set_form_sub(my_form, formSubWin);
    post_form(my_form);

    keypad(newEntryFormWindow, TRUE);
    wrefresh(newEntryFormWindow);

    int ch;
    while ((ch = getch()) != KEY_F(1)) {
        switch (ch) {
            case KEY_DOWN:
                form_driver(my_form, REQ_NEXT_FIELD);
                form_driver(my_form, REQ_END_LINE);
                break;
            case KEY_UP:
                form_driver(my_form, REQ_PREV_FIELD);
                form_driver(my_form, REQ_END_LINE);
                break;
            case KEY_LEFT:
                form_driver(my_form, REQ_PREV_CHAR);
                break;
            case KEY_RIGHT:
                form_driver(my_form, REQ_NEXT_CHAR);
                break;
            case KEY_BACKSPACE:
            case 127: // Handle backspace key
                form_driver(my_form, REQ_DEL_PREV);
                break;
            case 10: // Enter key
                form_driver(my_form, REQ_VALIDATION);

                // Retrieve field values
                // char *name = field_buffer(fields[0], 0);
                // char *phone = field_buffer(fields[1], 0);
                // char *email = field_buffer(fields[2], 0);

                dataEntry newDataEntry;
                strncpy(newDataEntry.name, rtrim(field_buffer(fields[0], 0)), sizeof(newDataEntry.name) - 1);
                strncpy(newDataEntry.address,rtrim(field_buffer(fields[1], 0)), sizeof(newDataEntry.address) - 1);
                strncpy(newDataEntry.phoneNumber,rtrim(field_buffer(fields[2], 0)), sizeof(newDataEntry.phoneNumber) - 1);

                // sendDataEntry(clientSocket, &newDataEntry);
                char errorMessage[MSG_LENGHT];
                int outcome = add_new_record(clientSocket, newDataEntry, errorMessage);

                // receive_signal(clientSocket, &outcome);
                if (outcome < 0) {
                    // char failureMsg[MSG_LENGHT];
                    // receiveMsg(clientSocket, failureMsg);

                    init_pair(2, COLOR_RED, COLOR_BLACK);
                    wattron(newEntryFormWindow, COLOR_PAIR(2));

                    // char footer[] = "Error: Invalid entry";
                    mvwprintw(newEntryFormWindow, getmaxy(newEntryFormWindow)-2, (getmaxx(newEntryFormWindow) - strlen(errorMessage)) / 2, "%s", errorMessage);
                    
                    wattroff(newEntryFormWindow, COLOR_PAIR(2));
                } else {
                    init_pair(2, COLOR_GREEN, COLOR_BLACK);
                    wattron(newEntryFormWindow, COLOR_PAIR(2));

                    char footer[] = "Entry succesfully added";
                    mvwprintw(newEntryFormWindow, getmaxy(newEntryFormWindow)-2, (getmaxx(newEntryFormWindow) - strlen(footer)) / 2, "%s", footer);
                    
                    wattroff(newEntryFormWindow, COLOR_PAIR(2));
                }
                wrefresh(newEntryFormWindow);
                getch();

                // Clean up the form and window
                curs_set(0);

                unpost_form(my_form);
                free_form(my_form);
                free_field(fields[0]);
                free_field(fields[1]);
                delwin(newEntryFormWindow);
                endwin();

                return 1; // Return status 1 for successful insertion
            default:
                form_driver(my_form, ch);
                break;
        }
        touchwin(newEntryFormWindow);
        wrefresh(newEntryFormWindow);
    }

    // ### If the submission is aborted send dummy data
    char name[MSG_LENGHT] = "0";
    char address[MSG_LENGHT] = "0";
    char phoneNumber[MSG_LENGHT] = "0";

    dataEntry newDataEntry;
    strcpy(newDataEntry.name, name);
    strcpy(newDataEntry.address,address);
    strcpy(newDataEntry.phoneNumber, phoneNumber);

    sendDataEntry(clientSocket, &newDataEntry);
    int outcome;
    receive_signal(clientSocket, &outcome);
    
    // Clean up the form and window
    curs_set(0);

    unpost_form(my_form);
    free_form(my_form);
    free_field(fields[0]);
    free_field(fields[1]);
    delwin(newEntryFormWindow);
    endwin();

    return 1; // Return status 0 for insertion cancelled
}

int open_form_to_delete_an_entry(int clientSocket){
    curs_set(1); // Turns the cursor visibility on

    server_option(clientSocket, REMOVE_RECORD);

    // Create a new window for the form
    // Calculate window dimensions and positions
    
    int height = 12;                   // Total height of the menu window (including border and title)
    int width = 60;                    // Total width of the menu window
    int starty = (LINES - height) / 2; // Center the window vertically
    int startx = (COLS - width) / 2;   // Center the window horizontally

    WINDOW *deleteEntryFormWindow = newwin(height, width, starty, startx);
    box(deleteEntryFormWindow, 0, 0);
    char header[] = "!!! Define UNIVOCALLY the entry to DELETE !!!";
    init_pair(3, COLOR_RED, COLOR_BLACK);
    wattron(deleteEntryFormWindow, COLOR_PAIR(3));
    mvwprintw(deleteEntryFormWindow, 1, (width - strlen(header)) / 2, "%s", header);
    wattroff(deleteEntryFormWindow, COLOR_PAIR(3));

    mvwprintw(deleteEntryFormWindow, 3, 1, "%8s:","Name");
    mvwprintw(deleteEntryFormWindow, 4, 1, "%8s:","Address");
    mvwprintw(deleteEntryFormWindow, 5, 1, "%8s:","Phone");

    // Create form fields
    FIELD *fields[4];
    int gap = 2;
    fields[0] = new_field(1, MAX_FIELD_LEN, 0, gap, 0, 0);
    fields[1] = new_field(1, MAX_FIELD_LEN, 1, gap, 0, 0);
    fields[2] = new_field(1, MAX_FIELD_LEN, 2, gap, 0, 0);
    fields[3] = NULL;

    // Highlits the fields with an underscore character 
    set_field_back(fields[0], A_UNDERLINE);
    set_field_back(fields[1], A_UNDERLINE);
    set_field_back(fields[2], A_UNDERLINE);


    // Create the form
    FORM *my_form = new_form(fields);
    set_form_win(my_form, deleteEntryFormWindow);

    scale_form(my_form, &height, &width);

    WINDOW* formSubWin = derwin(deleteEntryFormWindow, height, width, 3, 9);
    set_form_sub(my_form, formSubWin);
    post_form(my_form);

    keypad(deleteEntryFormWindow, TRUE);
    wrefresh(deleteEntryFormWindow);

    int ch;
    while ((ch = getch()) != KEY_F(1)) {
        switch (ch) {
            case KEY_DOWN:
                form_driver(my_form, REQ_NEXT_FIELD);
                form_driver(my_form, REQ_END_LINE);
                break;
            case KEY_UP:
                form_driver(my_form, REQ_PREV_FIELD);
                form_driver(my_form, REQ_END_LINE);
                break;
            case KEY_LEFT:
                form_driver(my_form, REQ_PREV_CHAR);
                break;
            case KEY_RIGHT:
                form_driver(my_form, REQ_NEXT_CHAR);
                break;
            case KEY_BACKSPACE:
            case 127: // Handle backspace key
                form_driver(my_form, REQ_DEL_PREV);
                break;
            case 10: // Enter key
                form_driver(my_form, REQ_VALIDATION);

                // Retrieve field values
                char *name = field_buffer(fields[0], 0);
                char *phone = field_buffer(fields[1], 0);
                char *email = field_buffer(fields[2], 0);

                dataEntry entryToDelete;
                strncpy(entryToDelete.name, rtrim(field_buffer(fields[0], 0)), sizeof(entryToDelete.name) - 1);
                strncpy(entryToDelete.address,rtrim(field_buffer(fields[1], 0)), sizeof(entryToDelete.address) - 1);
                strncpy(entryToDelete.phoneNumber,rtrim(field_buffer(fields[2], 0)), sizeof(entryToDelete.phoneNumber) - 1);

                sendDataEntry(clientSocket, &entryToDelete);

                int outcome = -1;
                receive_signal(clientSocket, &outcome);

                if (outcome != 0) {
                    char failureMsg[MSG_LENGHT];
                    receiveMsg(clientSocket, failureMsg);
                    remove_extra_whitespace(failureMsg);
                    init_pair(2, COLOR_RED, COLOR_BLACK);
                    wattron(deleteEntryFormWindow, COLOR_PAIR(2));

                    char footer[] = "Request failed!";
                    mvwprintw(deleteEntryFormWindow, getmaxy(deleteEntryFormWindow)-2, (getmaxx(deleteEntryFormWindow) - strlen(failureMsg)) / 2, "%s", failureMsg);
                    
                    wattroff(deleteEntryFormWindow, COLOR_PAIR(2));
                } else {
                    init_pair(2, COLOR_GREEN, COLOR_BLACK);
                    wattron(deleteEntryFormWindow, COLOR_PAIR(2));
                    
                    char footer[] = "Succesfully removed the entry";
                    mvwprintw(deleteEntryFormWindow, getmaxy(deleteEntryFormWindow)-2, (getmaxx(deleteEntryFormWindow) - strlen(footer)) / 2, "%s", footer);
                    
                    wattroff(deleteEntryFormWindow, COLOR_PAIR(2));
                }
                wrefresh(deleteEntryFormWindow);
                getch();

                // Clean up the form and window
                curs_set(0);

                unpost_form(my_form);
                free_form(my_form);
                free_field(fields[0]);
                free_field(fields[1]);
                delwin(deleteEntryFormWindow);
                endwin();

                return 1; // Return status 1 for successful insertion
            default:
                form_driver(my_form, ch);
                break;
        }
        touchwin(deleteEntryFormWindow);
        wrefresh(deleteEntryFormWindow);
    }

    // ### If the submission is aborted send dummy data
    char name[MSG_LENGHT] = "0";
    char address[MSG_LENGHT] = "0";
    char phoneNumber[MSG_LENGHT] = "0";

    dataEntry newDataEntry;
    strcpy(newDataEntry.name, name);
    strcpy(newDataEntry.address,address);
    strcpy(newDataEntry.phoneNumber, phoneNumber);

    sendDataEntry(clientSocket, &newDataEntry);
    int outcome;
    receive_signal(clientSocket, &outcome);
    
    // Clean up the form and window
    curs_set(0);

    unpost_form(my_form);
    free_form(my_form);
    free_field(fields[0]);
    free_field(fields[1]);
    delwin(deleteEntryFormWindow);
    endwin();

    return 1; // Return status 0 for insertion cancelled
}

int open_form_to_edit_an_entry(int clientSocket){
    
    curs_set(1); // Turns the cursor visibility on
    server_option(clientSocket, EDIT_RECORD);

    // Create a new window for the form
    // Calculate window dimensions and positions
    
    int height = 12;                   // Total height of the menu window (including border and title)
    int width = 60;                    // Total width of the menu window
    int starty = (LINES - height) / 2; // Center the window vertically
    int startx = (COLS - width) / 2;   // Center the window horizontally

    WINDOW *editEntryFormWindow = newwin(height, width, starty, startx);
    box(editEntryFormWindow, 0, 0);
    init_pair(3, COLOR_MAGENTA, COLOR_BLACK);
    wattron(editEntryFormWindow, COLOR_PAIR(3));
    char header[] = "!!! Define UNIVOCALLY the entry to EDIT !!!";
    wattroff(editEntryFormWindow, COLOR_PAIR(3));

    mvwprintw(editEntryFormWindow, 1, (width - strlen(header)) / 2, "%s", header);
    mvwprintw(editEntryFormWindow, 3, 1, "%8s:","Name");
    mvwprintw(editEntryFormWindow, 4, 1, "%8s:","Address");
    mvwprintw(editEntryFormWindow, 5, 1, "%8s:","Phone");

    // Create form fields
    FIELD *fields[4];
    int gap = 2;
    fields[0] = new_field(1, MAX_FIELD_LEN, 0, gap, 0, 0);
    fields[1] = new_field(1, MAX_FIELD_LEN, 1, gap, 0, 0);
    fields[2] = new_field(1, MAX_FIELD_LEN, 2, gap, 0, 0);
    fields[3] = NULL;

    // Highlits the fields with an underscore character 
    set_field_back(fields[0], A_UNDERLINE);
    set_field_back(fields[1], A_UNDERLINE);
    set_field_back(fields[2], A_UNDERLINE);


    // Create the form
    FORM *my_form = new_form(fields);
    set_form_win(my_form, editEntryFormWindow);

    scale_form(my_form, &height, &width);

    WINDOW* formSubWin = derwin(editEntryFormWindow, height, width, 3, 9);
    set_form_sub(my_form, formSubWin);
    post_form(my_form);

    keypad(editEntryFormWindow, TRUE);
    wrefresh(editEntryFormWindow);

    int search = 1;

    int ch;
    while ((ch = getch()) != KEY_F(1)) {
        switch (ch) {
            case KEY_DOWN:
                form_driver(my_form, REQ_NEXT_FIELD);
                form_driver(my_form, REQ_END_LINE);
                break;
            case KEY_UP:
                form_driver(my_form, REQ_PREV_FIELD);
                form_driver(my_form, REQ_END_LINE);
                break;
            case KEY_LEFT:
                form_driver(my_form, REQ_PREV_CHAR);
                break;
            case KEY_RIGHT:
                form_driver(my_form, REQ_NEXT_CHAR);
                break;
            case KEY_BACKSPACE:
            case 127: // Handle backspace key
                form_driver(my_form, REQ_DEL_PREV);
                break;
            case 10: // Enter key
                form_driver(my_form, REQ_VALIDATION);

                // Retrieve field values
                char *name = field_buffer(fields[0], 0);
                char *phone = field_buffer(fields[1], 0);
                char *email = field_buffer(fields[2], 0);

                dataEntry entryToEdit;
                strncpy(entryToEdit.name, rtrim(field_buffer(fields[0], 0)), sizeof(entryToEdit.name) - 1);
                strncpy(entryToEdit.address,rtrim(field_buffer(fields[1], 0)), sizeof(entryToEdit.address) - 1);
                strncpy(entryToEdit.phoneNumber,rtrim(field_buffer(fields[2], 0)), sizeof(entryToEdit.phoneNumber) - 1);

                sendDataEntry(clientSocket, &entryToEdit);

                int outcome = -1;
                receive_signal(clientSocket, &outcome);
                if (outcome != 0) {
                    char failureMsg[MSG_LENGHT];
                    receiveMsg(clientSocket, failureMsg);
                    remove_extra_whitespace(failureMsg);
                    init_pair(2, COLOR_RED, COLOR_BLACK);
                    wattron(editEntryFormWindow, COLOR_PAIR(2));

                    char footer[] = "Request failed!";
                    mvwprintw(editEntryFormWindow, getmaxy(editEntryFormWindow)-2, (getmaxx(editEntryFormWindow) - strlen(failureMsg)) / 2, "%s", failureMsg);
                    
                    wattroff(editEntryFormWindow, COLOR_PAIR(2));
                } else {
                    init_pair(2, COLOR_GREEN, COLOR_BLACK);
                    wattron(editEntryFormWindow, COLOR_PAIR(2));
                    
                    char footer[] = "Succesfully removed the entry";
                    mvwprintw(editEntryFormWindow, getmaxy(editEntryFormWindow)-2, (getmaxx(editEntryFormWindow) - strlen(footer)) / 2, "%s", footer);
                    
                    wattroff(editEntryFormWindow, COLOR_PAIR(2));
                }
                wrefresh(editEntryFormWindow);
                getch();

                // Clean up the form and window
                curs_set(0);

                unpost_form(my_form);
                free_form(my_form);
                free_field(fields[0]);
                free_field(fields[1]);
                delwin(editEntryFormWindow);
                endwin();

                return 1; // Return status 1 for successful insertion
            default:
                form_driver(my_form, ch);
                break;
        }
        touchwin(editEntryFormWindow);
        wrefresh(editEntryFormWindow);
    }

    // ### If the submission is aborted send dummy data
    char name[MSG_LENGHT] = "0";
    char address[MSG_LENGHT] = "0";
    char phoneNumber[MSG_LENGHT] = "0";

    dataEntry newDataEntry;
    strcpy(newDataEntry.name, name);
    strcpy(newDataEntry.address,address);
    strcpy(newDataEntry.phoneNumber, phoneNumber);

    sendDataEntry(clientSocket, &newDataEntry);
    int outcome;
    receive_signal(clientSocket, &outcome);
    
    // Clean up the form and window
    curs_set(0);

    unpost_form(my_form);
    free_form(my_form);
    free_field(fields[0]);
    free_field(fields[1]);
    delwin(editEntryFormWindow);
    endwin();

    return 1; // Return status 0 for insertion cancelled
}


int loginFormWindow() {

    curs_set(1);

    // Create a new window for the form

    // Calculate window dimensions and positions
    int height = 12; // Total height of the menu window (including border and title)
    int width = 60;                    // Total width of the menu window
    int starty = (LINES - height) / 2; // Center the window vertically
    int startx = (COLS - width) / 2;   // Center the window horizontally

    WINDOW *form_win = newwin(height, width, starty, startx);
    box(form_win, 0, 0);
    char header[] = "Please enter your information:";
    mvwprintw(form_win, 1, (width - strlen(header)) / 2, "%s","Please enter your information:");
    mvwprintw(form_win, 3, 1, "%8s","Name: ");
    mvwprintw(form_win, 4, 1, "%8s","Pass: ");
    

    // Create form fields
    FIELD *fields[4];
    int gap = 2;
    fields[0] = new_field(1, MAX_FIELD_LEN, 0, gap, 0, 0);
    fields[1] = new_field(1, MAX_FIELD_LEN, 1, gap, 0, 0);
    fields[2] = NULL;

    // Set field options
    set_field_back(fields[0], A_UNDERLINE);
    set_field_back(fields[1], A_UNDERLINE);

    field_opts_off(fields[0], O_AUTOSKIP);
    field_opts_off(fields[1], O_AUTOSKIP);


    // Create the form
    FORM *my_form = new_form(fields);
    set_form_win(my_form, form_win);

    scale_form(my_form, &height, &width);

    WINDOW* formSubWin = derwin(form_win, height, width, 3, 9);
    set_form_sub(my_form, formSubWin);
    post_form(my_form);

    keypad(form_win, TRUE);
    wrefresh(form_win);

    int ch;
    while ((ch = getch()) != KEY_F(1)) {
        switch (ch) {
            case KEY_DOWN:
                form_driver(my_form, REQ_NEXT_FIELD);
                form_driver(my_form, REQ_END_LINE);
                break;
            case KEY_UP:
                form_driver(my_form, REQ_PREV_FIELD);
                form_driver(my_form, REQ_END_LINE);
                break;
            case KEY_LEFT:
                form_driver(my_form, REQ_PREV_CHAR);
                break;
            case KEY_RIGHT:
                form_driver(my_form, REQ_NEXT_CHAR);
                break;
            case KEY_BACKSPACE:
            case 127: // Handle backspace key
                form_driver(my_form, REQ_DEL_PREV);
                break;
            case 10: // Enter key
                form_driver(my_form, REQ_VALIDATION);

                // Retrieve field values
                // char *name = field_buffer(fields[0], 0);
                // char *pass = field_buffer(fields[1], 0);

                // int client_socket = create_client_socket(SERVER_IP, PORT);
                // login(client_socket, name, pass);
                
                // if(0){
                //     // TODO: Failed to login print red stuff
                // }

                curs_set(0);

                unpost_form(my_form);
                free_form(my_form);
                free_field(fields[0]);
                free_field(fields[1]);
                delwin(form_win);
                endwin();
                return 1; // Return status 1 for successful insertion
            default:
                form_driver(my_form, ch);
                break;
        }
        touchwin(form_win);
        wrefresh(form_win);
    }

    // Clean up the form and window
    curs_set(0);

    unpost_form(my_form);
    free_form(my_form);
    free_field(fields[0]);
    free_field(fields[1]);
    delwin(form_win);
    endwin();

    return 0; // Return status 0 for insertion cancelled
}

// A self evident util
void fill_pad_with_entries(WINDOW *pad, dataEntry contacts[], int numContacts) {
    for (int i = 0; i < numContacts; i++) {
        wprintw(pad, "  # %d\n", i);
        wprintw(pad, "  Name: %s\n", contacts[i].name);
        wprintw(pad, "  Address: %s\n", contacts[i].address);
        wprintw(pad, "  Phone Number: %s\n", contacts[i].phoneNumber);
        wprintw(pad, "  ----------------------------------------\n");
    }
}

void show_search_results(){};

void server_option(int clientSocket, int option){
    int request = option;
    send_signal(clientSocket, &request);
};