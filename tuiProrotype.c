
#include <curses.h>
#include <ncurses.h>
#include <menu.h>
#include <stdlib.h>
#include <string.h>
#include <form.h>
#include <stdio.h>

#define MENU_CHOICES 5
#define MAX_FIELD_LEN 30

int displayFormWindow();

int main() {

    ITEM **my_items;
    MENU *my_menu;
    WINDOW *my_menu_win;
    int choice;
    int i;

    char *choices[MENU_CHOICES] = {
        "Search entries",
        "Add new entry",
        "Log in",
        "Info",
        "Exit"
    };

    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    start_color();
    
    init_pair(1, COLOR_YELLOW, COLOR_BLACK);

    keypad(stdscr, TRUE);

    // Create items for the menu
    my_items = (ITEM **)calloc(MENU_CHOICES + 1, sizeof(ITEM *));
    for (i = 0; i < MENU_CHOICES; ++i) {
        my_items[i] = new_item(choices[i], "");
        set_item_userptr(my_items[i], choices[i]); // Set user pointer to the choice string
    }
    my_items[MENU_CHOICES] = (ITEM *)NULL;

    // Create the menu
    my_menu = new_menu(my_items);

    // Calculate window dimensions and positions
    int height = MENU_CHOICES + 4 + 6; // Total height of the menu window (including border and title)
    int width = 40;                    // Total width of the menu window
    int starty = (LINES - height) / 2; // Center the window vertically
    int startx = (COLS - width) / 2;   // Center the window horizontally

    // Create the main menu window
    my_menu_win = newwin(height, width, starty, startx);
    keypad(my_menu_win, TRUE);

    // Create a sub-window for the menu items with padding
    int xOffset = width / 3 - 1;
    int yOffset = height / 3 + 1;
    WINDOW *menu_subwin = derwin(my_menu_win, MENU_CHOICES, width - 2 - xOffset, yOffset, xOffset);
    set_menu_win(my_menu, my_menu_win);
    set_menu_sub(my_menu, menu_subwin);

    // Set menu format and mark
    set_menu_format(my_menu, MENU_CHOICES, 1);
    set_menu_mark(my_menu, "");

    // Print a border around the menu window with title
    box(my_menu_win, 0, 0);
    char header[] = "YellowPages TUI client";
    wattron(my_menu_win, COLOR_PAIR(1));
    mvwprintw(my_menu_win, 2, (width - strlen(header)) / 2, "%s", header);
    wattroff(my_menu_win, COLOR_PAIR(1));
    // Post the menu
    post_menu(my_menu);
    mvprintw(LINES - 2, 3, "Press F1 to exit the program.");
    refresh();
    wrefresh(my_menu_win);

    // Loop for menu selection
    while ((choice = wgetch(my_menu_win)) != KEY_F(1)) {
        
        switch (choice) {
            case KEY_DOWN:
                menu_driver(my_menu, REQ_DOWN_ITEM);
                break;
            case KEY_UP:
                menu_driver(my_menu, REQ_UP_ITEM);
                break;
            case 10: // Enter key
                {
                    ITEM *cur_item = current_item(my_menu);
                    char *selected_choice = (char *)item_userptr(cur_item);

                    if (strcmp(selected_choice, "Exit") == 0) {
                        unpost_menu(my_menu);
                        free_menu(my_menu);
                        for (i = 0; i < MENU_CHOICES; ++i)
                            free_item(my_items[i]);
                        endwin();
                        system("stty sane");
                        system("clear");
                        return 0;
                    } else if (strcmp(selected_choice, "Log in") == 0){
                        /* Change log in to log out and toggle Add new entry */
                        move(LINES - 4, 0);
                        clrtoeol();
                        refresh();
                    }
                    else if (strcmp(selected_choice, "Add new entry") == 0) {
                        // Open the form window
                        clear();
                        mvprintw(LINES - 2, 3, "Press F1 to exit the program.");
                        refresh();
                        /* wclear(my_menu_win);
                        wrefresh(my_menu_win); */
                        unpost_menu(my_menu);
                        if (displayFormWindow() == 1) {
                            // Successful insertion, return to main menu
                             // Clear screen before redrawing menu
                            clear();
                            refresh();  
                        } else {
                            // Insertion cancelled or failed, exit program
                            system("stty sane");
                            system("clear");
                            return 1;
                        }
                    } else {
                        // Handle other menu choices here
                        // wrefresh(my_menu_win);
                        move(LINES - 4, 3);
                        clrtoeol();
                        printw("You selected: %s", selected_choice);
                        refresh();
                    }
                }
                break;
        }
        
        mvprintw(LINES - 2, 3, "Press F1 to exit the program.");
        refresh();
        box(my_menu_win, 0, 0);
        char header[] = "YellowPages TUI client";
        wattron(my_menu_win, COLOR_PAIR(1));
        mvwprintw(my_menu_win, 2, (width - strlen(header)) / 2, "%s", header);
        wattroff(my_menu_win, COLOR_PAIR(1));
        touchwin(my_menu_win);
        post_menu(my_menu); 
        wrefresh(my_menu_win);
    }

    system("stty sane");
    system("clear");
    
    // Clean up
    unpost_menu(my_menu);

    if(my_menu != NULL){
        for (i = 0; i < item_count(my_menu); ++i)
            free_item(menu_items(my_menu)[i]);
        free_menu(my_menu);
        endwin();
    }
    
    return 0;
}

// Function to display a window with a form and save input to a file
int displayFormWindow() {

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
    mvwprintw(form_win, 4, 1, "%8s","Phone: ");
    mvwprintw(form_win, 5, 1, "%8s","Email: ");

    // Create form fields
    FIELD *fields[4];
    int gap = 2;
    fields[0] = new_field(1, MAX_FIELD_LEN, 0, gap, 0, 0);
    fields[1] = new_field(1, MAX_FIELD_LEN, 1, gap, 0, 0);
    fields[2] = new_field(1, MAX_FIELD_LEN, 2, gap, 0, 0);
    fields[3] = NULL;

    // Set field options
    set_field_back(fields[0], A_UNDERLINE);
    set_field_back(fields[1], A_UNDERLINE);
    set_field_back(fields[2], A_UNDERLINE);

    field_opts_off(fields[0], O_AUTOSKIP);
    field_opts_off(fields[1], O_AUTOSKIP);
    field_opts_off(fields[2], O_AUTOSKIP);


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
                char *name = field_buffer(fields[0], 0);
                char *phone = field_buffer(fields[1], 0);
                char *email = field_buffer(fields[2], 0);

                // Save input to file
                FILE *fp = fopen("userinfo.txt", "a");
                if (fp != NULL) {
                    fprintf(fp, "Name : %s\n", name);
                    fprintf(fp, "Phone: %s\n", phone);
                    fprintf(fp, "Email: %s\n", email);
                    fprintf(fp, "----------------\n");
                    fclose(fp);
                    
                    init_pair(2, COLOR_GREEN, COLOR_BLACK);
                    wattron(form_win, COLOR_PAIR(2));
                    char footer[] = "Data saved to 'userinfo.txt'";
                    mvwprintw(form_win, getmaxy(form_win)-2, (getmaxx(form_win) - strlen(footer)) / 2, "%s", footer);
                    wattroff(form_win, COLOR_PAIR(2));

                    wrefresh(form_win);
                    
                    curs_set(0);
                    getch(); // Wait for user input to exit
                }

                // Clean up the form and window
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
