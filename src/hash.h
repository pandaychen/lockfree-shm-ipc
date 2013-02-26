#ifndef LSI_HASH_H_
#define LSI_HASH_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*HashFunc)(const void*);
typedef int (*CmpFunc)(const void*, const void*);

typedef struct HashNode
{
    void* m_data;
    struct HashNode* m_next;
}HashNode;

typedef struct HashTable
{
    int m_size;
    int m_count;

    HashFunc m_hash_func;
    CmpFunc m_cmp_func;

    struct HashNode** m_table;
}HashTable;

struct HashTable* hash_create(HashFunc hash, CmpFunc cmp, int size);

int hash_destroy(struct HashTable* htable);

int hash_clean(struct HashTable* htable);

int hash_insert(struct HashTable* htable, void* data);

int hash_remove(struct HashTable* htable, void* data);

void* hash_find(struct HashTable* htable, void* data);

#ifdef __cplusplus
}
#endif

#endif // LSI_HASH_H_

