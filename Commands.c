#include "Commands.h"
#include <string.h>

/* table of supported assembly actions
 * defines opcode, funct, number of operands, and allowed addressing methods
 * for both source and destination operands
 */
static table_of_actions arr[16] = {
    {"mov",  0,  0, 2,{1,1,0,1},{0,1,0,1}},
    {"cmp",  1,  0, 2,{1,1,0,1},{1,1,0,1}},
    {"add",  2, 10, 2,{1,1,0,1},{0,1,0,1}},
    {"sub",  2, 11, 2,{1,1,0,1},{0,1,0,1}},
    {"lea",  4,  0, 2,{0,1,0,0},{0,1,0,1}},
    {"clr",  5, 10, 1 ,{0,0,0,0},{0,1,0,1}},
    {"not",  5, 11, 1,{0,0,0,0},{0,1,0,1}},
    {"inc",  5, 12, 1,{0,0,0,0},{0,1,0,1}},
    {"dec",  5, 13, 1,{0,0,0,0},{0,1,0,1}},
    {"jmp",  9, 10, 1,{0,0,0,0},{0,1,1,0}},
    {"bne",  9, 11, 1,{0,0,0,0},{0,1,1,0}},
    {"jsr",  9, 12, 1,{0,0,0,0},{0,1,1,0}},
    {"red", 12,  0, 1,{0,0,0,0},{0,1,0,1}},
    {"prn", 13,  0, 1,{0,0,0,0},{1,1,0,1}},
    {"rts", 14,  0, 0,{0,0,0,0},{0,0,0,0}},
    {"stop",15,  0, 0,{0,0,0,0},{0,0,0,0}}
};

/* extracts the command name from a line and validates its existence
 * if found, it proceeds to check if the operands provided are valid for that command
 */
int check_command_num(char line[],int current_line_number) {
    int i,found_command_name=0;
    char temp_line[82],*token,*operands;
    strcpy(temp_line,line);
    token = strtok(temp_line," \t\n");
    for (i=0; i<16; i++) {
        if (strcmp(arr[i].name,token)==0) {
            found_command_name =1;
            operands = strtok(NULL,"\n");
            if (operands==NULL) operands = "";
            /* validate command structure and operand compatibility */
            if (check_command_valid(i,operands,current_line_number)) return i;
        }
    }
    if (!found_command_name) printf("error in line: %d invalid command\n",current_line_number);
    return -1;
}

/* directs the validation process based on the expected number of operands
 * check for proper comma usage before parsing operands
 */
int check_command_valid(int command_num, char *line,int current_line_number) {
    char temp_line[82];
    strcpy(temp_line,line);
    if (!check_commas(temp_line,current_line_number)) return 0;
    strcpy(temp_line,line);
    /* branch to specific check based on the commands number of operands */
    if (arr[command_num].num_ops==2) {
        if (two_operands_check(temp_line,command_num,current_line_number)) return 1;
        return 0;
    }
    if (arr[command_num].num_ops==1) {
        if (one_operand_check(temp_line,command_num,current_line_number)) return 1;
        return 0;
    }
    if (arr[command_num].num_ops==0) {
        if (zero_operand_check(temp_line,current_line_number)) return 1;
        return 0;

    }
    return 0;
}


/* validates source and destination operands for two operand commands
 * ensures the addressing methods used are permitted for the specific opcode
 */
int two_operands_check(char line[], int command_num,int current_line_number) {
    char *token;
    int type = -1;

    /* validate source operand */
    token = strtok(line," \t,\n");
    if (token!=NULL) type = addressing_type(token,current_line_number);
    if (type ==-2) return 0;
    if (type ==-1) {
        printf("error in line: %d not enough operands\n", current_line_number);
        return 0;
    }
    if (arr[command_num].src_addressing_methods[type]==0) {
        printf("error in line: %d invalid addressing method\n",current_line_number);
        return 0;
    }

    /* validate destination operand */
    type = -1;
    token = strtok(NULL," \t,\n");
    if (token!=NULL) type = addressing_type(token,current_line_number);
    if (type ==-2) return 0;
    if (type ==-1) {
        printf("error in line : %d not enough operands\n",current_line_number);
        return 0;
    }
    if (arr[command_num].dst_addressing_methods[type]==0) {
        printf("error in line: %d invalid addressing method\n", current_line_number);
        return 0;
    }
    if (strtok(NULL," \t,\n")!=NULL) {
        printf("error in line : %d too many operands\n", current_line_number);
        return 0;
    }
    return 1;
}

