struct message{
    int type;
    int* content;
    char* description;
}
struct express{
    int id;
    char expresschar;
    //tell is alive. 0 dead,1 alive;
    int alive;
}