## SocketClientTest:

    void add_new_record(int clientSocket);
        Richiede al server l'aggiunta di un nuovo record.

    void delete_record(int clientSocket);
        Richiede al server la rimozione di un record.

    void receive_entries(int clientSocket);
        Riceve dal server un certo numero di record.
        Riceve prima un segnale contenente il numero di record e poi molteplici messaggi contenenti le strutture dati.


## SocketServerTest:

    void add_new_record(int clientSocket);
        Gestisce le richieste di aggiunta di un nuovo record al database.
    
    void search_record_procedure(int clientSocket, dataEntry entries[], int entriesCount);
        Gestisce le richieste di ricerca di record nel database.

    void delete_record_procedure(int clientSocket, dataEntry entries[], int entriesCount);
        Gestisce le richieste di rimozione di un record dal database.

    int delete_record(dataEntry entries[], int entriesCount, dataEntry entryToDelete);
        Esegue la rimozione del record desiderato (attualmente solo dalla memoria del server).
        Restituisce 0 in caso di successo, -1 qualora la query non sia ben formattata, -2 qualora il record non esista

    int search_records(dataEntry entries[], int entriesCount, dataEntry query, dataEntry queryResults[]);
        Ricerca nella memoria del server uno o più record che rispettino la query specificata.
        Restituisce il numero di record che rispettano la query.

    int search_record_position(dataEntry entries[], int entriesCount, dataEntry query);
        Ricerca la posizione nell'array fornito di un determinato record.
        NB: restituisce sempre il primo risultato che rispetta la query, assumendo che non esistano duplicati e che la query contenga tutti e 3 i campi
        Restituisce 0 in caso di successo, -1 in caso contrario. 

    int matches(dataEntry entry, dataEntry filter);

    void print_all_entries();
        Stampa a schermo tutti i record contenuti nel database.

    void send_entries(int clientSocket, dataEntry entries[], int entriesCount);
        Invia la desiderata collezione di record al client specificato.


## DatabaseHandler:

    void debug_populate_db();
        Funzione di debug, popola il database con 30 record prestabiliti.

    FILE * open_db_read();
        Apre il database in funzione di lettura.
        Restituisce l'indirizzo del file contenente il db.

    FILE * open_db_write();
        Apre il database in funzione di scrittura.
        Restituisce l'indirizzo del file contenente il db.

    void close_db(FILE *filePointer);
        Chiude il database.

    void writeEntry(dataEntry dataEntry, FILE *filePointer);
        Scrive un record nel database.
        (Attualmente non esegue alcun controllo)

    void readEntry(FILE *filePointer, dataEntry * readEntry);
        Legge un singolo record del database a partire dal puntatore interno del file che gli viene passato.

    int countEntries(FILE *filePointer, int sizeOfEntry);
        Conta il numero di record presenti nel database.

    int readEntries(FILE *filePointer, dataEntry dataEntries[]);
        Legge tutti i record presenti nel database.
        Restituisce il numero di record letti.

    int remove_all_whitespace(char str[]);
        Pulisce una stringa rimuovendo ogni tipo di spazio (' ' , '\n' , '\t' etc.)
        Restituisce la lunghezza della stringa dopo la procedura.

    int validate_entry(dataEntry entry);
        Verifica la validità di un record.
        Attualmente si limita a controllare che i tre campi siano non nulli e non composti solo da spazi.
        Restituisce 0 in caso di esito positivo, -1 altrimenti.

    char* rtrim(char *str);

