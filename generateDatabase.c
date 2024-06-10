#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "DatabaseHandler.h"



void generatePhoneNumber(char phoneNumber[]) {
    for (int i = 0; i < 10; ++i) 
        phoneNumber[i] = '0' + rand() % 10; // Genera un numero casuale da 0 a 9 e lo converte in carattere
    phoneNumber[10] = '\0';
}

void generateName(char name[]) {
    const char *names[115] = {
        "Mario",     "Luigi",    "Giovanni", "Antonio",     "Roberto",
        "Francesco", "Giuseppe", "Paolo",    "Andrea",      "Stefano",
        "Maria",     "Anna",     "Giulia",   "Laura",       "Eleonora",
        "Francesca", "Sara",     "Alessia",  "Martina",     "Valentina",
        "John",      "Michael",  "Emily",    "Jessica",     "David",
        "James",     "Linda",    "Sophia",   "Christopher", "Isabella",
        "Alessandro","Luca",     "Marco",    "Matteo",      "Fabio",
        "Claudia",   "Silvia",   "Federica", "Chiara",      "Angela",
        "Simone",    "Daniele",  "Emanuele", "Massimo",     "Vincenzo",
        "Gabriella", "Elena",    "Lorenzo",  "Filippo",     "Tommaso",
        "Rebecca",   "Alice",    "Giorgio",  "Alberto",     "Riccardo",
        "Christian", "Sofia",    "Alexander","Brian",       "Kevin",
        "Lisa",      "Olivia",   "Mia",      "Charlotte",   "Benjamin",
        "Yuki",      "Hiroshi",  "Aiko",     "Akira",       "Sakura",
        "Taro",      "Kenji",    "Satoshi",  "Rina",        "Yumi",
        "Haruto",    "Hinata",   "Mika",     "Takumi",      "Yuna",
        "Wei",       "Lian",     "Mei",      "Liang",       "Zhi",
        "Aarav",     "Vihaan",   "Vivaan",   "Ananya",      "Aadhya",
        "Aarohi",    "Ibrahim",  "Zara",     "Aaliyah",     "Omar",
        "Fatima",    "Zayn",     "Amara",    "Ethan",       "Liam",
        "Amina",     "Chinwe",   "Kwame",    "Zola",        "Nia",
        "Kofi",      "Said",     "Layla",    "Ahmed",       "Hassan",
        "Nia",       "Tariq",    "Adisa",    "Farah",       "Zubair"
    };

    const char *surnames[75] = {
        "Rossi",     "Bianchi",  "Verdi",    "Ferrari",
        "Russo",     "Esposito", "Bianco",   "Colombo",
        "Ricci",     "Marino",   "Romano",   "Greco",
        "Conti",     "Costa",    "Gallo",    "Fontana",
        "Leone",     "Messina",  "Orlando",  "Pellegrini",
        "Rinaldi",   "Santoro",  "Silvestri","Testa",
        "Valente",   "Vitali",   "Williams", "Smith",
        "Johnson",   "Brown",    "Miller",   "Davis",
        "Wilson",    "Moore",    "Taylor",   "Anderson",
        "Thomas",    "Jackson",  "White",    "Harris",
        "Martin",    "Thompson", "Garcia",   "Martinez",
        "Robinson",  "Wang",     "Li",       "Zhang",
        "Liu",       "Chen",     "Yang",     "Huang",
        "Zhao",      "Wu",       "Sharma",   "Patel",
        "Reddy",     "Khan",     "Singh",    "Iyer",
        "Gupta",     "Bedi",     "Okafor",   "Diallo",
        "Ndiaye",    "Abdi",     "Hussein",  "Mohamed",
        "Ali",       "Abbas",    "Sheikh",   "Hassan",
        "Chowdhury", "Ahmed", "Asparagi"
    };


  strcpy(name, names[rand() % 115]);
  strcat(name, " ");
  strcat(name, surnames[rand() % 75]);
}

