#include <string.h>
#include <stdlib.h>
#include "hash.h"

HashTable* hash_create(HashFunc hash, CmpFunc cmp, int size)
{
    if (!hash || !cmp || size <= 0)
    {
        return NULL;
    }

    HashTable* htable = (HashTable*)(malloc(sizeof(HashTable)));
    if (!htable)
    {
        return NULL;
    }

    memset(htable, 0, sizeof(*htable));
    htable->m_size = size;
    htable->m_cmp_func = cmp;
    htable->m_hash_func = hash;
    htable->m_table = (HashNode**)malloc(sizeof(HashNode*) * htable->m_size);
    if (!htable->m_table)
    {
        return NULL;
    }

    memset(htable->m_table, 0, sizeof(HashNode*) * htable->m_size);
    return htable;
}

int hash_destroy(HashTable* htable)
{
    if (!htable)
    {
        return -1;
    }

    hash_clean(htable);

    if (htable->m_table)
    {
        free(htable->m_table);
        htable->m_table = NULL;
    }
    free(htable);
    htable = NULL;
    return 0;
}

int hash_clean(HashTable* htable)
{
    if (!htable)
    {
        return -1;
    }

    int i = 0;
    for (; i < htable->m_size; i++)
    {
        // free list node
        HashNode* node = htable->m_table[i];
        HashNode* bak = 0;
        while (node)
        {
            bak = node;
            node->m_data = 0;
            node = node->m_next;
            free(bak);
            bak = 0;
        }
        htable->m_table[i] = 0;
    }

    return 0;
}

int hash_insert(HashTable* htable, void* data)
{
    if (!htable || !data)
    {
        return -1;
    }

    int hash_key = htable->m_hash_func(data);
    int index = hash_key % htable->m_size;
    HashNode* node = htable->m_table[index];
    HashNode* prev = 0;
    while (node)
    {
        // exist items
        if (0 == htable->m_cmp_func(node->m_data, data))
        {
            return -1;
        }
        prev = node;
        node = node->m_next;
    }

    node = (HashNode*)malloc(sizeof(HashNode));
    node->m_data = data;
    node->m_next = 0;
    if (prev)
    {
        prev->m_next = node;
    }
    else
    {
        htable->m_table[index] = node;
    }

    htable->m_count ++;
    return 0;
}

int hash_remove(HashTable* htable, void* data)
{
    if (!htable || !data)
    {
        return -1;
    }

    int hash_key = htable->m_hash_func(data);
    int index = hash_key % htable->m_size;
    HashNode* node = htable->m_table[index];
    HashNode* prev = 0;

    while (node)
    {
        if (0 == htable->m_cmp_func(node->m_data, data))
        {
            if (prev)
            {
                prev->m_next = node->m_next;
            }
            else
            {
                htable->m_table[index] = 0;
            }
            free(node);
            node = 0;
            htable->m_count --;
            return 0;
        }
        prev = node;
        node = node->m_next;
    }
    return -1;
}

void* hash_find(HashTable* htable, void* data)
{
    if (!htable || !data)
    {
        return NULL;
    }

    int hash_key = htable->m_hash_func(data);
    HashNode* node = htable->m_table[hash_key % htable->m_size];

    while (node)
    {
        if (0 == htable->m_cmp_func(node->m_data, data))
        {
            return node->m_data;
        }

        node = node->m_next;
    }

    return NULL;
}

