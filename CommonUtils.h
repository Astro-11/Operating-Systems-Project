#include "CommonDefines.h"

#ifndef COMMON_UTILS_H
#define COMMON_UTILS_H

void print_data_entry(dataEntry entry);
void print_all_entries(dataEntry entries[], int entriesCount);

int matches(dataEntry entry, dataEntry filter);

int remove_all_whitespace(char str[]);
int remove_extra_whitespace(char str[]);
char* rtrim(char *str);
char* uppercase_to_lowercase(char* str);

#endif