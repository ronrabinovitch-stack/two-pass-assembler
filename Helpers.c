
#include <stdio.h>
#include "main.h"

/* appends the appropriate file extension to a filename based on the status */
void handle_file_name ( char filename2[],int status) {

    if (status ==1) strcat(filename2,".am");
    if (status ==2) strcat(filename2,".ob");
    if (status ==3) strcat(filename2,".ext");
    if (status ==4) strcat(filename2,".ent");
    if (status ==5) strcat(filename2,".as");
}

/* identifies comments in the source code and truncates the line at the semicolon
 * to prevent the assembler from processing comment text
 */
void comment_handle(char line[]) {
    char *comment_ptr;
    comment_ptr = strchr(line, ';');
    if (comment_ptr!=NULL) {
        /* replace the semicolon with a newline */
        *comment_ptr='\n';
        *(comment_ptr+1)='\0';
    }
}

/* validates the syntax of commas
 * ensures no leading commas, no consecutive commans, no missing commas
 * between operands, and no trailing commas
 */
int check_commas(char *str,int current_line_number) {
    int comma_seen = 0;
    int contact_seen = 0;
    char prev_char;

    if (str == NULL) {
        return 1;
    }

    /* Skip leading whitespace */
    while (*str && (*str == ' ' || *str == '\t')) str++;

    /* Check if the string starts with a comma */
    if (*str == ',') {
        printf("error in line %d Illegal comma\n",current_line_number);
        return 0;
    }

    /* Iterate through the string to check comma usage */
    while (*str) {
        if (*str == ',') {
            if (comma_seen) {
                printf("error in line %d Multiple consecutive commas\n", current_line_number);
                return 0;
            }
            contact_seen = 0;
            comma_seen = 1;
        } else if (*str != ' ' && *str != '\t' && *str != '\n') {
            /* Found content (not whitespace or comma) */
            if (contact_seen) {
                prev_char = *(str-1);
                if  (prev_char == ' '|| prev_char == '\t' || prev_char == '\n'){
                    printf("error in line %d Missing comma\n", current_line_number);
                    return 0;
                }
            }
            comma_seen = 0;
            contact_seen = 1;
        }
        str++;
    }
    if (comma_seen){
        printf("error in line %d Extraneous text after end of command\n",current_line_number);
        return 0;
    }

    return 1;
}

/* checks if a give string is a reserved word
* compares the input against a list of opcodes, directives, and registers
 */
int check_reserved_words(char word[]) {
    int i;
    const char *reserved_words [] = {
        "mov", "cmp", "add", "sub", "lea", "clr", "not", "inc", "dec",
        "jmp", "bne", "jsr", "red", "prn", "rts", "stop",
        ".data", ".string", ".entry", ".extern", ".code",
        "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
        NULL
    };
    i = 0;
    while (reserved_words[i] != NULL) {
        if (strcmp(word,reserved_words[i])==0){
            return 1;}
        i++;
    }
    return 0;
}


/* cleans up resources for the first pass
 * closes open file pointers and frees dynamically allocated memory
 * for the symbol table and missing label tracking
 */
void clean_first_pass(FILE *fp1, dynamic_label_arr *label_tablet, second_pass_tablet *missing_labels) {

    if (fp1 != NULL) {
        fclose(fp1);
    }

    if (label_tablet != NULL) {
        if (label_tablet->label_name != NULL) {
            free(label_tablet->label_name);
            label_tablet->label_name = NULL;
        }
        label_tablet->count = 0;
        label_tablet->size = 0;
    }


    if (missing_labels != NULL) {
        if (missing_labels->missing_label != NULL) {
            free(missing_labels->missing_label);
            missing_labels->missing_label = NULL;
        }
        missing_labels->count = 0;
        missing_labels->size = 0;
    }
}