#ifndef FIRST_PASS_H
#define FIRST_PASS_H

#include <ctype.h>
#include "main.h"

/*a mcro to handle memory allocation errors cleanly
 *if a memory error occurs (-99) it cleans up all allocated memory
 *(files, label table, mcro list) and exits tje first pass safely.
 */
#define HANDLE_MEM_ERROR_FIRST(fp, tablet, missing,hptr) \
do { \
if (memory_error_flag == -99) { \
clean_first_pass(fp, tablet, missing); \
freelist_first(hptr);\
return 0; \
} \
} while(0)

/* enum to easily identify the type of the current ine/label*/
typedef enum flag_type {
    data,
    string,
    external,
    code,
    entry
} flag_type;


/*FUNCTIONS DECLARATIONS*/

int initialize_label_tablet(dynamic_label_arr *label_tablet, size_t initial_size);
int initialize_missing_labels(second_pass_tablet *missing_labels, size_t initial_size);
int check_reserved_words(char word[]);
int check_label_exist(char new_label_name[], dynamic_label_arr *label_tablet);
int add_to_dynamic_label(char new_label_name[],dynamic_label_arr *label_tablet, flag_type, int *value);
int check_Label(char word[],dynamic_label_arr *label_tablet,int *flag_error,int current_line_number,char line[], ptr *hptr);
int type_check(char line[]);
int check_commas(char *str,int current_line_number);
int check_num_is_valid(char word[]);
void string_handle (char line[],word *data_image,int *DC,int *flag_error,int current_line_number);
void data_handle (char line[],word *data_image,int *DC, int *flag_error,int current_line_number);
int is_label_name_valid(char word[],int flag_status);
int check_command_num(char line[],int current_line_number);
int encode_instruction(int command_num, char line[], word code_image[], int *IC,int current_line_number,second_pass_tablet *missing_labels) ;
void add_IC(int *IC, dynamic_label_arr *label_tablet);
void clean_first_pass(FILE *fp1, dynamic_label_arr *label_tablet, second_pass_tablet *missing_labels);
int extern_add_handle (char word[], dynamic_label_arr *label_tablet,int type, int extern_val, int current_line_number);
int is_label_name_mcro_name(char word[],ptr *hptr);

#endif /* FIRST_PASS_H */