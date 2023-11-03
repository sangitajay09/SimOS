#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simos.h"

#define freeFrame 1
#define usedFrame 0

// For Demand paging
#define dirtyFrame 1
#define tidyFrame 0

// Page replacement policy - aging
#define zeroAge 0x01000000
#define highestAge 0x80000000

#define nullIndex -1
#define nullPage -1
#define tailPointer -1

// Types of pages required to detect a page fault or lazy write back
#define diskPage -2
#define waitingPage -3
// PageOffset and shiftPageNumber
#define flagRead 1
#define flagWrite 2
// Uncomment the following two lines to realize Frame Locking
// #define lockedFrames
// #define unlockedFrames
int headOfFrames, tailOfFrames;

typedef unsigned aType;

typedef struct
{ int pid;
  // To refer to the next node
  int page;
  int next;
  int free;             // To realize whether the frame is available to write
  int dirty;            // Verifying prior to accessing if the memory is stale
  // Need to figure out a suitable datatype for age
  aType age;            // Tracking its age

} memFrameStruct;
memFrameStruct *mFrame;

int fetch_pageNumber(int logicalAddress){
    // Gets page number for the process
    return (logicalAddress/pageSize);
}

int calculate_offSet(int logicalAddress){
    // Calculating the required offset while deciphering logical add to physical address
    return logicalAddress % pageSize;
}

int calculate_memory_address(unsigned offset, int rwflag){
    // Calculating frame number from page number and offset
    // Depending on rwflag, decide it is going to read or write
    int pageNum;
    
    if (offset == 0) { pageNum = 0; }
    else { pageNum = fetch_pageNumber(offset); }
    fprintf(infF, "Generated page number : %d\n", pageNum);

    if(pageNum < maxPpages){
        int frameNum = CPU.PTptr[pageNum];
        // We can dump the information about one frame
        // TODO - Reading from an empty frame should cause access violation
        if (frameNum == nullPage && rwflag == flagRead) { 
            return mError; 
        }
        if (frameNum == diskPage || frameNum == waitingPage)
        {
            // Page is in the swap-space, yet to be brought in the memory
            // AKA Page Fault
            set_interrupt(pFaultException);
            return mPFault;
        }
        if (rwflag == flagWrite){   
            if (frameNum == nullPage){
                frameNum = get_free_frame();
                update_frame_info (frameNum, CPU.Pid, pageNum);
                update_process_pagetable (CPU.Pid, pageNum, frameNum);
            }
            else{ 
                mFrame[frameNum].dirty = dirtyFrame; 
            }
            // Calculating logical address while writing to swap-space require
            // us to change that frame to dirty
        }
        // Debug print
        mFrame[frameNum].age = highestAge;
        return frameNum * pageSize + (offset - pageNum * pageSize);
    }
    else{
        printf("ERROR : Segmentation Fault occurred!");
        return mError;
    }
}

void initialize_mframe_manager(){
    // Malloc for predetermined memFrame size
    // Initializing array of mframeSize
    mFrame = (memFrameStruct *) malloc (numFrames*sizeof(memFrameStruct));

    // Since properties of mFrame are flags, initializing them 
    // with default values

    // Since OS occupies the beginning of the memory (OSPages in simos.h),
    // separately initializing that first
    int i;
    // Initializing OS pages with OSpid
    for(i=0; i<OSpages; i++){
        mFrame[i].pid = osPid;
        //mFrame[i].page = osPid;
        mFrame[i].free = usedFrame;
        mFrame[i].age = zeroAge;
        mFrame[i].dirty = tidyFrame;
    }

    // After fixed base, initializing frames for processes
    for(i=OSpages;i<numFrames;i++){
        mFrame[i].pid = nullPid;
        mFrame[i].free = freeFrame;
        mFrame[i].age = zeroAge;
        mFrame[i].dirty = tidyFrame;

        // Keeping next Process's track with 'next' pointer (property)
        // Note that we are using a separate index as a pointer and not PID of the process
        if (i+1 < numFrames){
            // mFrame[i].next = pid+1;
            mFrame[i].next = i+1;
        }
        else{
            mFrame[i].next = tailPointer;
        }
    }
    // Keeping the track of the head and tail of the free frames
    // We will shrink this as we use more frames
    // headOfFrames and tailOfFrames are keeping track of free-list
    headOfFrames = OSpages;
    tailOfFrames = numFrames - 1;
    //add_timer (periodAgeScan, osPid, actAgeInterrupt, periodAgeScan);

}