void generateAddress(char address[]) {

    const char *streets[90] = {
        "Via Roma",                  "Corso Vittorio Emanuele", "Via Dante", 
        "Corso Italia",              "Via Garibaldi",           "Via Leonardo da Vinci", 
        "Via Verdi",                 "Via Manzoni",             "Via Carducci", 
        "Via Veneto",                "Via XX Settembre",        "Via Marconi", 
        "Viale Kennedy",             "Via Amendola",            "Via Fiume", 
        "Via Solferino",             "Via Mazzini",             "Viale Europa", 
        "Via Cavour",                "Corso Matteotti",         "Via Piave", 
        "Viale dei Giardini",        "Via Po",                  "Via Sardegna", 
        "Via Sicilia",               "Corso Garibaldi",         "Via Trento", 
        "Via Trieste",               "Via Nazionale",           "Via della Repubblica", 
        "Via Libertà",               "Via Vittorio Veneto",     "Via della Pace", 
        "Via dell'Indipendenza",     "Via San Giovanni",        "Via San Francesco", 
        "Via Matteotti",             "Via Gramsci",             "Via Di Vittorio", 
        "Via Alfieri",               "Via Boccaccio",           "Via Foscolo", 
        "Via Tasso",                 "Via Leopardi",            "Via Montale", 
        "Via Pascoli",               "Via Petrarca",            "Via Pirandello", 
        "Via Alfieri",               "Via Manzoni",             "Via Verga", 
        "Via Ungaretti",             "Via Saba",                "Via Svevo", 
        "Via Quasimodo",             "Via Carducci",            "Via De Amicis", 
        "Via Deledda",               "Via Morante",             "Via Parini", 
        "Via Ariosto",               "Via D'Annunzio",          "Via Goldoni", 
        "Via Machiavelli",           "Via Alighieri",           "Via Pavese", 
        "Via Tasso",                 "Via Nievo",               "Via Fogazzaro", 
        "Via Gobetti",               "Via Tommaseo",            "Via Capuana", 
        "Via Duse",                  "Via Baracca",             "Via San Martino", 
        "Via Castello",              "Via Torre",               "Via Giardino", 
        "Via Fontana",               "Via delle Rose",          "Via delle Mimose", 
        "Via dei Pini",              "Via degli Olmi",          "Via dei Gelsi", 
        "Via dei Fiori",             "Via delle Querce",        "Via dei Tigli", 
        "Via degli Aceri",           "Via dei Frassini",        "Via degli Abeti",
    };


    const char *provinces[40] = {
        "Milano",     "Roma",       "Napoli",     "Torino",        "Palermo",
        "Genova",     "Bologna",    "Firenze",    "Bari",          "Catania",
        "Venezia",    "Verona",     "Messina",    "Padova",        "Trieste",
        "Brescia",    "Taranto",    "Prato",      "Modena",        "Reggio Calabria",
        "Salerno",    "Parma",      "Ravenna",    "Ferrara",       "Rimini",
        "Pisa",       "Siracusa",   "Livorno",    "Pescara",       "Latina",
        "Monza",      "Como",       "Bergamo",    "Trento",        "Vicenza",
        "Terni",      "Novara",     "Forlì",      "Varese",        "Treviso"
    };

  
    strcpy(address, streets[rand() % 90]);
    int civicNumber = rand() % 120 + 1;
    char numStr[7];
    sprintf(numStr, " %d, ", civicNumber);
    strcat(address, numStr);
    strcat(address, provinces[rand() % 40]);
}

int main(int argc, char *argv[]) {
    srand(time(NULL)); 

    unsigned numEntries = 50;
    if (argc == 2) {
        numEntries = atoi(argv[1]); 
    }

    dataEntry *entries = (dataEntry *)malloc(numEntries * sizeof(dataEntry));
    if (entries == NULL) {
        fprintf(stderr, "Errore di allocazione della memoria\n");
        return 1;
    }

    for (int i = 0; i < numEntries; i++) {
        generateName(entries[i].name);
        generateAddress(entries[i].address);
        generatePhoneNumber(entries[i].phoneNumber);
    }

    FILE * db = open_db_write(NULL);
    for(int i = 0; i < numEntries; i++)
        fwrite(&entries[i], sizeof(dataEntry), 1, db);
    fclose(db);

    if(numEntries>100){
        printf("%d database entries have been generated.\n", numEntries);
        return 0;
    }

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
