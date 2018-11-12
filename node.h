#ifndef _NODE_H
#define _NODE_H

#include<stdlib.h>
#include<stdio.h>
#include<string.h>

typedef struct Node {
    struct Node *next;
    char *word;   
} node;

node *new_node(char *s){
    node *n = malloc(sizeof(node));
    n->next = NULL;
    n->word = malloc(sizeof(char)*(1+strlen(s)));
    strcpy(n->word, s);
    return n;
}
#endif  