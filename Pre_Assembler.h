#ifndef PRE_ASSEMBLER_H
#define PRE_ASSEMBLER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* structure to hold mcro information*/
typedef struct mcro {
    long int Address;
    char Name[81];
} mcro;

typedef struct node * ptr;

/* linked list node  to store defined mcros */
typedef struct node {
    mcro data;
    ptr next;
} item;

/* FUNCTIONS DECLARATIONS */
int Pre_Assembler (const char *filename3, ptr *hptr ,int *flag_too_long);
int check_reserved_words(char word[]);
long int check_if_macro_exist(char word[], ptr p);
int add2list_first(ptr *hptr, mcro data);
void freelist_first(ptr *hptr);
void handle_file_name ( char filename2[],int status);
void CLOSE (FILE * fp1, FILE * fp2, FILE * fp3);
int check_text_after_mcro(FILE *fp1, FILE *fp2, FILE *fp3, ptr *hptr, char *fname, int status, char line []);
void file_cleanup (FILE * fp1, FILE * fp2, FILE * fp3, char *fname, ptr *hptr);
void expand_mcro (FILE *fp1, FILE *fp2, int long pos);
int valid_mcro_name(char name[], FILE *fp1, FILE *fp2, FILE *fp3,ptr *hptr, char *filename);
void comment_handle(char line[]);
int open_files(const char *input, const char *output,FILE **fp1, FILE **fp2, FILE **fp3);
int check_line_too_long(FILE *fp, char *line, int len, int current_line_number, int *flag_too_long);


#endif /* PRE_ASSEMBLER_H */