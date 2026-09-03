#include "second_pass.h"
#include "First_Pass.h"

/* the main second pass function
 * re scans the file to handle .entry, validates label consistency,
 * resolves previously unknown label addresses and generates to out put files if no errors are found
*/
int second_pass( char filename[] ,dynamic_label_arr *label_tablet,word *code_image, word *data_image, int *IC, int *DC,second_pass_tablet *missing_labels) {
    FILE *fp1;
    ptr2 hptr =NULL;
    int label_index,current_line_number=0, error_flag = 0,extern_found_flag =0, entry_found_flag = 0;
    char filename2[256], filename3[256],temp_line[82],line[82],*token;
    strcpy(filename3,filename);
    handle_file_name(filename3, 1);
    fp1 = fopen(filename3,"r");

    /* read the file line by line */
    while (fgets(line,82,fp1)!=NULL) {
        ++current_line_number;
        strcpy(temp_line,line);
        token = strtok(temp_line," \t");

        /* ignore data or string as they were already handled in the first pass */
        if (strstr(line,".string")!=NULL||strstr(line,".data")!=NULL) continue;

        /* mark if external declarations exist for output file generation */
        if (strstr(line,".extern")!=NULL) extern_found_flag = 1;

        /* handle .entry */
        if (strcmp(token,".entry")==0) {
            token = strtok(NULL," \t\n");

            /* ensure the label referenced by entry actually exists in the label tablet */
            label_index = check_label_exist(token,label_tablet);
            if (label_index==-1) {
                printf("error in line: %d label was not declared\n",current_line_number);
                error_flag =1;
            }
            else {
                entry_found_flag = 1;
                label_tablet->label_name[label_index].flag_entry = 1;
                if (label_tablet->label_name[label_index].flag_external) {
                    printf("error in line: %d label cannot be both entry and external\n",current_line_number);
                    error_flag =1;
                }
            }
            continue;
        }
        /* skip label definitions as they were handled in first */
        if (strchr(token,':')!=NULL) continue;
    }

    /* resolves addresses for labels that were unresolved during the first pass */
    if (!second_pass_handle(missing_labels,label_tablet,&error_flag,IC,code_image, &hptr)) {
        error_flag = 1;
    }
    if (error_flag) {
        printf("invalid file\n");
        clean_second_pass(fp1,label_tablet,missing_labels,&hptr);
        return 0;
    }

    /* generates output files */
    strcpy(filename2,filename);
    handle_file_name(filename2, 2);
    print_ob_file(code_image,data_image,IC,DC,filename2);
    if (extern_found_flag) {
        print_external_file(filename, hptr);
    }
    if (entry_found_flag) {
        print_entry_file(filename, label_tablet);
    }
    clean_second_pass(fp1,label_tablet,missing_labels,&hptr);
    return 1;
}

/* iterates through the list of labels that were unresolved in the first pass
 * calculates the correct address, determines A,R,E property
 *  and completes the encoding in the code image
 */
int second_pass_handle(second_pass_tablet *missing_labels,dynamic_label_arr *label_tablet,int *error_flag,int *IC,word *code_image,ptr2 *hptr) {
    int i, hole,label_index,target_address,current_address;
    char *target_name;
    address_name temp;

    /* goes through every missing label */
    for (i=0;i<missing_labels->count;i++) {
        hole = missing_labels->missing_label[i].IC;
        target_name = missing_labels->missing_label[i].name;
        label_index = check_label_exist(target_name,label_tablet);

        /* validates label for relative addressing */
        if (missing_labels->missing_label[i].relative_flag) {
            if (label_tablet->label_name[label_index].flag_code!=1) {
                printf("error in line: %d not suitable label for relative addressing\n",missing_labels->missing_label[i].line_number);
                *error_flag = 1;
            }
        }

        /* ensure label was actually defined somewhere in the source */
        if (label_index==-1) {
            printf("error in line: %d label was not declared\n",missing_labels->missing_label[i].line_number);
            *error_flag = 1;
        }
        else {
            target_address = label_tablet->label_name[label_index].value;
            /* handle external labels */
            if (label_tablet->label_name[label_index].flag_external) {
                temp.address = hole;
                strcpy(temp.label_name ,label_tablet->label_name[label_index].word);
                if (!add2list_second(hptr,temp)) {
                    printf("memory allocating falied\n");
                    return 0;
                }
                code_image[hole].A_R_E = 'E';
                code_image[hole].val.data = 0;
            }
            /* handle direct addressing */
            else if (missing_labels->missing_label[i].direct_flag == 1) {
                code_image[hole].A_R_E = 'R';
                code_image[hole].val.data = target_address;
            }

            /* handle relative addressing */
            else {
                current_address = hole;
                code_image[hole].A_R_E = 'A';
                code_image[hole].val.data = target_address-current_address;
            }
        }
    }
    return 1;

}

