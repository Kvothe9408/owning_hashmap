// Standard Libraries
#include <stdio.h> // functions like printf
#include <stdlib.h> // functions like malloc
#include <string.h> // functions like strlen (string length), strcmp (string compare)

// Defintions

typedef struct Node Node; // declare the Node strcut to avoid self referencing in the next structu
typedef struct Node {
    char *key; // char is the string type in C / because this a a hashmap, the initial input value needs to be noted in the struct so the search can happen later a specific key/value pair
    char *value;
    struct Node *next;
} Node;
typedef struct linkedlist {
    Node *head;
} linkedlist;
typedef struct hashmap {
    linkedlist *list;
    int count; // counts the number of values that have been stored in the hashmap, which is needed for the load factor (count/capacity)
    int capacity; // indicates the capacity of the hashmap array, which is needed for the load factor
} hashmap; // found out later that the load factor could have been included in the struct, probably should be instead of being hardcoded in the functions

// functions
unsigned int hash(char *key) { //unisgned int indicates that the number must stay positive, so instead of having a range of -2000 -> 2000 it's 0 -> 4000 / char is C for string variables
    int result = 5381; // defining interger variable / 5381 is standrad for this type of hash function
    int index = 0; // defining index
    while(key[index] != '\0') { // loop that keeps going as long as the key doesn't equal \0 how string arrays terminate in C / note all strings in C are just an array of numbers
        result = result * 31; // 31 is standard prime to use for this type of hash function as it has shown to distribute values better
        result = result + key[index]; // after the multiplication we add the result with the current int value at key[index]
        index++; // increment the index count
    } 
    return result; // returns the result / mod happens in the function itself once, the capacity is defined
}
hashmap *create_hashmap(int capacity) { // as noted, this is a version of an array
    hashmap *hash = (hashmap *)malloc(sizeof(hashmap)); // memory allocation for the struct
    if (hash == NULL) { // check if the memory failed to allocate
        printf("memory allocation failed"); 
        return NULL;
    }
    hash->list = (linkedlist *)malloc(sizeof(linkedlist) * capacity); // enter through the hash->list pointer and allocate memory for the array of linkedlist pointers
    if (hash->list == NULL) { // check is the memory allocation failed
        printf("memory allocation failed");
        return NULL;
    }
    int index = 0;
    while (index != capacity) { // this while loop re-assigns the linked list head in each slot in the hashmap array to NULL
        hash->list[index].head = NULL; // enter through hash->list pointer and reveal value at current index. Since the value at the index is a struct go to that structs head and set to NULL
        index++; // increment index
    }
    hash->count = 0; // set the count of the hashmap to 0
    hash->capacity = capacity; // set the hashmap capacity to the variable capacity;
    return hash; // return the hashmap pointer
}
hashmap *rehash(hashmap *old_hash) { // function to expand the hashmap, if the load factor gets to high. Will be nested in the insert function, had to create the variable old_hash so there isn't a conflict with the funciton call hash
    hashmap *new_hash = (hashmap *)malloc(sizeof(hashmap)); // memory allocation for the struct, needed new_hash for the same reason as old_hash
    if (new_hash == NULL) { // NULL check
        printf("memory allocation failed");
        return NULL;
    }
    new_hash->list = (linkedlist *)malloc(sizeof(linkedlist) * old_hash->capacity * 2); // memory allocation for the new hashmap
    if (new_hash->list == NULL) { // NULL check
        printf("memory allocation failed");
        return NULL;
    }
    new_hash->capacity = old_hash->capacity * 2; // setting the capacity of the new hashmap to double of the old hashmap
    int index_rehash = 0; // new variable for index that I will be using to increment walk through the hashmap of linkedlists
    while (index_rehash != new_hash->capacity) { // loop that is true while the index hasn't reached hashmap capacity yet
        new_hash->list[index_rehash].head = NULL; // within the new hashmap, go to list at "index" and set the head to NULL
        index_rehash++; // increment index by +1
    }
    int index_hash = 0;
    while (index_hash != old_hash->capacity) { // loop to move through the hashmap and rehash all the values into the new hashmap
        Node *current = old_hash->list[index_hash].head; // creating a node pointer that can walk through the linkedlists
        while (current != NULL) { // loop rehash value from the old to the new hashmap
            unsigned int result = hash(current->key); // creates variable 'result' and sets it to the hashed value of the key being used from teh original hashmap
            int result_mod = result % new_hash->capacity; // creates a variable 'result_mod' and sets it to the mod of result but using the capacity of the new hashmap
            index_rehash = result_mod; // sets the variable index_rehash to result_mod, so we can use it to place the key/value pair in the right linkedlist in the new hashmap
            Node *node = (Node*)malloc(sizeof(Node)); // creates a memory alloaction for a Node struct
            if (node == NULL) { // NULL check
                printf("memory allocation failed");
                return NULL;
            }
            node->value = current->value; // places value from the node in the old hashmap into the value field of the newly created node
            node->key = current->key; // ditto for the key
            node->next = new_hash->list[index_rehash].head; // sets the node's next pointer to the previous head of the linkedlist, as we are pushing this new node to the fornt of the list
            new_hash->list[index_rehash].head = node; // setting the linkedlist head pointer to point at the new node
            Node *delete = current; // create pointer delete and set to current, so we delete the node form the original hashmap as we move along the linkedlist during the rehash
            current = current->next; // increment the current pointer to the next node in the original hashmap
            free(delete); // free the node from the original hashmap linkedlist
        }
        index_hash++; // increment the index of the original hashmap by 1
    }
    free(old_hash->list); // free up the memory for the old hashmap array
    old_hash->list = new_hash->list; // re-assing the address of the new hashmap to the old one to make sure this works in the main function
    old_hash->capacity = new_hash->capacity; // ditto
    free(new_hash); // free up the memory struct for the new hashmap
    return old_hash; // return the new hashmap with the rehashed data
}

