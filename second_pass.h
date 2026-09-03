#ifndef SECOND_PASS_H
#define SECOND_PASS_H

#include "main.h"

void handle_file_name ( char filename2[],int status);
int check_label_exist(char new_label_name[], dynamic_label_arr *label_tablet);

/* structure holds label name with its memory address */
typedef struct address_name{
    char label_name[32];
    int address;
}address_name;

/* pointer definition for the second pass linked list */
typedef struct node2 * ptr2;

/* node structure for a linked list of external label references
* uses to store data for the .ext output file
 */
typedef struct node2 {
    address_name data;
    ptr2 next;
} item2;

/* FUNCTIONS DECLARATIONS */

int add2list_second (ptr2 *hptr, address_name data);
int second_pass_handle(second_pass_tablet *missing_labels,dynamic_label_arr *label_tablet,int *error_flag,int *IC,word *code_image,ptr2 *hptr);
void print_ob_file(word code_image[],word data_image[],int *IC ,int *DC,char filename2[]);
void print_external_file(char filename[], ptr2 hptr);
void print_entry_file(char filename[], dynamic_label_arr *label_tablet);
void freelist_second(ptr2 *hptr);
void clean_second_pass(FILE *fp1, dynamic_label_arr *label_tablet, second_pass_tablet *missing_labels, ptr2 *hptr);


#endif /* SECOND_PASS_H */
