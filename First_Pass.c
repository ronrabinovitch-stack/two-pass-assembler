#include "First_Pass.h"
#include "main.h"

/*the main logic for the first pass
 *scans the file, finds labels, handles different types of directives,
 *counts IC  and DC, and encodes available information
 *and leaves placeholders for unresolved labels to be completed in the second pass
 */

int first_pass(char filename[], word code_image[],word data_image[], int *IC, int *DC,dynamic_label_arr *label_tablet,second_pass_tablet *missing_labels, ptr *hptr,int *flag_too_long) {
    int   flag_label, type, flag_error = 0, command_num,current_line_number = 0,extern_val =0, memory_error_flag =0;
    FILE *fp1;
    char line[82],*token,temp_line[82],temp_line2[82],temp_line3[82],*operands;

    /*allocate initial memory for the label arrays*/
    if (!initialize_label_tablet(label_tablet, 5)) return 0;
    if (!initialize_missing_labels(missing_labels, 5)) {
        free(label_tablet->label_name);
        return 0;
    }

    fp1 = fopen(filename,"r");
    if (fp1 == NULL) {
        fprintf(stderr,"error in line %d Can't open file %s\n",current_line_number,filename);
        free(label_tablet->label_name);
        return 0 ;
    }

    /*read the file line by line*/
    while (fgets(line,82,fp1)!=NULL) {
        char *label_name = NULL;
        flag_label = 0;
        ++current_line_number;
        strcpy(temp_line,line);
        strcpy(temp_line2,line);
        strcpy(temp_line3,line);
        token = strtok(temp_line," \t\n");
        if (token == NULL) continue; /*skip empty lines*/

        /* check if the first word is label declaration*/
        if (check_Label(token,label_tablet,&flag_error,current_line_number,temp_line3, hptr)) {
            flag_label = 1;
            label_name = token;
            token = strtok(NULL," \t"); /*move to the nest word after the label*/
        }
        type = type_check(token);

        /* .entry is handled in the seconds pass, just ignore here but check fpr warnings*/
        if (type == entry) {
            if (flag_label) {
                printf("warning in line %d label name before entry\n",current_line_number);
            }
            continue;
        }

        /*find where the operands start in the original line*/
        operands = strstr(temp_line2,token);
        if (operands!=NULL) operands += strlen(token);

        /* handle data variables */
        if (type == data || type == string ) {
            if (flag_label) memory_error_flag = add_to_dynamic_label(label_name,label_tablet,type,DC);
            HANDLE_MEM_ERROR_FIRST(fp1,label_tablet,missing_labels,hptr);
            if (type == data) {
                data_handle(operands,data_image,DC,&flag_error,current_line_number);
            }
            if (type == string) {
                token = strtok(NULL,"\n");
                if (token!=NULL)string_handle(operands, data_image, DC,&flag_error,current_line_number);
            }
            continue;
        }

        /* handle external variables*/
        if (type == external) {
                token = strtok(NULL," \t\n");
                if (token!=NULL){
                    memory_error_flag = extern_add_handle(token,label_tablet,type,extern_val,current_line_number);
                    HANDLE_MEM_ERROR_FIRST(fp1,label_tablet,missing_labels,hptr);
                    if (memory_error_flag ==0) flag_error=1;
                }
                continue;
        }

        if (flag_label) {
            memory_error_flag = add_to_dynamic_label(label_name,label_tablet,code,IC);
            HANDLE_MEM_ERROR_FIRST(fp1,label_tablet,missing_labels,hptr);
        }

        /* find instruction type and encode it*/
        operands = strstr(temp_line2,token);
        command_num = check_command_num(operands,current_line_number);
        if (command_num==-1) flag_error = 1;
        if (command_num!=-1) {
            operands += strlen(token);
            memory_error_flag = encode_instruction(command_num,operands,code_image,IC,current_line_number,missing_labels);
            HANDLE_MEM_ERROR_FIRST(fp1,label_tablet,missing_labels,hptr);
        }
    }

    /*update data labels to point after the code section in memory*/
    add_IC(IC,label_tablet);
    if (flag_error || *flag_too_long) {
        printf("file is invalid\n");
        clean_first_pass(fp1,label_tablet,missing_labels);
        return 0;
    }
    fclose(fp1);
    freelist_first(hptr);
return 1;
}

/* allocates the initial array for the symbol table*/
int initialize_label_tablet(dynamic_label_arr *label_tablet, size_t initial_size) {
    label *temp_p1;
    temp_p1 = calloc(initial_size,sizeof(label));
    if (temp_p1 != NULL) {
        label_tablet->label_name = temp_p1;

    }
    else {
        printf("memory allocation failed\n");
        free(temp_p1);
        return 0;
    }
    label_tablet->size = 5;
    label_tablet->count = 0;
    return 1;
}