/* writes the final machine code and data to the object file
 * prints the IC and DC headers followed by the encoded instructions
 * and data words in hexadecimal format with their respective A,R,E flags
 */
void print_ob_file(word code_image[],word data_image[],int *IC ,int *DC,char filename2[]) {
    int i;
    FILE *fp;
    fp = fopen(filename2,"w");

    /* write the header */
    fprintf(fp,"   %d %d\n",*IC-100, *DC);

    /* write the code segment starting from the address 100 */
    for (i=100;i<*IC;i++) {
        /* check A,R,E flag to append the correct attribute */
        if (code_image[i].A_R_E == 'E') {
            fprintf(fp, "%04d: %03X E\n", i,code_image[i].val.data & 0xFFF);
        }
        else if (code_image[i].A_R_E == 'R') {
            fprintf(fp, "%04d: %03X R\n", i,code_image[i].val.data & 0xFFF);
        }
        else {
            fprintf(fp, "%04d: %03X A\n", i,code_image[i].val.data & 0xFFF);
        }
    }

    /* write the data segment */
    for (i=0;i<*DC;i++) {
        fprintf(fp, "%04d: %03X A\n", i+*IC,data_image[i].val.data & 0xFFF);
    }
    fclose(fp);
}

/* generates the external file for the assembly process
 * iterates through a linked list of external label references and prints
 * each label name alongside the memory address where it was used
 */
void print_external_file(char filename[], ptr2 hptr) {
    FILE *fp;
    ptr2 temp;
    char filename2[50];
    temp = hptr;
    strcpy(filename2,filename);
    handle_file_name(filename2,3);
    fp = fopen(filename2,"w");

    while (temp != NULL) {
        /* write label name and its 4 digit memory address to file */
        fprintf(fp, "%s %04d\n", temp->data.label_name, temp->data.address);
        temp = temp->next;
    }
    fclose(fp);
}

/* generates the entry file
 * scans the label tablet for labels marked with .entry
 * and write their names and calculated memory addresses to the file
 */
void print_entry_file(char filename[], dynamic_label_arr *label_tablet) {
    FILE *fp;
    int i;
    handle_file_name(filename,4);
    fp = fopen(filename,"w");
    /* iterates through the label tablet to find all entry flagged labels */
    for (i=0;i<label_tablet->count;i++) {
        if (label_tablet->label_name[i].flag_entry) {
            /* write the label name and its 4 digit memory address to the file */
            fprintf(fp, "%s %04d\n", label_tablet->label_name[i].word, label_tablet->label_name[i].value);
        }
    }
    fclose(fp);

}

/* preforms resource cleanup at the end of the second pass
 * closes the file pointer, deallocates the linked list for external labels,
 * and frees the dynamic arrays used for the symbol table and missing labels
 */
void clean_second_pass(FILE *fp1, dynamic_label_arr *label_tablet, second_pass_tablet *missing_labels, ptr2 *hptr) {
    if (fp1 != NULL) {
        fclose(fp1);
    }

    /* free the linked list containing external references */
    if (hptr != NULL) {
        freelist_second(hptr);
    }

    /* free the dynamic array for the symbol table */
    if (label_tablet != NULL && label_tablet->label_name != NULL) {
        free(label_tablet->label_name);
        label_tablet->label_name = NULL;
        label_tablet->count = 0;
        label_tablet->size = 0;
    }

    /* free the dynamic array for missing labels */
    if (missing_labels != NULL && missing_labels->missing_label != NULL) {
        free(missing_labels->missing_label);
        missing_labels->missing_label = NULL;
        missing_labels->count = 0;
        missing_labels->size = 0;
    }
}
