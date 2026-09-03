#include "Pre_Assembler.h"

/* The main pre-assembler function.
 * Reads the source file, identifies macro declarations, saves ther address,
 * and expands them intp the output file (.am) when called.
 */

int Pre_Assembler (const char *filename3, ptr *hptr,int *flag_too_long) {
    int long pos2,pos;
    char line[82], Line_temp[82],Line_temp2[82] ,filename[256], filename2[256], *token, *token2;
    mcro temp;
    int mcro_end_flag,current_line_number2=0,len,line_too_long =0;
    FILE *fp1, *fp2,*fp3;

    /* setup filenames with correct extensions */
    strcpy (filename2,filename3);
    strcpy(filename,filename3);
    handle_file_name(filename2,1);
    handle_file_name(filename,5);
    if (!open_files(filename, filename2, &fp1, &fp2, &fp3)) return 0;

    /* process the file line by line */
    while (fgets (line, 82, fp1) != NULL) {
        comment_handle(line);
        ++current_line_number2;
        strcpy(Line_temp,line);
        len = (int)strlen(Line_temp);

        /* makes sure the line doesn't exceed 80 chars (not including \n) */
        line_too_long = check_line_too_long(fp1, line, len, current_line_number2, flag_too_long);
        if (line_too_long) line[80] = '\n';

        token = strtok (Line_temp, " \t\n");
        if (token == NULL) continue;

        /*handle new mcro definition */
        if (strcmp(token,"mcro")==0) {
            token = strtok(NULL, " \t\n");
            if (token == NULL) {
                printf("error in line: %d Error: missing mcro name\n",current_line_number2);
                file_cleanup(fp1,fp2,fp3,filename2, hptr);
                return 0;
            }


            if (!valid_mcro_name(token,fp1,fp2,fp3,hptr,filename2)) {
                return 0;
            }
            else {
                /* syntax check: nothing should come after the mcro name */
                if (check_text_after_mcro(fp1, fp2, fp3, hptr, filename2, 0, line)) {
                    return 0;
                }

                /* saves the physical address of the mcro content in the file */
                pos = ftell (fp1);
                temp.Address = pos;
                strcpy(temp.Name,token);
                 if (!add2list_first(hptr,temp)) {
                     printf("memory allocating failed\n");
                     return 0;
                 }

            }

            /* skip mcro body in the main read loop until we hit 'mcroend' */
            mcro_end_flag = 0;
            while (fgets (line, 82, fp1) != NULL) {
                ++current_line_number2;
                strcpy(Line_temp2,line);
                token2 = strtok (Line_temp2, " \t\n");
                if (token2!=NULL && strcmp(token2,"mcroend")==0) {
                    if (check_text_after_mcro(fp1, fp2, fp3, hptr, filename2, 1, line)) {
                        return 0;
                    }
                    mcro_end_flag = 1;
                    break;
                }
            }
            if (mcro_end_flag==0) {
                printf("Error: missing mcroend\n");
                file_cleanup(fp1,fp2,fp3,filename2,hptr);
                return 0;
            }
        }

        /* if token matches a known mcro, expand it */
        else if ((pos2 = check_if_macro_exist(token,*hptr))!=-1) {
                 if (check_text_after_mcro(fp1, fp2, fp3, hptr, filename2, 0, line)){
                     return 0;
                }

                expand_mcro(fp2, fp3, pos2);
            }
        else {
            /* check for illegal inline mcro definitions*/
            token = strtok(NULL, " \t\n");
            while (token!=NULL) {
                if (strcmp(token,"mcro")==0) {
                    printf ("error in line: %d extra text before mcro \n",current_line_number2);
                    file_cleanup(fp1,fp2,fp3,filename2,hptr);
                    return 0;
                    }
                token = strtok (NULL, " \t\n");
                }

            /* normal line just copy it to the output file */
            fprintf (fp3,"%s",line);
        }
    }

    CLOSE(fp1,fp2,fp3);
    return 1;
}


/* validates the mcro name against reserved assembly keywords and exitsting mcros */
int valid_mcro_name(char name[], FILE *fp1, FILE *fp2, FILE *fp3,ptr *hptr, char *filename) {
    if (check_reserved_words(name)) {
        printf ("cannot be a reserved word\n");
        file_cleanup(fp1,fp2,fp3,filename,hptr);
        return 0;
    }
    if ( check_if_macro_exist(name,*hptr)!=-1) {
        printf("Mcro cannot be declared again\n");
        file_cleanup(fp1,fp2,fp3,filename,hptr);
        return 0;
    }
    return 1;
}

