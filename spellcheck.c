#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"./node.h"
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


node *table[HASH_SIZE];


void init();
int hash(char *s);
void add_word(char *s);
void to_lower(char *s);
int check(char *s);
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