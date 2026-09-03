/* mmn 14
 * authors: Stav Barzilay 218046035 & Ron Rabinovitch 332188051
 */

/* note for tester: at 09.03.2026 it was written in the forum that we can find only one error for each line*/
#include "main.h"

#include "Pre_Assembler.h"

/* main entry point for assembler project
 * loops through assembly files
 * and passes each file through full assembly pipeline:
 * pre-assembler-> first pass -> second pass
 */
int main(int argc, char *argv[]) {
    char file_name[256];
    word data_image[4096];
    word code_image[4096];
    

    /* initialize the symbol table and the second pass tablet */
    dynamic_label_arr label_tablet = {0};
    second_pass_tablet missing_labels = {0};

    ptr hptr = NULL; /*pointer for the mcro linked list */
    int i, flag_too_long =0;
    char am_filename[256];

    /* ensure at least one file was passed as an argument*/
    if (argc < 2) {
        printf("no file provided\n");
        return 1;
    }

    /* process each file independently*/
    for ( i=1 ; i<argc; i++) {
        /* initialize memory counters*/
        int IC = 100;
        int DC = 0;
        flag_too_long = 0;
        
        /*initialize the machine memory arrays*/
        memset(&label_tablet, 0, sizeof(label_tablet));
        memset(&missing_labels, 0, sizeof(missing_labels));
        memset(code_image, 0, sizeof(code_image));
        memset(data_image, 0, sizeof(data_image));
        memset(&label_tablet, 0, sizeof(label_tablet));
        memset(&missing_labels, 0, sizeof(missing_labels));
        strcpy(file_name,argv[i]);
        hptr = NULL;
        am_filename[0] = '\0';
        /* pre assembler (mcro unfolding)*/
        /* if successful creates a new file with the ".am" extension*/
        if (Pre_Assembler(file_name, &hptr,&flag_too_long)){

            /*builds the resulting .am filename to pass to the next stage*/
            strcpy(am_filename,argv[i]);
            strcat(am_filename,".am");

            /* first pass */
            /* parses the code, extracts labels, update counters and builds the memory images*/
            if (first_pass(am_filename,code_image,data_image,&IC,&DC,&label_tablet,&missing_labels, &hptr,&flag_too_long)) {

                /* seconds pass*/
                /*resolves missing label addresses and creates file .ob .ent .ext files*/
                second_pass(file_name, &label_tablet,code_image,data_image, &IC, &DC,&missing_labels);
            }
        }
    }
    return 0;
}
