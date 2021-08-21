#include "headers/hashtable.h"
uint32_t jenkins(char *key)
{
    uint32_t hash, i;
    size_t len = strlen(key);
    for(hash = i = 0; i < len; ++i)
    {
        hash += key[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}
uint32_t hash(char* key,size_t cap){
    uint32_t hash = jenkins(key);
    return hash % cap;
}

void HT_CREATE(Buckets* ht ,size_t capacity){
    capacity*= capacity;
    SAFE_ALLOC(ht->item,calloc(capacity , sizeof *ht->item));
    if(ht->item){
        ht->capacity = capacity;
        ht->items = 0;
    }else{
        fprintf(stderr,"Failed to allocate memory\n");
        exit(EXIT_FAILURE);
    }
}

void ADD_ITEM(Buckets* ht,void** structure,char* key){
    Item* item = NULL;
    Item* _item = NULL;
    uint32_t index = hash(key,ht->capacity);

    SAFE_ALLOC(item,calloc(1,sizeof *item));
    item->data = (void *)*structure;
    item->next = NULL;
    item->items = 1;
    SAFE_ALLOC(item->key,calloc(strlen(key) + 1,sizeof *item->key));
    strcpy(item->key,key);

    _item = ht->item[index];
    if DEBUG_HT 
        fprintf(stdout,"key - :%s\tindex - %d\tdata ptr - 0x%p\n",key,index,*structure);

    if(!_item)
        ht->item[index] = item;
    else{
        _item->items++;
        while(_item->next)
            _item = _item->next;
        _item->next = item;
    }
    ht->items++;
}
boolean FIND_ITEM(Buckets* ht,void** structure,char* key){
    Item* _item = NULL;
    
    if(key){
        uint32_t index = hash(key,ht->capacity);
        _item = ht->item[index];
        
        while(_item){
            if(strcmp(_item->key,key) == 0){
                if(structure)
                    *structure = _item->data;
                return TRUE;
            }
            _item = _item->next;
        }
    }
    return FALSE;
}


void HT_FREE(Buckets* ht){
    Item* _pitem = NULL;
    size_t index = 0;
    for(;index < ht->capacity;index++){
        if(ht->item[index]){
            FREE(ht->item[index]->key);
            while(ht->item[index]){
                _pitem = ht->item[index];
                ht->item[index] = ht->item[index]->next;
                FREE(_pitem);
            }
        }
    }
    FREE(ht->item);
    ht->capacity = 0;
    ht->items = 0;
}