void addto_free_frame(int indexOfFrame, int status){
    // Freeing a frame and adding to the free-frames list
    // Setting last frame's tail as a large number

    // 1. Check dirty or tidy frame
    // 2. Append to the tail of the list and make the new tail and this process

    // Setting first frame (beginning from OSPages)'s head to freeFrame's head
    //update_frame_info(indexOfFrame, pid, page);
    unsigned buf[pageSize];
    if (status == dirtyFrame){
        // This could come from page fault handler
        // Write that frame to swapQ with actWrite
        int page;
        for(page=0; page<pageSize; page++){
            if(Memory[page+indexOfFrame*pageSize].mData > 10000000){
                // TODO - mData is redundant - remove that
                buf[page] = Memory[page+indexOfFrame*pageSize].mInstr;
            }
            else{
                buf[page] = Memory[page+indexOfFrame*pageSize].mData;
            }
        }
        // Reconsidering actWrite and Nothing
        insert_swapQ(CPU.Pid, mFrame[indexOfFrame].page, buf, actWrite, Nothing);
    }
    if(tailOfFrames == nullIndex && headOfFrames == nullIndex){
        headOfFrames = indexOfFrame;
    }
    else{
        // Appending the tidy frame to the tail and keeping the status freeFrame
        mFrame[tailOfFrames].next = indexOfFrame;
        mFrame[indexOfFrame].free = freeFrame;
    }
    tailOfFrames = indexOfFrame;
}
void update_frame_info(int indexOfFrame, int pid, int page){
    // Updating frame meta data while adding a new frame
    // Loader will also update frame meta data using this function
    mFrame[indexOfFrame].pid = pid;
    mFrame[indexOfFrame].page = page;

    // Resetting and freeing using this function (pid = nullPid, page = nullPage or nullIndex) 
    // if(pid == nullPid && (page == nullPage || page == nullIndex)){
    //     mFrame[indexOfFrame].free = freeFrame;
    // }
    // else{
    mFrame[indexOfFrame].free = usedFrame;
    mFrame[indexOfFrame].next = nullIndex; 
    //}
}
     
int get_free_frame(){
    // Since we use contagious memory blocks, headofFrames represents the head of the free pages
    // And similarly, tailOfFrames represents the tail of the free pages
    // Fetches available frames (head)
    int indexOfFreeFrame;

    // if (mFrame[headOfFrames].free != freeFrame){
    //     fprintf(infF, "ERROR : Head of Free frames should always be nullIndex");
    //     return nullIndex;
    // }
    if (headOfFrames != nullIndex){
        indexOfFreeFrame = headOfFrames;
        headOfFrames = mFrame[headOfFrames].next;
        return indexOfFreeFrame;
    }
    else{
        return select_agest_frame();
    }
}

void init_process_pagetable (int pid){
    // Initializing process pages for pid (argument)
    int i;
    PCB[pid]->PTptr = (int *) malloc (maxPpages);
    for (i=0; i<maxPpages; i++){
        PCB[pid]->PTptr[i] = nullPage;
    }
    fprintf(infF, "Process pagetable is initialized\n");
    dump_process_pagetable (infF, pid);
}

void update_process_pagetable (int pid, int page, int frame){
    // Sets a page type - if it is in the memory, swapSpace or it is an unused page
    fprintf(infF, "Entered update_process_pagetable : %d\n", frame);
    PCB[pid]->PTptr[page] = frame;
}

void free_process_memory (int pid){
    // Once a process is terminated, free pages associated with that process
    int index = 0;
    int frameNum;
    for(index=0; index<maxPpages; index++){
        frameNum = PCB[pid]->PTptr[index];
        if(frameNum > 0 && frameNum < numFrames){
            mFrame[frameNum].dirty = tidyFrame;
            mFrame[frameNum].age = zeroAge;
            mFrame[frameNum].free = freeFrame;
            mFrame[frameNum].pid = nullPid;
        }
    }
}

