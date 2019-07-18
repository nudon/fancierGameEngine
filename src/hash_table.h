#ifndef FILE_HASHTABLE_SEEN
#define FILE_HASHTABLE_SEEN

typedef struct hash_table_struct hash_table;

hash_table* create_hash_table(int (*compare_func)(void* a, void* b), int (*raw_hash_func)(void*a), int power);

void init_hash_table(hash_table* table, int (*compare_func)(void* a, void* b), int (*raw_hash_func)(void*a), int power, double sat_thresh);

int get_table_size(hash_table* t);

//returns negative 1 if data is already in table
//otherwise returns value of index to put data
int calc_data_index(hash_table* table, void* data);

//returns 1 if data was added
//returns 0 otherwise
int insert(hash_table* table, void* data);

void grow_table(hash_table* t);

void clear_table(hash_table* table);

// 2^m = bucket_size of hash table
int mult_shift_hash(int x, int m);

int simple_hash(hash_table* table, void* data);

int index_hash(hash_table* table, void* data);

#endif