/* allocates the initial array for the missing labels*/
int initialize_missing_labels(second_pass_tablet *missing_labels, size_t initial_size) {
    second_pass_labels *temp_p2;
    temp_p2 = calloc(initial_size,sizeof(second_pass_labels));
    if (temp_p2!=NULL) missing_labels -> missing_label = temp_p2;
    else {
        printf("memory allocation failed\n");
        free(temp_p2);
        return  0;
    }
    missing_labels ->size = 5;
    missing_labels -> count = 0;
    return 1;
}

/* validates label syntax: ends with colon, text after it, and reserved words*/
int check_Label(char word[],dynamic_label_arr *label_tablet,int *flag_error,int current_line_number,char line[], ptr *hptr){
    int len,i, text_after_label_flag =0;
    char *colonptr;
    len = (int)strlen(word);

    /*  a label definition must end with ':'*/
    if (word[len-1] == ':') {
        if (!is_label_name_valid(word,1)) {
            printf("error in line: %d illegal label name\n",current_line_number);
            *flag_error =1;
        }

        /* make sure the is actual code/data after the label declaration*/
        colonptr = strchr(line,':');
        for (i=1; colonptr[i]!='\0';i++) {
            if (isalnum(colonptr[i])) {
                text_after_label_flag = 1;
                break;
            }
        }
        if (!text_after_label_flag) {
            printf("error in line: %d no text after label name\n",current_line_number);
            *flag_error = 1;
            return 0;
        }
        if (len>32) {
            printf("error in line %d The label is too long\n",current_line_number);
            *flag_error =1;
            return 0;
        }

        /* removes the colon to save the real name*/
        word[len-1] = '\0';
        if (check_reserved_words(word)) {
            printf("error in line %d The label cannot be a reserved word\n",current_line_number);
            *flag_error =1;
            return 0;
        }

        if (check_label_exist(word,label_tablet)!=-1) {
            printf("error in line %d The label already exist\n", current_line_number);
            *flag_error = 1;
            return 0;
        }
        if (is_label_name_mcro_name(word, hptr)) {
            printf("error in line: %d label name cannot be mcro name\n", current_line_number);
            *flag_error = 1;
        }
        return 1;
    }
    return 0;
}

/* checks if the label contains only letters and numbers, starting with a letter*/
int is_label_name_valid(char word[],int flag_status) {
    int len,i;
    len = (int)strlen(word);

    if (!isalpha(word[0])) {
        return 0;
    }

    if (flag_status) {
        len = len-1; /*ignore the colon if it's a declaration*/
    }
    for (i=1;i<len;i++) {
        if (isalnum(word[i])==0) return 0;
    }
    return 1;
}

/* ensure we don't declare a label with the same name as a mcro*/
int is_label_name_mcro_name(char word[],ptr *hptr) {
    ptr current = *hptr;

    while (current!=NULL) {
        if (strcmp(word, current->data.Name) == 0) {
            return 1;
        }
        current = current->next;
    }
    return 0;
}

/* add a new label to the symbol table, reallocating memory if the array is full*/
int add_to_dynamic_label(char new_label_name[],dynamic_label_arr *label_tablet,flag_type flag,int *value) {
    label *temp_label_p;
    label temp = {"", 0, 0, 0,0 };
    temp.value = *value;
    strcpy(temp.word,new_label_name);

    /*checks if we need to double array size*/
    if (label_tablet->size - label_tablet->count < 1) {
        temp_label_p = realloc(label_tablet->label_name,(label_tablet->size)*2*sizeof(label));
        if (temp_label_p!=NULL) label_tablet->label_name =temp_label_p;
        else {
            printf("memory allcation failed\n");
            return -99;
        }
        label_tablet->size = label_tablet->size * 2;
    }

    /* turn on the correct flag based on the type*/
    switch (flag) {
        case data: temp.flag_data = 1; break;
            case entry: temp.flag_entry = 1; break;
                case string:  temp.flag_string = 1; break;
                    case external: temp.flag_external = 1; break;
                        case code: temp.flag_code = 1; break;
                            default: break;
    }
    label_tablet->label_name[label_tablet->count] = temp;
    label_tablet->count++;
    return 1;
}

/* searches the symbol table for a label, returns index if found -1 otherwise*/
int check_label_exist(char new_label_name[], dynamic_label_arr *label_tablet) {
    int i;
    for (i = 0; i < label_tablet->count; i++) {
        if (strcmp(label_tablet->label_name[i].word, new_label_name) == 0) {
            return i;
        }
    }
    return -1;
}


