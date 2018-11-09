#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include <unistd.h>
//included header files
#include"./node.h"
#include"./client.h"
//for threads
#include <pthread.h>
//for networking
#include <sys/socket.h>
#include <netinet/in.h>
//file names
#define DEFUALT_DICTIONARY "./dictionary.txt"
#define LOG_FILE "./log.txt"
//size of hash table
#define HASH_SIZE 2000
//max characters for word buffer
#define BUFF_SIZE 49
//max buffer for client input
#define CLIENT_BUFF 100
//number of worker threads
#define WORKER_COUNT 3
//true/false
#define FALSE 0
#define TRUE 1




//for dictionary functions
void init();
int hash(char *s);
void add_word(char *s);
void to_lower(char *s);
char* trim(char *s);
int check(char *s);
void write_log(char *string, pthread_t threadID);
//for networking
int open_listenfd(int port);
void connection_handler(int argc, char **argv);
void addClient(int clientSocket);
//worker threads to handle requests
client *getClient();
void *workerThread(void *id);

//hash table for holding dictionary
node *table[HASH_SIZE];
//queue for holding incoming clients
queue *clientQueue;
//for synchronization
pthread_mutex_t mutex;
pthread_cond_t work;
int count;

int main(int argc, char **argv){
    //start message in log file
    write_log("**********SERVER START*************\n", 0);
    //initialize dictionary
    init();
    //creat the client queue
    clientQueue = create();
    //create worker thread pool
    pthread_t threadPool[WORKER_COUNT];
    int threadIDs[WORKER_COUNT];
    //init worker threads
    printf("%s\n","Creating threads");
    for(int i = 0; i < WORKER_COUNT; i++){
        threadIDs[i] = i;
        //Start running the threads.
        pthread_create(&threadPool[i], NULL, &workerThread, &threadIDs[i]);
    }

    printf("All threads launched.\n");
    connection_handler(argc, argv);

    for(int i = 0; i < WORKER_COUNT; i++){
        //Wait for all threads to finish executing.
        pthread_join(threadPool[i], NULL);
    }
}

//write string to log file
void write_log(char *string, pthread_t threadID){
    FILE *fp = fopen(LOG_FILE, "a");
    if (fp == NULL){
        puts("Could not write to log file.");
    }
    else{
        fprintf(fp, "Thread %lu: %s", threadID, string);
    }
    fclose(fp);
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

//remove trailing newline char from string
char* trim(char *s){
    int i = strlen(s) - 1;
    if (i > 0 && s[i] == '\n')
        s[i] = '\0';
    return s;
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

client *getClient(){
    //get lock
    pthread_mutex_lock(&mutex);
    //if queue is empty
    while(clientQueue->count <= 0){
        //wait for signal for work
        pthread_cond_wait(&work, &mutex);
    }
    //get node from clientQueue
    client *temp = pull(clientQueue);
    //release lock
    pthread_mutex_unlock(&mutex);

    return temp;
}
void *workerThread(void *id){
    //get thread ID
    pthread_t threadID = pthread_self();
    //buffer gets input from client
    char recvBuffer[CLIENT_BUFF];
    int bytesReturned;
    //used to check individual words
    char *token;

    while (1) {
        //get next client
        client *temp = getClient();
        int clientSocket = temp->ClientSocket;

        //used for messages
        char* clientMessage = "Now connected to the spellcheck server.\n";
        char* msgRequest = "Send me text to check if a word or words are in the dictionary.\nUse the escape key to exit.\n";
        char* wordFound = " is in the dictionary.\n";
        char *wordNotFound = " is not in the dictionary.\n";
        char* msgPrompt = ">>>";
        char* inputReceived = "Input from client: ";
        char* msgError = "Please input text for the spellchecker.\n";
        char* msgClose = "Goodbye!\n";

        //print and write connected message to log
        printf("Thread %lu: New client connected\n", threadID);
        write_log("New client connected\n", threadID);

        //Send connected message to client
        send(clientSocket, clientMessage, strlen(clientMessage), 0);
        //Ask for input
        send(clientSocket, msgRequest, strlen(msgRequest), 0);
        
        //Begin sending and receiving messages.
        while(1){
            //send promt (">>>") to the client
            send(clientSocket, msgPrompt, strlen(msgPrompt), 0);
            //recv() will store the message from the user in the buffer, returning
            //how many bytes we received.
            bytesReturned = recv(clientSocket, recvBuffer, CLIENT_BUFF, 0);
            //add nul char to string in place of trailing newline char
            recvBuffer[bytesReturned-(sizeof(char)*2)] = '\0';
            //Check if we got a message, send a message back or quit if the
            //user specified it.
            if(bytesReturned == -1){
                send(clientSocket, msgError, strlen(msgError), 0);
            }
            //'27' is the escape key.
            else if(recvBuffer[0] == 27){
                send(clientSocket, msgClose, strlen(msgClose), 0);
                close(clientSocket);
                break;
            }
                //else check words in string
            else{
                //remove trailing newline char
                char *string = trim(recvBuffer);

                //create an input recieved message
                char *s = malloc(sizeof(char)*(strlen(inputReceived)+strlen(string)+2));
                sprintf(s, "%s%s\n", inputReceived, string);

                //print and log message
                printf("Thread %lu: %s", threadID, s);
                write_log(s, threadID);
                //free memory
                free(s);

                //get first word in string
                token = strtok(string, " ");
                //loop until end of string
                while( token != NULL ) {
                    //if word found
                    if(check(token)){
                        //create found message
                        char *msg = malloc(sizeof(char)*(strlen(token)+strlen(wordFound)+1));
                        sprintf(msg, "%s%s", token, wordFound);
                        //send message
                        send(clientSocket, msg, strlen(msg), 0);
                        //print and write message to log
                        printf("Thread %lu: %s",threadID, msg);
                        write_log(msg, threadID);
                        //free memory
                        free(msg);
                    }
                        //else word not found
                    else{
                        //create not found message
                        char *msg = malloc(sizeof(char)*(strlen(token)+strlen(wordNotFound)+1));
                        sprintf(msg, "%s%s", token, wordNotFound);
                        //send message
                        send(clientSocket, msg, strlen(msg), 0);
                        //print and write message to log
                        printf("Thread %lu: %s",threadID, msg);
                        write_log(msg, threadID);
                        //free memory
                        free(msg);

                    }
                    token = strtok(NULL, " ");
                }
            }
        }
        //free memory
        free(temp);
    }
}