/* searches tthe linked list for given mcro name.
 * returns the file address of the mcro if found or, -1 otherwise
 */
int long check_if_macro_exist(char word[],ptr p) {
    while (p!=NULL) {
        if (strcmp(word,p->data.Name)==0) {
            return p->data.Address;
        }
        p = p->next;
    }
    return -1;
}

/* ensures there's no extra garbage text on the same line after 'mcro' or 'mcroend'.
 * uses both strtok and direct file reading to catch edge cases.
 */
int check_text_after_mcro(FILE *fp1, FILE *fp2, FILE *fp3, ptr *hptr, char *fname, int status, char line []){
    int c;
    long pos = ftell(fp1);

    /* checks if strtok still has tokens left */
    if (strtok (NULL, " \n\t")!=NULL) {
        file_cleanup(fp1,fp2,fp3,fname,hptr);
        if (status==1) {
            printf ("extra text after mcroend\n");
        }
        else if (status==0) {
            printf ("extra text after mcro name\n");
        }
        return 1;
    }

    /* checks if there are non-whitespace characters eft before the newline in the file */
    if (strchr(line, '\n')==NULL && !feof(fp1)) {
        while ((c = fgetc(fp1))!='\n' && c!=EOF) {
            if (c!= ' ' && c!= '\t') {
                fseek(fp1, pos, SEEK_SET);
                if (status == 1 ) printf("extra text after mcroend\n");
                if (status ==0) printf("extra text after mcro name\n");
                file_cleanup(fp1,fp2,fp3,fname,hptr);
                return 1;
            }
        }
    }
    return 0;
}

/* jumps to the mcro's position in the secondary file pointer (fp2)
 * and copies its contents to the output file (fp3) until 'mcroend'
 */
void expand_mcro (FILE *fp1, FILE *fp2, int long pos) {
    char line[81], Line_temp[81], *token;;
    fseek(fp1,pos,SEEK_SET);

    while (fgets (line, 81,fp1) != NULL) {
        strcpy(Line_temp,line);
        token = strtok (Line_temp, " \t\n");

        if (token!=NULL && strcmp(token,"mcroend")==0) {
            break;
        }
        fprintf (fp2,"%s",line);
    }
}

/* helper for function to safley close the file pointers */
void CLOSE (FILE * fp1, FILE * fp2, FILE * fp3) {
    if (fp1!=NULL) fclose(fp1);
    if (fp2!=NULL) fclose(fp2);
    if (fp3!=NULL) fclose(fp3);
}

/* handles errors by freeing memory, closing files, and removing the incomplete output file*/
void file_cleanup (FILE * fp1, FILE * fp2, FILE * fp3, char *fname, ptr *hptr) {
    CLOSE(fp1,fp2,fp3);
    freelist_first(hptr);
    remove(fname);
}

/* opens the required files.
 * fp1 and fp2 both point to the input file to allow independent fseek operations.
 */
int open_files(const char *input, char const *output, FILE **fp1, FILE **fp2, FILE **fp3) {
    *fp1 = fopen(input,"r");
    *fp2 = fopen(input,"r");
    *fp3 = fopen(output,"w");
    if (*fp1==NULL || *fp2==NULL || *fp3==NULL) {
            printf ("Error in opening files\n");
        CLOSE(*fp1,*fp2,*fp3);
        return 0;
    }
    return 1;
}

/* checks if the line exceeds the maximum length.
 * a line is considered valid if the exceeding characters are merely whitespaces.
 */
int check_line_too_long(FILE *fp, char *line, int len, int current_line_number, int *flag_too_long) {
    int c, only_space = 1;
    /* if the line buffer didn't capture a new line, we need to check the rest of the line */
    if (line[len-1] != '\n' && !feof(fp)) {
        while ((c=fgetc(fp)) != EOF && c != '\n') {
            if (c != ' ' && c != '\t') {
                only_space = 0;
            }
        }

        /* if we found characters, throw an error*/
        if (!only_space) {
            *flag_too_long =1;
            printf("error in line %d The line is too long\n",current_line_number);
        }
        return 1;
    }
    return 0;
}
