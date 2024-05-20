TODO:
- [ ] Completare il TODO con tutte le necessità ed obbiettivi
- [ ] Expand the documentation with expected use case of procedures and parameters.
- [ ] Make enums to clearly label server modes options
- [ ] Salvataggio DB su disco...
    - [ ] ...quando si chiude un client loggato con exit.
    - [x] ...quando viene terminato il server con ctrl+C.
        - Needs testing, but it's committed.
- [ ] Validation for database entries addition and client queries
    - [ ] Name is alphanum and trim left and right
    - [ ] Phone number is num, has ten characters, allows indiscriminate white space when added or searched, is stored with no space for more uniform searching but is printed with a `"012 345 6789"` formatting. 

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
