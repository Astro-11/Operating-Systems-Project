#include <ncurses.h>
#include <menu.h>
#include <form.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

// #include <stdio.h> // If we want to save to file


#include "ClientProcedures.h"

#define MENU_CHOICES 6
#define MAX_FIELD_LEN 30
#define BUFFER_SIZE 256

#define DEBUG 0

int admin_client_routine(int socket);
void base_client_routine(int socket);

int open_form_to_search(int clientSocket);
int open_form_to_add_new_entry(int clientSocket);
int open_form_to_delete_an_entry(int clientSocket);
int open_form_to_edit_an_entry(int clientSocket);
void build_form_window(WINDOW **w, FORM **f, char *title);
void clean_form_window(WINDOW **w, FORM **f);

void fillPadWithContacts(WINDOW *pad, dataEntry results[], int totalResults);
int show_entries_in_array(dataEntry results[], int totalResults);

void display_form_message(WINDOW *win, const char *message, int color);
void admin_welcome();
void assemble_admin_window(WINDOW *w, MENU *m);
void user_welcome();
void terminate_process(int sig);

int client_socket;
int logoutValue = 0;

int main(int argc, char *argv[]) {
    int userType = BASE;
    char *password = "0";
    
    if (argc > 1) {
        userType = ADMIN;
        password = argv[1];
    }

    client_socket = init(password);
    if(client_socket < 0 ){
        printf( "\nACCESS DENIED.\n");
        return -1;
    }

    signal(SIGINT, terminate_process);
    if ( userType == ADMIN )
        logoutValue = admin_client_routine(client_socket);
    else
        base_client_routine(client_socket);
    
    terminate_process(0);
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
    keypad(adminMenuWindow, TRUE);

    // Create the sub-window for the menu items with padding
    int xOffset = width / 3 - 1;
    int yOffset = height / 3 + 1;
    WINDOW *menuSubwin = derwin(adminMenuWindow, MENU_CHOICES, width - 2 - xOffset, yOffset, xOffset);
    set_menu_win(adminMenu, adminMenuWindow);
    set_menu_sub(adminMenu, menuSubwin);

    // Set up the menu (format and mark) 
    set_menu_format(adminMenu, MENU_CHOICES, 1); 
    set_menu_mark(adminMenu, "");
    
    // This function decorates and shows the menu window, more details in the declaration
    assemble_admin_window(adminMenuWindow, adminMenu);

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
            // Enter key
            case 10: { 
                char *selectedOption = (char *)item_userptr(current_item(adminMenu));
                
                admin_welcome();
                unpost_menu(adminMenu);

                /* SEARCH THE DATABASE */
                if (strcmp(selectedOption, "Search the database") == 0)
                    open_form_to_search(clientSocket);
                /* ADD NEW ENTRY */
                else if (strcmp(selectedOption, "Add new entry") == 0)
                    open_form_to_add_new_entry(clientSocket);
                /* EDIT ENTRY */
                else if (strcmp(selectedOption, "Edit an entry") == 0)
                    open_form_to_edit_an_entry(clientSocket);
                /* REMOVE ENTRY */
                else if (strcmp(selectedOption, "Remove an entry") == 0)
                    open_form_to_delete_an_entry(clientSocket);
                /* EXIT AND SAVE */
                else if (strcmp(selectedOption, "Exit and save") == 0)
                    return 1;
                /* EXIT WITHOUT SAVING */
                else if (strcmp(selectedOption, "Exit without saving") == 0)
                    return 0;
            } break;
        }
        assemble_admin_window(adminMenuWindow, adminMenu);  
    } 
    return 0;
};

void base_client_routine(int clientSocket){

    initscr(); 
    cbreak();
    noecho();
    curs_set(0);
    start_color();
    keypad(stdscr, TRUE);

    do {
        user_welcome();
        open_form_to_search(clientSocket);
        mvprintw(LINES - 3, 3, "Press any key make another search.");
        mvprintw(LINES - 2, 3, "Press F1 to exit/go back.");
    }while (getch() != KEY_F(1));

};

