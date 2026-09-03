#include "Pre_Assembler.h"
#include "second_pass.h"

/* adds a new mcro node to the head of the linked list (pre assembler)*/
int add2list_first (ptr *hptr, mcro data) {
    ptr p1;
    ptr t;

    /* allocate a memory for the new node*/
    t = (ptr) malloc(sizeof(item));
    if (!t) {
        printf("malloc failed");
        return 0;
    }

    /* set the data and push the new node to the front of the list*/
    t->data = data;
    p1 = *hptr;
    t->next = p1;
    *hptr = t;
    return 1;
}

/*frees the entire mcro linked list to prevent memory leaks*/
void freelist_first (ptr *hptr) {
    ptr p1;
    while (*hptr) {
        p1 = *hptr;
        *hptr = (*hptr)->next;
        free(p1);
    }
}

/* adds a new address node to the head of the list for 'external' type label (second pass) */
int add2list_second (ptr2 *hptr, address_name data) {
    ptr2 p1;
    ptr2 t;

    /* allocate memory for the new node*/
    t = (ptr2) malloc(sizeof(item2));
    if (!t) {
        printf("malloc failed");
        return 0;
    }
    /* set the data and push the new node to the front of the list*/
    t->data = data;
    p1 = *hptr;
    t->next = p1;
    *hptr = t;
    return 1;
}

/*frees entire second pass linked list to prevent memory leaks*/
void freelist_second(ptr2 *hptr) {
    ptr2 p1;
    while (*hptr) {
        p1 = *hptr;
        *hptr = (*hptr)->next;
        free(p1);
    }
}