int indexSwap = 2;
unsigned readBufGlobe[32];
void swap_in_page (int pidin, int pagein, int finishact){

    // 1. Get free frames using get_free_frame()
    // 1a - If no free frames are available, call select_agest_frame()
    fprintf(infF, "---------------------\n");
    fprintf(infF, "Inside swap_in_page()\n");
    fprintf(infF, "---------------------\n");
    mType *writeBuf = (mType*) malloc (pageSize*sizeof(mType));

    // If we are to read the page directly in the memory when we get a free frame
    int index;
    int newFrame = get_free_frame();

    fprintf(infF, "Finish action is %d\n", finishact);
    if (newFrame == nullIndex){
        newFrame = select_agest_frame();
        
        int pidout = mFrame[newFrame].pid;
        
        int pageout = mFrame[newFrame].page;
        if (mFrame[newFrame].dirty == dirtyFrame){
            // Since dirty, write it back to swap-space
            for(index=0; index<pageSize; index++){
                writeBuf[index] = Memory[newFrame*pageSize+index];
            }
            // 2. Update frame information using update_frame_info()
            // Hard coding pendingPage to -3
            update_process_pagetable(pidout, pageout, -3);
            // 2a. Write it to swap-space
            insert_swapQ(pidout, pageout, (unsigned *) writeBuf, actWrite, freeBuf);
        }
    }
    else{
        fprintf(infF, "Directly getting a free frame ... %d\n, newFrame");
    }
    // 3. Fetch the page from swap space to memory
    fprintf(infF, "Fetching pid->page = %d->%d to memory \n", pidin, pagein);
    
    fprintf(infF, "Size check for memory - \n");
    dump_process_pagetable (infF, pidin);
    //insert_swapQ(pidin, pagein, (unsigned int *) &Memory[indexSwap*pageSize], actRead, finishact);
    fprintf(infF, "Using Global buffer\n");
    insert_swapQ(pidin, pagein, readBufGlobe, actRead, finishact);
    usleep (diskRWtime);
    for(indexSwap=0; indexSwap<32; indexSwap ++){
        fprintf(infF, "Global buffer %d\n", readBufGlobe[indexSwap]);
    }
    fprintf(infF, "Post insert_swapQ call - \n");
    // 4. Updating the respective frame information and process table
    dump_process_pagetable (infF, pidin);
    for (page=0; page<=pageSize; page++){
        // Check whether it is an instruction (MSB = 1) or data
        if(buf[page] > 10000000){
            Memory[page+newFrame*pageSize].mInstr = readBufGlobe[page];
        }
        else{
            Memory[page+newFrame*pageSize].mData = readBufGlobe[page];
        }
    }
    free(writeBuf);
    fprintf(infF, "Before exiting ---\n");
    for(indexSwap=0; indexSwap<32; indexSwap ++){
        fprintf(infF, "Global buffer %d\n", readBufGlobe[indexSwap]);
    }
}