int open_form_to_search(int clientSocket) {

    WINDOW *formWindow;
    FORM *myForm;
    char title[] = "Who are you looking for?";

    build_form_window(&formWindow, &myForm, title);
    FIELD **fields = form_fields(myForm);
    mvprintw(LINES - 2, 3, "Press F1 to go back/exit. Press F2 to save results to searchResults.txt");
    form_driver(myForm, REQ_FIRST_FIELD);
    refresh();
    wrefresh(formWindow);

    int outcome=-1;
    dataEntry results[128];
    int exit = 0;
    int ch;
    while (!exit && (ch = getch()) != KEY_F(1)) {
        switch (ch) {
            case KEY_DOWN:
                form_driver(myForm, REQ_NEXT_FIELD);
                form_driver(myForm, REQ_END_LINE);
                break;
            case KEY_UP:
                form_driver(myForm, REQ_PREV_FIELD);
                form_driver(myForm, REQ_END_LINE);
                break;
            case KEY_LEFT:
                form_driver(myForm, REQ_PREV_CHAR);
                break;
            case KEY_RIGHT:
                form_driver(myForm, REQ_NEXT_CHAR);
                break;
            case KEY_BACKSPACE:
            case 127: // Handle backspace key
                form_driver(myForm, REQ_DEL_PREV);
                break;
            case 10: // Enter key
                form_driver(myForm, REQ_VALIDATION);

                dataEntry filterEntry;
                strcpy(filterEntry.name, field_buffer(fields[0], 0));
                strcpy(filterEntry.address,field_buffer(fields[1], 0));
                strcpy(filterEntry.phoneNumber,field_buffer(fields[2], 0));

                outcome = search_record(clientSocket, filterEntry, results);

                #if DEBUG
                    FILE *file = fopen("output.txt", "a");
                    fprintf(file, "DEBUG search_form\n");
                    fprintf(file, "outcome: %d\n", outcome);
                    fclose(file);
                #endif

                if (outcome <= 0) 
                    display_form_message(formWindow, "No entry matches the search", 1);

                exit = 1;
                break;
            default:
                form_driver(myForm, ch);
                break;
        }
        touchwin(formWindow);
        wrefresh(formWindow);
    }

    clean_form_window(&formWindow, &myForm);

    if(outcome > 0)
        show_entries_in_array(results, outcome);

    return 0;
}

// Function to display a window with a form and save input to a file
int open_form_to_add_new_entry(int clientSocket) {
    WINDOW *formWindow;
    FORM *myForm;
    char title[] = "Submit the new entry";

    build_form_window(&formWindow, &myForm, title);
    FIELD **fields = form_fields(myForm);

    int exit = 0;
    int ch;
    while ( !exit && (ch = getch()) != KEY_F(1)) {
        switch (ch) {
            case KEY_DOWN:
                form_driver(myForm, REQ_NEXT_FIELD);
                form_driver(myForm, REQ_END_LINE);
                break;
            case KEY_UP:
                form_driver(myForm, REQ_PREV_FIELD);
                form_driver(myForm, REQ_END_LINE);
                break;
            case KEY_LEFT:
                form_driver(myForm, REQ_PREV_CHAR);
                break;
            case KEY_RIGHT:
                form_driver(myForm, REQ_NEXT_CHAR);
                break;
            case KEY_BACKSPACE:
            case 127: // Handle backspace key
                form_driver(myForm, REQ_DEL_PREV);
                break;
            case 10:{ // Enter key
                form_driver(myForm, REQ_VALIDATION);

                dataEntry newEntry;
                strcpy(newEntry.name, field_buffer(fields[0], 0));
                strcpy(newEntry.address,field_buffer(fields[1], 0));
                strcpy(newEntry.phoneNumber,field_buffer(fields[2], 0));

                #if DEBUG
                    FILE *file = fopen("output.txt", "w");
                    if (file == NULL) return 1;
                    fprintf(file, "%s\n", newEntry.name);
                    fprintf(file, "%s\n", newEntry.address);
                    fprintf(file, "%s\n", newEntry.phoneNumber);
                    fclose(file);
                #endif

                char errorMessage[MSG_LENGHT] = "???";
                int outcome = add_new_record(clientSocket, newEntry, errorMessage);

                if (outcome < 0)
                    display_form_message(formWindow, errorMessage, 1);
                else
                    display_form_message(formWindow, "Entry succesfully added", 2);

                exit = 1;
                
                } break;
            default:
                form_driver(myForm, ch);
                break;
        }
        touchwin(formWindow);
        wrefresh(formWindow);
    }

    clean_form_window(&formWindow, &myForm);

    return 1;
}