hashmap *insert(hashmap *hash_old, char *key, char *value) { // function to insert a new value into the hashmap / don't forget that there can be naming conflicts with variables and functions like the hash function and then variables that are in place of the hashmap pointer
    Node *node = malloc(sizeof(Node)); // creating memory allocation for the node that will hold the pointers poiting at the strings copied from the data segement to the heap
    if (node == NULL) {
        fprintf(stderr, "memory allocation failed\n");
        return hash_old; // the limitation here is that failure and success returns look the same so we can't tell but it's not worth re-doing everything at this stage as this failure will sure come up later
    }
    char *key_copy = malloc(strlen(key)+1); // allocating memory on the heap for the string
    if (key_copy == NULL) {
        fprintf(stderr, "memory allocation failed\n");
        free(node);
        return hash_old;
    }
    char *value_copy = malloc(strlen(value)+1); // allocating memory on the heap for the string
    if (value_copy == NULL) {
        fprintf(stderr, "memory allocation failed\n");
        free(key_copy);
        free(node);
        return hash_old;
    }
    strcpy(key_copy, key); // copying the string in the data segement into the memory allocation on the heap
    strcpy(value_copy, value); // copying the string in the data segement into the memory allocation on the heap
    node->value = value_copy; // setting the node value pointer to point at the value_copy address
    node->key = key_copy; // setting the node value pointer to point at the value_copy address
    unsigned int result = hash(key); // result is unsigned int because it can't be negative for the mod to work within the hash function
    int result_mod = result % hash_old->capacity; // calculating the mod of the hashed result
    int index_hash = result_mod; // setting the result value to the index value to use below
    node->next = hash_old->list[index_hash].head; //setting the new node next value to the header of the previous node at the front of the linkedlist
    hash_old->list[index_hash].head = node; // setting the head pointer to the linkedlist to point at the head of the new node
    hash_old->count++; // incrementing the count of the hashmap
    if ((float) hash_old->count/hash_old->capacity > 0.75) { // checking he load factor of the hashmap and determining if we need to rehash the hashmap / note that the load factor is hardcoded into the function but probably could and should be included in the hashmap struct / also note that one of the variables in the equation needs to be cast (temporarily change its data type) as float, so that C doesn't truncate the result and gives a decimal value
        hash_old = rehash(hash_old); // rehash is the load factor is too high
    }
    return hash_old; // return the hashmap pointer, which now points at the new rehashed hashmap
}

