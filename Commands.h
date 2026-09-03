
#ifndef COMMANDS_H
#define COMMANDS_H

#include "First_Pass.h"

/* structure representing an entry in the command lookup table
 * stores opcode, funct, operand cound and arrays for valid source
 * and destination addressing methods
 */
typedef struct  table_of_actions{
    char *name;
    int opcode;
    int funct;
    int num_ops;
    int src_addressing_methods[4];
    int dst_addressing_methods[4];
} table_of_actions;

/* enum representing the four addressing types */
typedef enum addressing_types {
    immediate,
    direct,
    relative,
    register_direct
}addressing_types;

/* FUNCTION DECLARATIONS */

int check_command_valid(int command_num, char *line,int current_line_number);
int check_for_register(char word[]);
int addressing_type(char word[],int current_line_number);
int check_label_for_operand_syntax(char word[]);
int two_operands_check(char line[], int command_num,int current_line_number);
int one_operand_check(char line[], int command_num,int current_line_number);
int zero_operand_check(char line[],int current_line_number);
int encode_two_ops(char line_temp[],int *IC,word code_image[],int command_num,int current_line_number,second_pass_tablet *missing_labels);
int encode_addressing_types(int address_type,word *code_image,char *op, int *IC,second_pass_tablet *missing_labels,int current_line_number);
int encode_one_op(char line_temp[],int *IC,word code_image[],int command_num,int current_line_number,second_pass_tablet *missing_labels);
void encode_zero_op(int *IC,word code_image[],int command_num) ;
int add_to_missing_labels(second_pass_tablet *missing_labels,int type, int IC,char name[],int line_number);

#endif /* COMMANDS_H */