int open_form_to_delete_an_entry(int clientSocket){

    WINDOW *formWindow;
    FORM *myForm;
    char title[] = "Who are you looking for?";

    build_form_window(&formWindow, &myForm, title);

    FIELD **fields = form_fields(myForm);

    int exit = 0;
    int ch;
    while ( !exit && (ch = getch()) != KEY_F(1)) {
        switch (ch) {
            case KEY_DOWN:
                form_driver(myForm, REQ_NEXT_FIELD);
                form_driver(myForm, REQ_END_LINE);
                break;
            case KEY_UP:
                form_driver(myForm, REQ_PREV_FIELD);
                form_driver(myForm, REQ_END_LINE);
                break;
            case KEY_LEFT:
                form_driver(myForm, REQ_PREV_CHAR);
                break;
            case KEY_RIGHT:
                form_driver(myForm, REQ_NEXT_CHAR);
                break;
            case KEY_BACKSPACE:
            case 127: // Handle backspace key
                form_driver(myForm, REQ_DEL_PREV);
                break;
            case 10: // Enter key
                form_driver(myForm, REQ_VALIDATION);

                dataEntry newEntry;
                strcpy(newEntry.name, field_buffer(fields[0], 0));
                strcpy(newEntry.address,field_buffer(fields[1], 0));
                strcpy(newEntry.phoneNumber,field_buffer(fields[2], 0));

                #if DEBUG
                    FILE *file = fopen("output.txt", "w");
                    if (file == NULL) return 1;
                    fprintf(file, "%s\n", newEntry.name);
                    fprintf(file, "%s\n", newEntry.address);
                    fprintf(file, "%s\n", newEntry.phoneNumber);
                    fclose(file);
                #endif

                char errorMessage[MSG_LENGHT] = "???";
                int outcome = delete_record(clientSocket, newEntry, errorMessage);

                if (outcome < 0)
                    display_form_message(formWindow, errorMessage, 1);
                else
                    display_form_message(formWindow, "Entry succesfully removed", 2);

                exit = 1;
                break;
            default:
                form_driver(myForm, ch);
                break;
        }
        touchwin(formWindow);
        wrefresh(formWindow);
    }

    clean_form_window(&formWindow, &myForm);

    return 1; 
}

int open_form_to_edit_an_entry(int clientSocket){
    
    WINDOW *formWindow;
    FORM *myForm;
    char title[] = "Submit the new entry";

    build_form_window(&formWindow, &myForm, title);
    FIELD **fields = form_fields(myForm);

    int exit = 0;
    int search = 1;
    dataEntry results[128];
    int ch;
    while ( !exit &&  (ch = getch()) != KEY_F(1)) {
        switch (ch) {
            case KEY_DOWN:
                form_driver(myForm, REQ_NEXT_FIELD);
                form_driver(myForm, REQ_END_LINE);
                break;
            case KEY_UP:
                form_driver(myForm, REQ_PREV_FIELD);
                form_driver(myForm, REQ_END_LINE);
                break;
            case KEY_LEFT:
                form_driver(myForm, REQ_PREV_CHAR);
                break;
            case KEY_RIGHT:
                form_driver(myForm, REQ_NEXT_CHAR);
                break;
            case KEY_BACKSPACE:
            case 127: // Handle backspace key
                form_driver(myForm, REQ_DEL_PREV);
                break;
            case 10: // Enter key
               form_driver(myForm, REQ_VALIDATION);

                dataEntry submittedEntry;
                strcpy(submittedEntry.name, field_buffer(fields[0], 0));
                strcpy(submittedEntry.address,field_buffer(fields[1], 0));
                strcpy(submittedEntry.phoneNumber,field_buffer(fields[2], 0));

                #if DEBUG
                    FILE *file = fopen("output.txt", "w");
                    if (file == NULL) return 1;
                    fprintf(file, "%s\n", submittedEntry.name);
                    fprintf(file, "%s\n", submittedEntry.address);
                    fprintf(file, "%s\n", submittedEntry.phoneNumber);
                    fclose(file);
                #endif

                if(search == 1){
                    int outcome = search_record(clientSocket, submittedEntry, results);

                    if (outcome == 1) {
                        display_form_message(formWindow, "Success, press any key to continue", 2);
                        search = 0;
                        set_field_buffer(fields[0], 0, results[0].name);
                        set_field_buffer(fields[1], 0, results[0].address);
                        set_field_buffer(fields[2], 0, results[0].phoneNumber);
                    } else {
                        display_form_message(formWindow, "Error selecting entry", 1);
                        exit = 1;
                    }
                } else { 
                    char blank[] = "                                        ";
                    mvwprintw(formWindow, getmaxy(formWindow)-2, (getmaxx(formWindow) - strlen(blank)) / 2, "%s", blank);
                    char errorMessage[MSG_LENGHT];
                    int outcome = edit_record(clientSocket, results[0], submittedEntry, errorMessage);
                    if (outcome < 0){ 
                        display_form_message(formWindow, errorMessage, 1);
                        exit = 1;
                    } else { 
                        display_form_message(formWindow, "Entry succesfully edited", 2);
                        exit = 1;
                    }
                } break;
            default:
                form_driver(myForm, ch);
                break;
        }
        touchwin(formWindow);
        wrefresh(formWindow);
    }
    clean_form_window(&formWindow, &myForm);
    return 1;
}

