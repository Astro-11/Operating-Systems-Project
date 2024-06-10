#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "DatabaseHandler.h"

#define PHONE_LENGTH 11



void generatePhoneNumber(char phoneNumber[]) {
    for (int i = 0; i < 10; ++i) 
        phoneNumber[i] = '0' + rand() % 10; // Genera un numero casuale da 0 a 9 e lo converte in carattere
    phoneNumber[10] = '\0';
}

void generateName(char name[]) {
    const char *names[20] = {"Mario",   "Luigi",     "Giovanni", "Antonio",
                             "Roberto", "Francesco", "Giuseppe", "Paolo",
                             "Andrea",  "Stefano",   "Maria",    "Anna",
                             "Giulia",  "Laura",     "Eleonora", "Francesca",
                             "Sara",    "Alessia",   "Martina",  "Valentina"};
    const char *surnames[15] = {"Rossi", "Bianchi",  "Verdi",  "Ferrari",
                                "Russo", "Esposito", "Bianco", "Colombo",
                                "Ricci", "Marino",   "Romano", "Greco",
                                "Conti", "Costa",    "Gallo"};

    strcpy(name, names[rand() % 20]);
    strcat(name, " ");
    strcat(name, surnames[rand() % 15]);
}

void generateAddress(char address[]) {
    // Non so perchè il formatter faccia in modo diverso per i due array
    const char *streets[30] = {"Via Roma",
                               "Corso Vittorio Emanuele",
                               "Via Milano",
                               "Via Dante",
                               "Corso Italia",
                               "Via Garibaldi",
                               "Via Leonardo da Vinci",
                               "Via Verdi",
                               "Via Manzoni",
                               "Via Carducci",
                               "Via Veneto",
                               "Via XX Settembre",
                               "Via Marconi",
                               "Viale Kennedy",
                               "Via Amendola",
                               "Via Fiume",
                               "Via Solferino",
                               "Via Mazzini",
                               "Viale Europa",
                               "Via Cavour",
                               "Corso Matteotti",
                               "Via Piave",
                               "Viale dei Giardini",
                               "Via Po",
                               "Via Sardegna",
                               "Via Sicilia",
                               "Via Napoli",
                               "Corso Garibaldi",
                               "Via Trento",
                               "Via Trieste"};
  
    const char *provinces[20] = {
        "Milano",  "Roma",    "Napoli",  "Torino", "Palermo",
        "Genova",  "Bologna", "Firenze", "Bari",   "Catania",
        "Venezia", "Verona",  "Messina", "Padova", "Trieste",
        "Brescia", "Taranto", "Prato",   "Modena", "Reggio Calabria"};
  
    strcpy(address, streets[rand() % 30]);
    int civicNumber = rand() % 120 + 1;
    char numStr[5];
    sprintf(numStr, "%d, ", civicNumber);
    strcat(address, numStr);
    strcat(address, provinces[rand() % 20]);
}

int main(int argc, char *argv[]) {
    // Inizializza il generatore di numeri casuali con il tempo attuale
    srand(time(NULL)); 

    int numEntries = 50; // Imposta un valore predefinito per il numero di voci
    if (argc == 2) {
        numEntries = atoi(argv[1]); // Se viene passato un argomento da linea di comando, usa quello come numero di voci
    }

    dataEntry entries[numEntries];
    for (int i = 0; i < numEntries; i++) {
        generateName(entries[i].name);
        generateAddress(entries[i].address);
        generatePhoneNumber(entries[i].phoneNumber);
    }

    FILE * db = open_db_write(NULL);
    for(int i = 0; i < numEntries; i++)
        fwrite(&entries[i], sizeof(dataEntry), 1, db);
    

    fclose(db);

    printf("Elenco del database:\n");
    for (int i = 0; i < numEntries; i++) {
        printf(" # %3d:\n", i);
        printf("Name    : %s\n", entries[i].name);
        printf("Address : %s\n", entries[i].address);
        printf("Phone   : %s\n", entries[i].phoneNumber);
        printf("--------------------------------\n");
    }

    return 0;
}
