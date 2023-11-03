#include "simos.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#define opcodeShift 24
#define operandMask 0x00ffffff
#define diskPage -2

int load_process(int pid, char *fname)
{   printf("INside the load_process\n");
    fflush(stdout);
    int progStatus = 0;
    int ret = init_programfile(fname); //
    //Calls load_process_to_swap
    if(ret == 3) {
        int no_of_pages = load_process_to_swap(pid, fname);
        printf("No of pages %d\n", no_of_pages);
        fflush(stdout);
        if (no_of_pages > 0) {
            load_pages_to_memory(pid, no_of_pages);
            progStatus = progNormal;
        } else { progStatus = progError; }
    }
    else {progStatus = progError;}


    return (progStatus);
}
void load_pages_to_memory(int pid, int no_of_pages){
    int i = 0;
    int ori_no_of_pages = 0;
    int *frameNum = (int *) malloc (no_of_pages*sizeof(int));

    //unsigned *temp = (unsigned *) malloc (pageSize*sizeof(unsigned));
    printf("after the memory alloc in memory\n");
    fflush(stdout);

    printf("load pages %d\n", loadPpages);
    fflush(stdout);
    ori_no_of_pages = no_of_pages;
    printf("ori_no_of_pages: %d\n",ori_no_of_pages);
    while (i<ori_no_of_pages){
        printf("swap dump i:%d\n",i);
        dump_process_swap_page(infF,pid,i);
        i++;
    }
    i=0;
    if(no_of_pages > loadPpages) {no_of_pages = loadPpages;}

    while (i < no_of_pages-1) {
        printf("in memory page no: %d\n",i);
        fflush(stdout);
        swap_in_page(pid, i, Nothing);
        dump_process_pagetable(infF,pid);
        dump_process_swap_page(infF,pid,i);

        i++;
    }
    printf("last page in memory: %d\n",i);
    fflush(stdout);
    swap_in_page(pid, no_of_pages-1, toReady);
    dump_process_pagetable(infF,pid);
    dump_process_swap_page(infF,pid,no_of_pages-1);

}


int load_instruction (FILE *file, int page, int offset)
{
    int opcode,operand;
    int addr = (page * pageSize)+offset;
    fscanf (file, "%d %d\n", &opcode, &operand);
    printf ("load instruction: %d, %d\n",opcode, operand);
    opcode = opcode << opcodeShift;
    operand = operand & operandMask;
    printf("addr %d, instr %d\n",addr,opcode | operand);
    return (opcode | operand);
}

float load_data (FILE *file, int page, int offset)
{
    float data;
    int addr = (page * pageSize)+offset;
    fscanf (file, "%f\n", &data);
    printf ("load data: %.2f\n", data);
    return (data);
}

int load_process_to_swap(int pid,char *fname){
    //Read program instructions and data
    printf("INside the load_process_to_swap\n");
    fflush(stdout);


    FILE *file = fopen(fname, "r");
    if(!file){
        printf("cant open file\n");
        fflush(stdout);
        return;

    }
    printf("after open file in swap\n");
    fflush(stdout);
    init_process_pagetable(pid);

    // Line by line for loop
    int i;
    int j;
    char *line =NULL;

    int total_size = 0;
    int data_size = 0;
    int instr_size = 0;
    char *token = NULL;
    int a[3];
    int len;
    int numpage;
    int ret_load_ins;
    int ret_load_data;
    int page = NULL;
    int offset;
    int empty_offset;
    int empty_page;
    char l[100];
    int PID = pid;
    float temp_data;
    printf("before the memory alloc... swap\n");
    fflush(stdout);

    unsigned temp[pageSize];
    unsigned temp2[pageSize];
    int frame;
    printf("after the buffer initialisation in...swap\n");
    fflush(stdout);
    fgets(l, 100, file);

    fputs(l, stdout);
    token = strtok(l," ");
    total_size = (strtol(token,NULL,10));
    token = strtok(NULL," ");
    instr_size = strtol(token,NULL,10);
    token = strtok(NULL," ");
    data_size = strtol(token,NULL,10);
    numpage = (instr_size + data_size) / pageSize + (((instr_size + data_size) % pageSize)!=0);


    //For loop for 2nt int iterations, each line 1st int=opcode, 2nd int = operand
    printf("Loading Instructions, Instructions pages %d, No of Instructions %d\n",instr_size/pageSize,instr_size);
    for (i=0; i<instr_size; i++){
        //printf("inside for ....instruction \n");
        fflush(stdout);
        page = i / pageSize;
        offset = i % pageSize;
        printf(" pages size %d,page%d, offset %d\n", pageSize, page,offset);
        fflush(stdout);
        temp[offset] = load_instruction (file, page, offset);

        if (offset == pageSize-1){ //&& (page < (instr_size / pageSize)
            int k;
            for(k=0;k<pageSize;k++){
                temp2[k]=temp[k];
            }
            dump_process_swap_page(infF,pid,page);
            insert_swapQ(pid, page, temp2 , actWrite, Nothing);
            update_process_pagetable (pid, page, -2);
            dump_process_swap_page(infF,pid,page);
            printf("page inserted in swapQ %d\n", page);
            fflush(stdout);
        }

    }
    printf("i %d\n",i);
    printf("Loading Data, Data pages %d, No of Data %d\n",data_size/pageSize,data_size);

    //For loop for 3rd int iterations, each line just data
    for (j = instr_size; j < instr_size + data_size; j++) {
        //printf("inside ......data\n");
        fflush(stdout);
        page = j / pageSize;
        offset = j % pageSize;

        printf(" pages size %d,page%d, offset %d\n", pageSize, page,offset);
        fflush(stdout);
        temp_data = load_data(file, page, offset);
        temp[offset] = *(unsigned*)&temp_data;

        if ((offset == pageSize-1)&& (page < numpage -1)){
            int k;
            for(k=0;k<pageSize;k++){
                temp2[k]=temp[k];
            }
            dump_process_swap_page(infF,pid,page);
            insert_swapQ(pid, page, temp2 , actWrite, Nothing);
            update_process_pagetable (pid, page, -2);
            dump_process_swap_page(infF,pid,page);
            printf("page inserted in swapQ %d\n", page);
            fflush(stdout);
        }
        else if ((page == numpage - 1) && (offset == (instr_size + data_size - 1) % pageSize) ){
            int k;
            for(k=0;k<pageSize;k++){
                temp2[k]=temp[k];
            }
            dump_process_swap_page(infF,pid,page);
            insert_swapQ(pid, page, temp2 , actWrite, Nothing);
            dump_process_swap_page(infF,pid,page);
            update_process_pagetable (pid, page, -2);
            printf("normal execution of last page %d\n", pageSize);
            fflush(stdout);
        }
    }
    return numpage;
}

int init_programfile(char *fname)
{   printf("inside init program file");
    fflush(stdout);

    FILE *file;
    int ret_input;
    int msize, numinstr, numdata;
    file = fopen(fname,"r");

    if (file == NULL)
    { printf ("Incorrect program name: %s!\n", fname);
        return;
    }

    ret_input = fscanf (file, "%d %d %d\n", &msize, &numinstr, &numdata);
    if (ret_input < 3)   // did not get all three inputs
    { printf ("Submission failure: Need three inputs!\n");
        fflush(stdout);

        return;
    }
    fclose(file);
    printf("file closed\n");
    fflush(stdout);

    return ret_input;
}

