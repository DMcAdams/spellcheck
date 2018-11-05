#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"./node.h"

#define DEFUALT_DICTIONARY "./dictionary.txt"
#define HASH_SIZE 2000
#define BUFF_SIZE 49



node *table[HASH_SIZE];


void init();
void add_word(char *s);
int hash(char *s);
void to_lower(char *s);
int count;

int main(int argc, char const *argv[]){
    count = 0;
    init();

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

void add_word(char *s){
    //get hash
    int index = hash(s);
    //create node
    node *n = new_node(s);
    //temp used to find proper place in hashmap
    node *temp = table[index];
    //if position in hashmap is empty
    if (temp == NULL){
        //make node the first in that spot
        temp = n;
    }
    //else add node to beggining of hash table
    else {
        n->next = temp;
        temp = n;
    }
}
//check if word is in dictionary
int check(char *s){
    //convert input to lowercase
    to_lower(s);
    //find place in hash table
    int index = hash(s);
    //loop through section of table
    node *temp = table[index];
    while (temp != NULL){
        //if match found
        if (strcmp(s, temp) == 0){
            return TRUE;
        }
    }
}
int hash(char *s){
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