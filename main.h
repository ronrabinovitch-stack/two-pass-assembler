#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Pre_Assembler.h"

/* struct word
 * represents a single 12-bit word in the fictional machine's memory.
 * uses a union with bit-fields to handle both negative\positive data (int)
 * and raw machine code instructions (unsigned int) in the same memory space.
 */
typedef struct word {
      union val {
          int data: 12;       /* singed 12-bit value for numbers (.data)*/
          unsigned int code : 12;       /*unsinged 12-bit value for instruction encoding*/
     }val;
     char A_R_E;         /*absolute, relocatable, or external flag*/
}word;
/*
 *strcut label
 *represents a symbol (label) in the symbol table
 *holds name, its physical address, and various boolean flags
 *to indicate its type and
 */
typedef struct label {
     char word[32]; /* the name of the label (up to 31 chars + \0)*/

     /* flags for each of the types*/
     int flag_data;
     int flag_string;
     int flag_external;
     int flag_code;
     int flag_entry;

     int value;     /* the symbol's value its IC or DC */
}label;

/* strcut dynamic_label_arr
 * a dynamic array representing the primary symbol table */
typedef struct dynamic_label_arr {
     label *label_name;  /*pointer to the label strcut (used as a dynamic array*/
     int size;           /* current allocated capacity*/
     int count;          /* actual number of labels currently stored*/
}dynamic_label_arr;

/* strcut second_pass_labels
 * tracks missing labes found during fist pass
 * since we might use a label before defining it we save location
 * so the second pass can patch the correct address later.
 */
typedef struct second_pass_labels {
     char name[32]; /*name of the label*/
     int IC;        /* the IC where this label needs to be patched*/
     int direct_flag; /*1 if addressing method is direct*/
     int line_number; /* the line number in the source file */
     int relative_flag; /*1 if addressing method is relative*/
}second_pass_labels;

/*strcut second_pass_tablet
 * a dynamic array tracking all forward references for the second pass
 */
typedef struct second_pass_tablet {
     second_pass_labels *missing_label; /*pointer to the array of missing labels*/
     int size;                          /* current allocated capacity*/
     int count;                         /* actual number of items*/
}second_pass_tablet;

/* FUNCTIONS DECLARATIONS*/

/*pre-assembler, handle mcro's*/
int Pre_Assembler (const char *filename3, ptr *hptr,int *flag_too_long);

/*first pass bulids the symbol table and data/code image*/
int first_pass(char filename[], word code_image[],word data_image[], int *IC, int *DC,dynamic_label_arr *label_tablet,second_pass_tablet *missing_labels, ptr *hptr,int *flag_too_long);

/*second pass resolves missing labels and generates final output files*/
int second_pass( char filename[] ,dynamic_label_arr *label_tablet,word *code_image, word *data_image, int *IC, int *DC,second_pass_tablet *missing_labels);


#endif /* MAIN_H */
