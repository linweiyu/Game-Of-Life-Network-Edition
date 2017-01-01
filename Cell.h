 #include<curses.h>
 #define CharacterNumber 4
 #define ClientNumber 256
struct message{
    int type;
    int content[35][120];
    char description[1024];
};

struct express{
    // character list index
    int index;
    int valid;
};

struct clientsocket{
    int socketid;
    int index;
    char expresschar;
    int valid;
};