char *get(hashmap *current_hash, char *key) { //get function
    unsigned int result = hash(key); // hasing key to find index
    int result_mod = result % current_hash->capacity;  // mod
    int index_hash = result_mod;
    Node *current_head = current_hash->list[index_hash].head; // walk thgouth node
    if (current_head == NULL) { // NULL check for the linekedlist before starting the walk through
        printf("not found");
        return NULL;
        }
    while (strcmp(key, current_head->key) != 0) { // strcmp is the functionality taht checks whether two strings are equal / while loop to find the matching key within the nodes of the linkedlist
        current_head = current_head->next; // node walk through
        if (current_head == NULL) { // NULL check after every walk to make sure we stop when the linkedlist terminates
            printf("not found");
            return NULL;
        }
    }
    return current_head->value; // return the value of the key value pair
}

void delete(hashmap *current_hash, char *key) { // delete a specific key value pair node in the hashmap
    unsigned int result = hash(key);
    int result_mod = result % current_hash->capacity; // getting the mod for the correct index after the hash
    int index_hash = result_mod;
    Node *current_delete = current_hash->list[index_hash].head; // setting the new node to the linkedlist head at the specific index_hash position in the hashmap
    if (current_delete == NULL) { // checking if the list is empty
        printf("not found");
        return;
    }
    Node * current_2 = NULL; // creating a second node to trail the main deleting node during the list walk through. this node is needed, so we can link the pre-delete node to the post-delete node
    while (strcmp(key, current_delete->key) != 0) { //checks if the string value matches another, subtracts one from the other, thus we are checking if the results is 0 or not
        current_2 = current_delete; // setting the trailing node pointer to point at the delete node head
        current_delete = current_delete->next; // moving the delete node forward
        if (current_delete == NULL) { // checking if the list ended
            printf("not found");
            return;
        }
    }
    if (current_2 == NULL) { // checking if current_2 is NULL indicating the node to be deleted is the first in the list, since current_2 was never moved forward
        current_hash->list[index_hash].head = current_delete->next; // setting the head of the list to point at the head of the second node / if that next pointer is pointing at NULL, that is also fine
        free(current_delete); // delete the node
        current_hash->count--;
        return;
    }
    current_2->next = current_delete->next; // setting the next pointer of the pre-delete node to the head of the post-delete pointer
    free(current_delete); // delete the node and data
    current_hash->count--; // need this to make sure the load factor is accurate
}

void print(hashmap *current_hash, char *key) { // print function to check if the program is acting correctly
    int index_hash = 0;
    unsigned int result = hash(key);
    int result_mod = result % current_hash->capacity;
    printf("key:");
    printf("%s\n", key); // printing the key value
    printf("mod:");
    printf("%d\n", result_mod); // printing the manual hash_mod value to then check agains the slot it actually sits in
    printf("load factor:");
    printf("%f\n", (float)current_hash->count / current_hash->capacity);
    printf("Capacity:");
    printf("%d\n", current_hash->capacity);
    printf("\n");
    Node *current_node = NULL;
    for (index_hash = 0; index_hash != current_hash->capacity; index_hash++) { // using a for loop because it increments the value automatically even if the loop cycle terminates early
        printf("Bucket:");
        printf("%d\n", index_hash); // print the index position
        current_node = current_hash->list[index_hash].head; // set the node to the head of the current list
        if (current_node == NULL) { // check is the list is empty
            printf("no data");
            printf("\n");
            continue; // terminate the loop cycle but not the function
        }
        while (current_node != NULL) { // lopp to print values as long as the current node doesn't point at NULL, thus the list being finished
            printf("%s\n", current_node->key); // print key
            printf("%s\n", current_node->value); // print value
            printf("\n");
            current_node = current_node->next; // walk the node forward
        }    
    }
}

void free_hashmap(hashmap *old_hash) {
    int index_hash = 0;
    while (index_hash != old_hash->capacity) {
        Node *current_node = old_hash->list[index_hash].head;
        while (current_node != NULL) {
            Node *delete_node = current_node;
            current_node = current_node->next;
            free(delete_node);
        }
        index_hash++;
    }
    free(old_hash->list);
    free(old_hash);
}

int main(void) {
    hashmap *hash_new = create_hashmap(2);
    insert(hash_new, "alice", "abc");
    print(hash_new, "alice");
    insert(hash_new, "bob", "def");
    print(hash_new, "bob");
    insert(hash_new, "charlie", "hij");
    print(hash_new, "charlie");
    printf("%s\n", get(hash_new, "alice"));
    printf("%s\n", get(hash_new, "bob"));
    delete(hash_new, "alice");
    print(hash_new, "alice");
    print(hash_new, "bob");
    free(hash_new);
    return 0;
}