// Page table management functions for each processes
void dump_process_pagetable (FILE *outf, int pid){
    // TODO - Need to utilize the file descriptor

    int page;
    fprintf(infF, "-------------------------------------------------\n");
    fprintf(infF, "Dumping pagetable for process - pid: %d \n", pid);
    fprintf(infF, "-------------------------------------------------\n");
    for(page=0; page<maxPpages; page++){
        fprintf(infF, "%d \n", PCB[pid]->PTptr[page]);
    }
}
void dump_process_memory (FILE *outf, int pid){

    int page, frame;
    fprintf(infF, "-------------------------------------------------\n");
    fprintf(infF, "Dumping process memory for each frame - pid: %d \n", pid);
    fprintf(infF, "-------------------------------------------------\n");
    for(page=0; page<maxPpages; page++){
        frame = PCB[pid]->PTptr[page];
        fprintf(infF, "Printing the properties of a frame - \n");
        fprintf(infF, "Process id : PID = %d \n", mFrame[frame].pid);
        fprintf(infF, "Page = %d \n", mFrame[frame].page);
        fprintf(infF, "Free = %d \n", mFrame[frame].free);
    }
}
void dump_memory (FILE *outf){
    // For each page and each frame print out the content
    int page, frame, logicalAdd;
    for(page=0; page<numFrames; page++){
        for(frame=0; frame<pageSize; frame++){
            logicalAdd = page*pageSize + frame;
            // Print about flags, and instruction/data content
            fprintf(infF, "Location at memory frame %d \n",page);
            fprintf(infF, "Frame properties ------------------------------------------------------>\n");
            fprintf(infF, "Pid: %d \t Page: %d \t Next: %d \t Free: %d \t Dirty: %d \t Age: %x \n",
            mFrame[page].pid, mFrame[page].page, mFrame[page].next, mFrame[page].free, mFrame[page].dirty, mFrame[page].age);
            fprintf(infF, "Respective Data %.2f and instruction %d \n",Memory[logicalAdd].mData, Memory[logicalAdd].mInstr);
        }
    }
}
void memory_agescan(){
    // Aging policy for Demand Paging
    int index;
    for(index=OSpages; index<numFrames; index++){
        // 1. If a frame is free, no action is required
        if(mFrame[index].free == usedFrame){
            // If a frame is used, shift-right its age by 1
            if(mFrame[index].age != 0x0000000){
                mFrame[index].age = mFrame[index].age >> 1;
                // 2. When age of a frame reaches zeroAge, return it to the free list
                if (mFrame[index].age == 0x0000000){

                    // 3. In case the frame is dirty, write it back to the swap-space
                    // Taking care of this part in addto_free_frame() function itself
                        // Lazy write-back - only execute swap-out when the freed frame is allocated
                        // to another process
                    // 4. Update the process table accordingly
                    addto_free_frame(index, mFrame[index].dirty);
                }
            }
        }
    }
}
int select_agest_frame(){
    // Returns the most aged frame (lowest age number)
    
    // 1. For all frames with zeroAge, push them to free list using addto_free_frames()
    int page;
    int oldestAge = highestAge;
    int agestFrames = 0;
    for(page=OSpages; page<numFrames; page++){
        if(mFrame[page].age == 0){
            addto_free_frame(page, mFrame[page].dirty);
            return page;
        }
        // 2. Iterate through frames and store the age of agest frame (smallest number), call it oldestAge
        if(mFrame[page].age <= oldestAge){
            if(mFrame[page].age == oldestAge){
                agestFrames = 0;
            }
            oldestAge = mFrame[page].age;
            agestFrames++;
        }
    }
    // 3. Add frames with oldestAge to free list using addto_free_list()
    for(page=OSpages; page<numFrames; page++){
        if(mFrame[page].age == oldestAge){
            addto_free_frame(page, mFrame[page].dirty);
            return page;
        }
    }
    // Freeing every frame with oldestAge and returning the last
    //return page;
}
void page_fault_handler(){
    // 1. Get a free frame
    int newFrame = get_free_frame();
    int faultyPage;
    int page;
    // 2. Verify where in pagetable a page fault has occurred
    if (newFrame < 0){
        // Safety check for no free frames
        return mError;
    }
    for(page=0; page<maxPpages; page++){
        if(PCB[CPU.Pid]->PTptr[page] == diskPage){
            faultyPage = page;
            // Considering the first encounter of diskPage
            break;
        }
    }
    // 3. Call insert_swapQ for that page
    unsigned buf[pageSize];
    // Rethink about the finishact parameter
    insert_swapQ(CPU.Pid, faultyPage, buf, actRead, 10);
    fprintf(infF, "Line 442 - Page Fault Handler - \n");
    for (page=0; page<=pageSize; page++){
        // Check whether it is an instruction (MSB = 1) or data
        if(buf[page] > 10000000){
            Memory[page+newFrame*pageSize].mInstr = buf[page];
        }
        else{
            Memory[page+newFrame*pageSize].mData = buf[page];
        }
    }
    // 4. Update the respective page table
    update_process_pagetable(CPU.Pid, faultyPage, newFrame);
}