/* identifies guidance commands*/
int type_check (char word[]) {
    if (strcmp(word,".data")==0){
        return data;
    }
    else if (strcmp(word,".string")==0) {
        return string;
    }
    else if (strcmp(word,".entry")==0) {
        return entry;
    }
    else if (strcmp(word,".extern")==0) {
        return external;
    }
    return -1;
}


/* parses a line with .data, checks numbers and commas, and adds to data_image*/
void data_handle (char line[],word *data_image,int *DC, int *flag_error,int current_line_number) {
    char line_temp[82],*token;
    int num;
    if (line==NULL|| strlen(line)==0) return;

    strcpy(line_temp,line);
    strtok(line,"\n");
    if (!check_commas(line,current_line_number)) {
        *flag_error =1;
        return;
    }

    strcpy(line_temp,line);
    token = strtok(line_temp," \t,");

    if (token==NULL) {
        printf("error in line %d invalid command\n",current_line_number);
        *flag_error =1;
        return;
    }
    while (token) {
        if (!check_num_is_valid(token)) {
            printf("error in line %d only numbers can be written after .data\n",current_line_number);
            *flag_error = 1;
            return;
        }
        num = atoi(token);
        if (num>2047|| num<-2048) {
            printf("error in line %d The number is too big\n",current_line_number);
            *flag_error = 1;
            return;
        }
        /* store the number in the data array and advance DC*/
        data_image[*DC].val.data = num;
        (*DC) ++;
        token = strtok(NULL," \t,");

    }

}


/* parses a .string line, ensures correct quotes, and adds chars to data_image*/
void string_handle (char line[],word *data_image,int *DC,int *flag_error,int current_line_number) {

    char *ptr, *start, *end, *check_end,*check_start;
    start = strchr(line, '"');

    if (start == NULL) {
        printf("error in line %d No opening quotes to the string\n",current_line_number);
        *flag_error =1;
        return;
    }
    end = strchr(start+1,'"');

    if (end == NULL) {
        printf("error in line %d No closing quotes to the string\n",current_line_number);
        *flag_error =1;
        return;
    }

    /* make sure there is no junk text after closing quote*/
    check_end = end + 1;
    while (*check_end != '\n' && *check_end != '\0') {
        if (*check_end != ' ' && *check_end != '\t') {
            printf("error in line %d Theres text after the last quote\n",current_line_number);
            *flag_error =1;
            return;;
        }
        check_end++;
    }

    /* make sure there is no junk text before opening quote*/
    check_start =line;
    while (check_start<start) {
        if (*check_start != ' ' && *check_start != '\t') {
            printf("error in line %d Extraneous text before string\n",current_line_number);
            *flag_error =1;
            return;
        }
        check_start++;
    }

    /* add chars to data memory*/
    ptr = start + 1;
    while (ptr < end) {
        data_image[*DC].val.data = (int)(*ptr);
        (*DC) ++;
        ptr++;
    }
    /* null terminator for the string*/
    data_image[*DC].val.data = 0;
    (*DC) ++;
}


/* checks if a string represents a valid integer */
int check_num_is_valid(char word[]) {
    int i = 0,len;
    len = (int)strlen(word);
    if (word[0] == '-' || word[0]=='+') {
        if (len==1) return 0; /*just a sign is not a number*/
        i = 1;
    }
   for (; i<len; i++) {
       if (!isdigit(word[i])) {
           return 0;
       }
   }
    return 1;
}


/* updates the addresses of data/string labels
 * because data sits after code in memory, we add the final IC to their values*/
void add_IC(int *IC, dynamic_label_arr *label_tablet) {
    int i;
    for (i = 0; i < label_tablet->count; i++) {
        if (label_tablet->label_name[i].flag_data || label_tablet->label_name[i].flag_string) {
            label_tablet->label_name[i].value += *IC;
        }
    }
}

/*handles .extern declarations. adds to table or ignores if already declared*/
int extern_add_handle (char word[], dynamic_label_arr *label_tablet,int type, int extern_val, int current_line_number) {
    int existing_label_index, memory_error_flag;
    existing_label_index = check_label_exist(word,label_tablet);
    if (existing_label_index == -1) {
        memory_error_flag = add_to_dynamic_label(word,label_tablet,type,&extern_val);
        if (memory_error_flag ==-99) {
            printf("memory allocation failed\n");
            return -99;
        }
        return 1;
    }

    /* if it exists make sure it's actually an external label and not local*/
    if (label_tablet->label_name[existing_label_index].flag_external) {
        return 1;
    }
        printf("error in line: %d label cannot be both local and external type\n",current_line_number);
        return  0;
}
