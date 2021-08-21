#ifndef _HASHTABLE_H
#define _HASHTABLE_H

#include "utilities.h"

#define THRESHOLD 0.7
#define DEBUG_HT (0)

typedef struct Item{
    char* key;
    void* data;
    uint32_t items;
    struct Item* next;
}Item;

typedef struct Buckets{
    Item** item;
    size_t capacity;
    size_t items;
}Buckets;

void HT_CREATE(Buckets* buckets ,size_t capacity);
boolean FIND_ITEM(Buckets* ht,void** structure,char* key);
void ADD_ITEM(Buckets* ht,void** structure,char* key);
void HT_FREE(Buckets* ht);

#endif 