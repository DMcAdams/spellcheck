//standard libraries
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
//included header files
#include"./node.h"
#include"./client.h"
//for threads
#include <pthread.h>
//for networking
#include <sys/socket.h>
#include <netinet/in.h>

#define DEFUALT_DICTIONARY "./dictionary.txt"
#define HASH_SIZE 2000
#define BUFF_SIZE 49
#define FALSE 0
#define TRUE 1




//for dictionary functions
void init();
int hash(char *s);
void add_word(char *s);
void to_lower(char *s);
int check(char *s);
//for networking
int open_listenfd(int port);
void connection_handler(int argc, char **argv);
void addClient(int clientSocket);

//hash table for holding dictionary
node *table[HASH_SIZE];
//queue for holding incoming clients
queue *clientQueue;
//for synchronization
pthread_mutex_t mutex;
pthread_cond_t work;
int count;

int main(int argc, char const *argv[]){
    count = 0;
    init();
    char s[] = "helwpdplqwldoqwk[oqkwpk";
    //to_lower(s);
    //add_word(s);

    //char s2[] = "dandjwnwjdnjawldbawlj";
    printf("%s:%d\n", s, check(s));

}

void init(){
    //initialize hash table values to null
    for (int i = 0; i < HASH_SIZE; i ++){
        table[i] = NULL;
    }
    //open dictionary text file
    FILE *dictionary = fopen(DEFUALT_DICTIONARY, "r");
    //if file not found
    if (dictionary == NULL){
        //error message
        puts("ERROR: File could not be opened");
        //end
        exit(1);
    }
    //buffer holds words from dictionary
    char *buffer = malloc(sizeof(char)*BUFF_SIZE);
    //while words in dictionary
    while (fscanf(dictionary, "%s\n", buffer) > 0){
        //make word lowercase
        to_lower(buffer);
        //add node to table
        add_word(buffer);
    }

}

int hash(char *s){
    //convert to lower case
    to_lower(s);
    //used to read each char in string
    char *temp = s;
    //holds sum of all chars
    int sum = 0;
    //loop through string
    while (*temp != '\0'){
        //add value of current char to string
        sum += *temp;
        //move to next char
        temp++;
        //test message
    }
    //sum mod hash is the final hash.
    return sum % (HASH_SIZE);
}

void add_word(char *s){
    //get hash
    int index = hash(s);
    //create node
    node *n = new_node(s);
    //if position in hashmap is empty
    if (table[index] == NULL){
        //make node the first in that spot
        table[index] = n;
    }
    //else add node to beggining of hash table
    else {
        n->next = table[index];
        table[index] = n;
    }
}

//converts string to lowercase
void to_lower(char *s){
    //used to travel string
    char *temp = s;
    //loop until end of string
    while(*temp != '\0'){
        //if char is uppercase
        if (*temp >= 'A' && *temp <= 'Z'){
            //make it lowercase
            *temp += 32;
        }
        temp++;
    }
}

//check if word is in dictionary
int check(char *s){
    //convert input to lowercase
    to_lower(s);
    //find place in hash table
    int index = hash(s);
    printf("HASH: %d\n", index);

    //loop through section of table
    node *temp = table[index];
    do {
        printf("%s:%s\n", s, temp->word);
        //if match found
        if (strcmp(s, temp->word) == 0){
            return TRUE;
        }
        //else go to next node
        temp = temp->next;
    }
    while(temp->word != NULL && temp->next != NULL);
    
    //if no match found
    return FALSE;
}

int open_listenfd(int port){
    int listenfd, optval=1;
    struct sockaddr_in serveraddr;
    /* Create a socket descriptor */
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    }
    /* Eliminates "Address already in use" error from bind */
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int)) < 0){
        return -1;
    }
    //Reset the serveraddr struct, setting all of it's bytes to zero.
    //Some properties are then set for the struct, you don't
    // need to worry about these.
    //bind() is then called, associating the port number with the
    //socket descriptor.
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)port);
    if (bind(listenfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0){
        return -1;
    }
    //Prepare the socket to allow accept() calls. The value 20 is
    //the backlog, this is the maximum number of connections that will be placed
    //on queue until accept() is called again.
    if (listen(listenfd, 20) < 0){
        return -1;
    }
    return listenfd;
}

//Handles all incoming connections, adds new clients to the client queue
void connection_handler(int argc, char **argv) {
    //if not enough inputs
    if (argc < 2) {
        puts("Error: No port number entered.");
        exit(0);
    }
        //else if too many inputs
    else if (argc > 2) {
        puts("Error: Too many inputs");
        exit(0);
    }

    //get port number from argv
    int port = atoi(argv[1]);
    //only needed for accept()
    struct sockaddr_in client;
    //needed for socket connection
    socklen_t clientLen = sizeof(client);
    int connectionSocket;
    int clientSocket;

    //can't use ports below 1025 and ports above 65535
    if (port < 1025 || port > 65535) {
        puts("Error: port number must be greater than 1024 and less than 65536");
        exit(0);
    }

    //connect to port
    connectionSocket = open_listenfd(port);
    //if connect failed
    if (connectionSocket == -1) {
        printf("Error: Could not connect to port %d\n", port);
        exit(0);
    }
    //start accepting clients
    while (1) {
        //get client connection
        clientSocket = accept(connectionSocket, (struct sockaddr *) &client, &clientLen);
        //if connect failed
        if (clientSocket == -1) {
            printf("Error: Could not connnect to client.\n");
            exit(0);
        }
            //else add client to clientQueue
        else {
            addClient(clientSocket);
        }
    }
}

//adds clientSocket data into queue,
void addClient(int clientSocket){
    //get lock
    pthread_mutex_lock(&mutex);
    //add clientSocket to queue
    push(clientQueue, clientSocket);
    //signal to workers
    pthread_cond_signal(&work);
    //release lock
    pthread_mutex_unlock(&mutex);
}