void build_form_window(WINDOW **w, FORM **f, char *title){
    
    curs_set(1); // Turns the cursor visibility on
    // Create a new window for the form
    // Calculate window dimensions and positions
    int height = 12;                   // Total height of the menu window (including border and title)
    int width = 60;                    // Total width of the menu window
    int starty = (LINES - height) / 2; // Center the window vertically
    int startx = (COLS - width) / 2;   // Center the window horizontally

    *w = newwin(height, width, starty, startx);
    box(*w, 0, 0);
    mvwprintw(*w, 1, (width - strlen(title)) / 2, "%s", title);
    mvwprintw(*w, 3, 1, "%8s:","Name");
    mvwprintw(*w, 4, 1, "%8s:","Address");
    mvwprintw(*w, 5, 1, "%8s:","Phone");

    // Create form fields
    FIELD **fields = malloc(4 * sizeof(FIELD*));
    int gap = 2;
    fields[0] = new_field(1, MAX_FIELD_LEN, 0, gap, 0, 0);
    fields[1] = new_field(1, MAX_FIELD_LEN, 1, gap, 0, 0);
    fields[2] = new_field(1, MAX_FIELD_LEN, 2, gap, 0, 0);
    fields[3] = NULL;

    #if DEBUG
        if (!fields[0] || !fields[1] || !fields[2]) {
            FILE *file = fopen("output.txt", "a");
            fprintf(file, "DEBUG build function\n");
            fprintf(file, "fields are null\n");
            fclose(file);
            return;
        }
    #endif
    // Highlits the fields with an underscore character 
    set_field_back(fields[0], A_UNDERLINE);
    set_field_back(fields[1], A_UNDERLINE);
    set_field_back(fields[2], A_UNDERLINE);


    // Create the form
    *f = new_form(fields);
    set_form_win(*f, *w);

    scale_form(*f, &height, &width);

    WINDOW* formSubWin = derwin(*w, height, width, 3, 9);
    set_form_sub(*f, formSubWin);
    post_form(*f);

    keypad(*w, TRUE);
    wrefresh(*w);
}

void clean_form_window(WINDOW **w, FORM **f){
    
    if(*w == NULL || *f == NULL)
        return;

    // Clean up the form and window
    curs_set(0);
    unpost_form(*f);
    FIELD **fields = form_fields(*f);
    free_form(*f);
    for (int i = 0; i < 4; i++)
        if (fields[i] != NULL)
            free_field(fields[i]);        
    
    if(! (fields == NULL))
        free(fields);
    
    delwin(*w);
}