/* validates the single operand for one operand commands
 * checks that the addressing method is allowed
 */
int one_operand_check(char line[], int command_num,int current_line_number) {
    char *token;
    int type = -1;
    token = strtok(line," \t,\n");
    if (token!=NULL) type = addressing_type(token,current_line_number);
    if (type ==-2) return 0;
    if (type ==-1) {
        printf("error in line: %d not enough operands\n",current_line_number);
        return 0;
    }
    if (arr[command_num].dst_addressing_methods[type]==0) {
        printf("error in line: %d invalid addressing method\n",current_line_number);
        return 0;
    }
    if (strtok(NULL," \t,\n")!=NULL) {
        printf("error in line: %d too many operands\n",current_line_number);
        return 0;
    }
    return 1;
}

/* ensures that commands requiring zero operands have no extra text on the line */
int zero_operand_check(char line[],int current_line_number) {
    char *token;
    token = strtok(line," \t\n");
    if (token!=NULL) {
        printf("error in line: %d too many operands\n",current_line_number);
        return 0;
    }
    return 1;
}

/* determines the addressing type of an operand based on its syntax
 * and return the type or error
 */
int addressing_type(char word[],int current_line_number) {
    if (word==NULL || strlen(word) ==0) return -2;

    /* immediate addressing */
    if (word[0]=='#') {
        if (strlen(word)>1 && check_num_is_valid(word+1)) return immediate;
        printf("error in line: %d operand syntax error, invalid input\n",current_line_number);
        return -2;
    }

    /* relative addressing */
    if (word[0]=='%')return relative; /*{
        if (strlen(word)>1 && check_label_for_operand_syntax(word+1,current_line_number))return relative;
        return -2;
    }*/

    /* register direct addressing */
    if (check_for_register(word)) return register_direct;

    /* direct addressing */
    if (check_label_for_operand_syntax(word)) return direct;
    printf("error in line %d invalid operand\n",current_line_number);
    return -2;
}

/* checks if the given string matches one of the registers */
int check_for_register(char word[]) {
    int i;
    char *registers[] = {"r0","r1","r2","r3","r4","r5","r6","r7",};
    for ( i=0; i<8; i++) {
        if (strcmp(word,registers[i])==0) {
            return 1;
        }
    }
    return 0;
}

/* ensures label operands follow the correct syntax rules */
int check_label_for_operand_syntax(char word[]) {
    int len;
    len = (int)strlen(word);
    if (is_label_name_valid(word,0)) {
        if (len>31) {
            return 0;
        }
        if (check_reserved_words(word)) {
            return 0;
        }
        return 1;
    }
    return 0;
}

/* encodes instructions
 * selects the specific encoder based on the number of operands
 */
int encode_instruction(int command_num, char line[], word code_image[], int *IC,int current_line_number,second_pass_tablet *missing_labels) {
    char line_temp[82];
    int memory_error_flag =0;
    strcpy(line_temp, line);
    if (arr[command_num].num_ops == 2) {
        memory_error_flag = encode_two_ops(line_temp,IC,code_image,command_num,current_line_number,missing_labels);
    }
    else if (arr[command_num].num_ops == 1) {
        memory_error_flag = encode_one_op(line_temp,IC,code_image,command_num,current_line_number,missing_labels);
    }
    else {
        encode_zero_op(IC,code_image,command_num);
    }
    if (memory_error_flag==-99) return -99;
    return 1;
}

/* encodes instruction that take no operands */
void encode_zero_op(int *IC,word code_image[],int command_num) {
    word curr_word ={0};
    curr_word.val.code = (arr[command_num].opcode << 8) | (arr[command_num].funct << 4) | (0 << 2) | (0);
    code_image[*IC] = curr_word;
    (*IC)++;
}

/* encodes instructions that take a single operand */
int encode_one_op(char line_temp[],int *IC,word code_image[],int command_num,int current_line_number,second_pass_tablet *missing_labels) {
    char *op1;
    int dst_type, memory_error_flag1 =0;
    word curr_word;
    op1 = strtok(line_temp," \t,\n");
    dst_type = addressing_type(op1,current_line_number);
    curr_word.val.code = (arr[command_num].opcode << 8) | (arr[command_num].funct << 4) | (0 << 2) | (dst_type);
    code_image[*IC] = curr_word;
    (*IC)++;
    memory_error_flag1 = encode_addressing_types(dst_type,code_image,op1,IC,missing_labels,current_line_number);
    if (memory_error_flag1 == -99)  return -99;
    return 1;
}

