#include "CommonUtils.h"

void print_data_entry(dataEntry entry);
int remove_all_whitespace(char str[]);
int remove_extra_whitespace(char str[]);
char* rtrim(char *str);

void print_data_entry(dataEntry entry) {
    printf("Name_________: %s\n", entry.name);
    printf("Address______: %s\n", entry.address);
    printf("Phone Number_: %s\n", entry.phoneNumber);
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