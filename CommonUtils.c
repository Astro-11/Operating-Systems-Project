#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "CommonUtils.h"

void print_data_entry(dataEntry entry);
void print_all_entries(dataEntry entries[], int entriesCount);

int matches(dataEntry entry, dataEntry filter);

int remove_all_whitespace(char str[]);
int remove_extra_whitespace(char str[]);
char* rtrim(char *str);
char* uppercase_to_lowercase(char* str);

void print_data_entry(dataEntry entry) {
    printf("Name_________: %s\n", entry.name);
    printf("Address______: %s\n", entry.address);
    printf("Phone Number_: %s\n", entry.phoneNumber);
}

void print_all_entries(dataEntry entries[], int entriesCount) {
    int i = 0;
    while (i < entriesCount)
        print_data_entry(entries[i++]);
}

//Returns 0 if matches, -1 otherwise
int matches(dataEntry entry, dataEntry filter) {
    if (strlen(filter.name) > 0) {
        remove_extra_whitespace(filter.name);
        if (strstr(uppercase_to_lowercase(entry.name), uppercase_to_lowercase(filter.name)) == NULL)
            return -1;
    }
    if (strlen(filter.address) > 0) {
        remove_extra_whitespace(filter.address);
        if (strstr(uppercase_to_lowercase(entry.address), uppercase_to_lowercase(filter.address)) == NULL)
            return -1;
    }
    if (strlen(filter.phoneNumber) > 0) {
        remove_extra_whitespace(filter.phoneNumber);
        if (strstr(uppercase_to_lowercase(entry.phoneNumber), uppercase_to_lowercase(filter.phoneNumber)) == NULL)
            return -1;
    }
    return 0;
}

//Removes whitespaces and /n
//Returns lenght of string after cleaning
int remove_all_whitespace(char str[]) {
    int i = 0; 
    int len = 0;
    while (str[i] != '\0'){
        if (!isspace(str[i])) 
            str[len++] = str[i];
        i++;
        }
    
    str[len] = '\0';

    return len;
}

int remove_extra_whitespace(char str[]) {

    int len = strlen(str);
    int start = 0;
    int end = len - 1;
    int firstSpace = 1; // keeps track if we have already encountered a space or not
    
    while (isspace(str[start]) && start <= end)
        start++;
    
    if(start > end){
        str[0] = '\0';
        return 0;
    }

    while (end >= start && isspace(str[end]))
        end--;

    // Reduce multiple spaces in between and copy to the start of the string
    int j = 0;
    for (int i = start; i <= end; i++) {
        if (isspace(str[i])) {
            if (firstSpace) {
                str[j++] = ' ';
                firstSpace = 0;
            }
        } else {
            str[j++] = str[i];
            firstSpace = 1;
        }
    }
    
    str[j] = '\0';
    return j;
}

char* rtrim(char *str) {
    int len = strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1])) {
        len--;
    }
    str[len] = '\0';
    return str;
}

char* uppercase_to_lowercase(char* str) {
    int len = strlen(str);
    while (len >= 0 ) {
        str[len] = tolower(str[len]);
        len--;
    }
    return str;
}