/* encodes instruction that take two operands */
int encode_two_ops(char line_temp[],int *IC,word code_image[],int command_num,int current_line_number,second_pass_tablet *missing_labels) {
    char *op1, *op2;
    int src_type,dst_type, memory_error_flag1 =0, memory_error_flag2 =0;
    word curr_word;
    op1 = strtok(line_temp," \t,\n");
    op2 = strtok(NULL," \t,\n");
    src_type = addressing_type(op1,current_line_number);
    dst_type = addressing_type(op2,current_line_number);
    curr_word.val.code = (arr[command_num].opcode << 8) | (arr[command_num].funct << 4) | (src_type << 2) | (dst_type);
    code_image[*IC] = curr_word;
    (*IC)++;
    memory_error_flag1 = encode_addressing_types(src_type,code_image,op1,IC,missing_labels,current_line_number);
    if (memory_error_flag1 ==-99) return -99;
    memory_error_flag2 = encode_addressing_types(dst_type,code_image,op2,IC,missing_labels,current_line_number);
    if (memory_error_flag2 == -99) return -99;
    return 1;
}

/* encodes the additional memory words required based on the addressing type
 * handles immediate values, register direct and records labels for the second pass
 * incrementing the IC for each word added
 */
int encode_addressing_types(int address_type,word *code_image,char *op, int *IC,second_pass_tablet *missing_labels,int current_line_number) {
    int num, memory_flag_error =0;
    word curr_word;

    /* immediate addressing, encodes the numerical value directly */
    if (address_type == 0) {
        op = op+1;
        num = atoi(op);
        if (num>2047 || num<-2048){
          printf("Error in line: %d The number is too big\n",current_line_number);
        }
        curr_word.val.data = num;
        code_image[*IC] = curr_word;
        (*IC)++;
    }

    /* direct addressing, records for resolution in the second pass */
    else if (address_type == 1) {
        memory_flag_error = add_to_missing_labels(missing_labels,1,*IC,op,current_line_number);
        if (memory_flag_error ==-99) return -99;
        (*IC)++;
    }

    /* relative addressing, records label for offset calculation in the second pass */
    else if (address_type == 2) {
       memory_flag_error =  add_to_missing_labels(missing_labels,2,*IC,op+1,current_line_number);
        if (memory_flag_error ==-99) return -99;
        (*IC)++;
    }

    /* register direct, encodes a 1 at the bit positon of the register index */
    else if (address_type == 3) {
        op = op + 1; /* skip the 'r' character */
        num = atoi(op);
        curr_word.val.code = 1 << num; /* shfit 1 to the correct register bit */
        code_image[*IC] = curr_word;
        (*IC)++;
    }
    return 1;
}

/* adds an unresolved label reference to the missing label table for processing in the second pass
 * handles dynamic memory managment by doubling the table size when capacity is reached
 * and stores the addressing type, IC and source line metadata
 */
int add_to_missing_labels(second_pass_tablet *missing_labels,int type, int IC,char name[],int line_number) {
    second_pass_labels temp, *temp_pointer;

    /* check if the dynamic array is full and requires reallocation */
    if (missing_labels->size - missing_labels->count < 1) {
        /* double the current size */
        temp_pointer =realloc(missing_labels->missing_label,(missing_labels->size)*2*sizeof(second_pass_labels));
        if (temp_pointer!=NULL) missing_labels->missing_label = temp_pointer;
        else {
            printf("memory allocation failed\n");
            return -99;
        }
        missing_labels->size = missing_labels->size * 2;
    }
    /* determine addressing type based on the type parameter */
    if (type==1) {
        temp.direct_flag =1;
    }
    else {
        temp.direct_flag =0;
    }
    if (type == 2 ) {
        temp.relative_flag = 1;
    }
    else {
        temp.relative_flag = 0;
    }

    temp.IC = IC;
    strcpy(temp.name,name);
    temp.line_number = line_number;

    /* insert the new record into the table and update the count */
    missing_labels->missing_label[missing_labels->count] = temp;
    missing_labels->count++;
    return 1;
}