int show_entries_in_array(dataEntry results[], int totalResults){
    
    clear();
    refresh();
    keypad(stdscr, TRUE);

    // Get screen size
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    // Create a pad large enough to hold all the data entries
    int pad_rows = 5*totalResults; // Adjust the number of rows as needed
    int pad_cols = 64; // Adjust the number of columns as needed
    WINDOW *pad = newpad(pad_rows, pad_cols);

    // Fill the pad with contact details (using the empty contacts array)
    fillPadWithContacts(pad, results, totalResults);

    // Calculate the position to center the pad
    int pad_y = (rows - pad_rows) / 2;
    int pad_x = (cols - pad_cols) / 2;

    // Draw a box around the text
    int box_height = pad_rows + 2;
    int box_width = pad_cols + 2;
    int box_y = pad_y - 1;
    int box_x = pad_x - 1;
    WINDOW *box_win = newwin(box_height, box_width, box_y, box_x);
    // wrefresh(box_win);

    // Display the pad in the center of the screen
    prefresh(pad, 0, 0, pad_y, pad_x, rows - 1, cols - 1);

    // Scroll the pad
    int pad_top = 0; // The current top line of the pad being displayed
    int ch;
    while ((ch = getch()) != KEY_F(1) && ch != KEY_F(2) && ch != 10) {
        switch (ch) {
            case KEY_UP:
                if (pad_top > 0) {
                    pad_top -= 5;
                }
                break;
            case KEY_DOWN:
                if (pad_top < pad_rows - rows) {
                    pad_top += 5;
                }
                break;
        }
        prefresh(pad, pad_top, 0, pad_y, pad_x, rows - 1, cols - 1);
    }


    FILE *fp;
    if(ch == KEY_F(2) && (fp = fopen("searchResults.txt", "w")) != NULL){
        fprintf(fp, "Search results from last search on Yellowpages:\n");
        for(int i = 0; i < totalResults; i++){
            fprintf(fp, " # %d\n", i);
            fprintf(fp, " Name: %s\n", results[i].name);
            fprintf(fp, " Address: %s\n", results[i].address);
            fprintf(fp, " Phone Number: %s\n", results[i].phoneNumber);
            fprintf(fp, " ----------------------------------------\n");
        }
        fclose(fp);
    }

    // Cleanup
    clear();
    refresh();
    delwin(box_win);
    delwin(pad);
    endwin();

    return 1;
};

void fillPadWithContacts(WINDOW *pad, dataEntry results[], int totalResults) {
    for (int i = 0; i < totalResults; i++) {
        wprintw(pad, " # %d\n", i);
        wprintw(pad, " Name: %s\n", results[i].name);
        wprintw(pad, " Address: %s\n", results[i].address);
        wprintw(pad, " Phone Number: %s\n", results[i].phoneNumber);
        wprintw(pad, " ----------------------------------------\n");
    }
}

void terminate_process(int sig) {
    endwin();
    system("stty sane");
    system("clear");
    logout(client_socket, logoutValue);
}

// The parameters don't need to be directly modified so a single level of pointer is enough
void assemble_admin_window(WINDOW *w, MENU* m){
    // load the menu in the window (post)
    post_menu(m);

    // Print a title and border around the menu window  
    box(w, 0, 0);
    char header[] = "YellowPages TUI client";
    init_pair(1, COLOR_YELLOW, COLOR_BLACK); 
    wattron(w, COLOR_PAIR(1));
    mvwprintw(w, 2, (40 - strlen(header)) / 2, "%s", header);
    wattroff(w, COLOR_PAIR(1));
    
    // We print at the bottom of the screen a line with usefull info for the user
    // This is not printed on `adminMenuWindow` but on ncurses `stdscr`
    admin_welcome();

    wrefresh(w); // updates the visuals of `adminMenuWindow`
}

void display_form_message(WINDOW *win, const char *message, int color) {
    
    switch (color) {
        case 1: // Red
            init_pair(3, COLOR_RED, COLOR_BLACK);
            break;
        case 2: // Green
            init_pair(3, COLOR_GREEN, COLOR_BLACK);
            break;
        case 3: // Cyan
            init_pair(3, COLOR_CYAN, COLOR_BLACK);
            break;
    }

    wattron(win, COLOR_PAIR(3));

    int y = getmaxy(win) - 2; // Second to last line of the WINDOW, not stdscr
    int x = (getmaxx(win) - strlen(message)) / 2;

    mvwprintw(win, y, x, "%s", message);

    wattroff(win, COLOR_PAIR(3));

    wrefresh(win);
    getch();
}


void admin_welcome(){
    init_pair(1, COLOR_YELLOW, COLOR_BLACK); 

    clear();
    mvprintw(LINES - 3, 3, "Welcome to ");
    attron(COLOR_PAIR(1));
    printw("Yellowpages ");
    attroff( COLOR_PAIR(1));
    printw("Admin panel.");
    mvprintw(LINES - 2, 3, "Press F1 to go back/exit.");
    refresh();
}
void user_welcome(){
    init_pair(1, COLOR_YELLOW, COLOR_BLACK); 

    clear();
    mvprintw(LINES - 3, 3, "Welcome to ");
    attron(COLOR_PAIR(1));
    printw("Yellowpages ");
    attroff( COLOR_PAIR(1));
    printw("User panel.");
    mvprintw(LINES - 2, 3, "Press F1 to go back/exit.");
    refresh();
}
