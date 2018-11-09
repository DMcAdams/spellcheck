#ifndef CLIENT_H
#define CLIENT_H
#include<stdio.h>
#include<stdlib.h>

//the client struct
typedef struct Client{
    int ClientSocket;
    struct Client *next;
} client;

typedef struct Queue {
    //holds first and last client in queue
    client *front, *back;
    //holds number of nod
    int count;
} queue;

//create a new queue
queue* create() {
    queue *q = (queue *)malloc(sizeof(queue));
    q->front = NULL;
    q->count = 0;
    return q;
}

//creates a new client and returns it
client* new_client(int ClientSocket){
    client *temp = (client *)malloc(sizeof(client));
    temp->ClientSocket = ClientSocket;
    temp->next = NULL;
    return temp;
}

//adds a new client to the back of the queue.
void push (queue *q, int ClientSocket){
    //create new node
    client *temp = new_client(ClientSocket);

    //if queue is empty, set front and back to the new node
    if (q->front == NULL){
        q->front = temp;
        q->back = q->front;
    }
    //else add node to back of queue
    else {
        q->back->next = temp;
        q->back = temp;
    }
    //add one to node count
    q->count++;
}
//remove next client from the queue and return it
client* pull(queue *q){
    //if queue is empty
    if (q->count <= 0){
        return NULL;
    }
    //else get next client
    else{
        //save next client
        client *temp = q->front;
        //move to next node
        q->front = q->front->next;
        //if front is NULL, make sure back is also NULL
        if(q->front == NULL){
            q->back = NULL;
        }
        //subtract from queue count
        q->count--;
        //return the node
        return temp;
    }
}

#endif