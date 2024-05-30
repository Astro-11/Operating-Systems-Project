TODO:
- [ ] Completare il TODO con tutte le necessità ed obbiettivi
- [ ] Expand the documentation with expected use case of procedures and parameters.
- [X] Make enums to clearly label server modes options
- [X] Salvataggio DB su disco...
    - [X] ...quando si chiude un client loggato con exit.
    - [x] ...quando viene terminato il server con ctrl+C.
- [ ] Validation for database entries addition and client queries
    - [x] All three fields should be present
    - [X] Name is alphanum and trim left and right
    - [X] Phone number is num, has ten characters, allows indiscriminate white space when added or searched, is stored with no space for more uniform searching but is printed with a `"012 345 6789"` formatting. 
    - [ ] Prevent duplicated entries
- [ ] Update all methods to use snake_case and all variables to use camelCase
- [X] Update clients' internal entries to mantain database consistency after superuser commits changes to db
- [X] Login and logout
    - [X] Implement server-side login authentification 
    - [X] Add logout options for clients to close application
    - [X] Add SIGINT handling for client: it should inform the server of its premature demise
    - [ ] Use args when booting up client to decide wether to ask for admin permissions or not
- [X] Inter Process Communication inside of server
    - [X] Admin server should inform main server of new changes to the db and main server should inform user servers
    - [X] User servers and admin server should have their own control groups for signals, main server will keep a reference to the two control groups


QUESTIONS:
- [ ] What do we want to do with non ASCII characters like 'ò','è' and UNICODE symbols?
    - Do we allow them because Niccolò and such are legal names?
    - If we ignore them completely we may have unewxpected problems, but what kind of problems?
    - If we handle them we do it client side or server side?
    - We need to test the behaviour of our code in this cases 
- [ ] Do we need/want to handle overfilling the array at runtime?
- [ ] Should we rename the "dataEntry" struct to "record"? It seems to me like you find the term confusing for things like entries[].


---
Siti utili per ncurses:
- [HOWTO](https://tldp.org/HOWTO/NCURSES-Programming-HOWTO/index.html): Guida introduttiva che parla anche delle librerie sorelle
- [Official man pages](https://invisible-island.net/ncurses/man/)
- [Lista di esempi da un libro](https://github.com/wkoszek/ncurses_guide/tree/master/book)
