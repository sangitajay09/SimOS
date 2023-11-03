#include <pthread.h>
#include <semaphore.h>
#include "simos.h"


int currentPid = 2;    // user pid should start from 2, pid=0/1 are OS/idle
int numUserProcess = 0; 

//============================================
// context switch, switch in or out a process pid
//============================================

void context_in (int pid)
{ CPU.Pid = pid;
  CPU.PC = PCB[pid]->PC;
  CPU.AC = PCB[pid]->AC;
  CPU.PTptr = PCB[pid]->PTptr;
  CPU.exeStatus = PCB[pid]->exeStatus;
}

void context_out (int pid)
{ PCB[pid]->PC = CPU.PC;
  PCB[pid]->AC = CPU.AC;
  PCB[pid]->exeStatus = CPU.exeStatus;
}

//=========================================================================
// ready queue management
// Implemented as a linked list with head and tail pointers
// The ready queue needs to be protected in case insertion comes from
// process submission and removal from process execution
//=========================================================================

#define nullReady 0
   // when get_ready_process encoutered empty queue, nullReady is returned

typedef struct ReadyNodeStruct
{ int pid;
  struct ReadyNodeStruct *next;
} ReadyNode;

ReadyNode *readyHead = NULL;
ReadyNode *readyTail = NULL;
ReadyNode *readyHead2 = NULL;
ReadyNode *readyTail2 = NULL;
ReadyNode *readyHead3 = NULL;
ReadyNode *readyTail3 = NULL;
ReadyNode *readyHead4 = NULL;
ReadyNode *readyTail4 = NULL;

void init_level_ptrarry ()
{ level = (typeLevel **) malloc (maxProcess*addrSize); }

void insert_ready_process (int pid)
{ ReadyNode *node;
  int l;
  node = (ReadyNode *) malloc (sizeof (ReadyNode));
  node->pid = pid;
  l = level[pid]->l;
  node->next = NULL;
  level[pid]->waitTime = CPU.numCycles;

  switch (l)
  { case 1:
      if (readyTail == NULL) // readyHead would be NULL also
        { readyTail = node; readyHead = node; }
      else // insert to tail
        { readyTail->next = node; readyTail = node; }
      break;
    case 2:
      if (readyTail2 == NULL) // readyHead would be NULL also
        { readyTail2 = node; readyHead2 = node; }
      else // insert to tail
        { readyTail2->next = node; readyTail2 = node; }
      break;
    case 3:
      if (readyTail3 == NULL) // readyHead would be NULL also
        { readyTail3 = node; readyHead3 = node; }
      else // insert to tail
        { readyTail3->next = node; readyTail3 = node; }
      break;
    case 4:
      if (readyTail4 == NULL) // readyHead would be NULL also
        { readyTail4 = node; readyHead4 = node; }
      else // insert to tail
        { readyTail4->next = node; readyTail4 = node; }
      break;
    default:
      fprintf (infF, "No level?");
  }
}

int get_ready_process ()
{ ReadyNode *rnode;
  int pid;
  
  check_wait_time ();
  if (readyHead != NULL)
  { pid = readyHead->pid;
    rnode = readyHead;
    readyHead = rnode->next;
    free (rnode);
    fprintf (infF, "Getting process %d from level %d\n", pid, level[pid]->l);
    if (readyHead == NULL) readyTail = NULL;
    return (pid);
  }
  else if (readyHead2 != NULL)
  { pid = readyHead2->pid;
    rnode = readyHead2;
    readyHead2 = rnode->next;
    free (rnode);
    fprintf (infF, "Getting process %d from level %d\n", pid, level[pid]->l);
    if (readyHead2 == NULL) readyTail2 = NULL;  
    return (pid);
  }
  else if (readyHead3 != NULL)
  { pid = readyHead3->pid;
    rnode = readyHead3;
    readyHead3 = rnode->next;
    free (rnode);
    fprintf (infF, "Getting process %d from level %d\n", pid, level[pid]->l);
    if (readyHead3 == NULL) readyTail3 = NULL;  
    return (pid);
  }
  else if (readyHead4 != NULL)
  { pid = readyHead4->pid;
    rnode = readyHead4;
    readyHead4 = rnode->next;
    free (rnode);
    fprintf (infF, "Getting process %d from level %d\n", pid, level[pid]->l);
    if (readyHead4 == NULL) readyTail4 = NULL;  
    return (pid);
  }
  else
  { if (cpuDebug) fprintf (bugF, "No ready process now!!!\n");
    return (nullReady);
  }
  
}

void check_wait_time ()
{ ReadyNode *node; 
  int pid, l, waitTime;
  
  node = readyHead2;
  if (node != NULL)
  { waitTime = CPU.numCycles - level[node->pid]->waitTime;
    l = level[node->pid]->l;
  }

  while (node != NULL && waitTime >= cpuQuantum * 4)
  { pid = node->pid;
    readyHead2 = node->next;
    free (node);
    level[pid]->l = l - 1;
    fprintf (infF, "2 time quantums have expired for process %d at the current level, moving the process up to level %d\n", pid, level[pid]->l);
    insert_ready_process (pid);
    node = readyHead2;
    if (readyHead2 == NULL) readyTail2 = NULL;
    else waitTime = CPU.numCycles - level[node->pid]->waitTime;
  }
  
  node = readyHead3;
  if (node != NULL)
  { waitTime = CPU.numCycles - level[node->pid]->waitTime;
    l = level[node->pid]->l;
  }

  while (node != NULL && waitTime >= cpuQuantum * 6)
  { pid = node->pid;
    readyHead3 = node->next;
    free (node);
    level[pid]->l = l - 1;
    fprintf (infF, "2 time quantums have expired for process %d at the current level, moving the process up to level %d\n", pid, level[pid]->l);
    insert_ready_process (pid);
    node = readyHead3;
    if (readyHead3 == NULL) readyTail3 = NULL;
    else waitTime = CPU.numCycles - level[node->pid]->waitTime;
  }

  node = readyHead4;
  if (node != NULL)
  { waitTime = CPU.numCycles - level[node->pid]->waitTime;
    l = level[node->pid]->l;
  }

  while (node != NULL && waitTime >= cpuQuantum * 8)
  { pid = node->pid;
    readyHead4 = node->next;
    free (node);
    level[pid]->l = l - 1;
    fprintf (infF, "2 time quantums have expired for process %d at the current level, moving the process up to level %d\n", pid, level[pid]->l);
    insert_ready_process (pid);
    node = readyHead4;
    if (readyHead4 == NULL) readyTail4 = NULL;
    else waitTime = CPU.numCycles - level[node->pid]->waitTime;
  }
}

void dump_MLFQ (FILE *outf)
{ ReadyNode *node;

  fprintf (outf, "******************** Ready Queue Dump\n");
  fprintf (outf, "Level 1: ");
  node = readyHead;
  while (node != NULL)
  { fprintf (outf, "%d, ", node->pid); node = node->next; }
  fprintf (outf, "\n");
  fprintf (outf, "Level 2: ");
  node = readyHead2;
  while (node != NULL)
  { fprintf (outf, "%d, ", node->pid); node = node->next; }
  fprintf (outf, "\n");
  fprintf (outf, "Level 3: ");
  node = readyHead3;
  while (node != NULL)
  { fprintf (outf, "%d, ", node->pid); node = node->next; }
  fprintf (outf, "\n");
  fprintf (outf, "Level 4: ");
  node = readyHead4;
  while (node != NULL)
  { fprintf (outf, "%d, ", node->pid); node = node->next; }
  fprintf (outf, "\n");
}


//=========================================================================
// endIO list management
// processes that has finished waiting can be inserted into endIO list
//   -- when adding process to endIO list, should set endIO interrupt,
//      interrupt handler will move processes in endIO list to ready queue
// The list needs to be protected because multiple threads may insert
// to endIO list and a thread will remove nodes in the list concurrently
//=========================================================================

sem_t pmutex;

typedef struct EndIOnodeStruct
{ int pid;
  struct EndIOnodeStruct *next;
} EndIOnode;

EndIOnode *endIOhead = NULL;
EndIOnode *endIOtail = NULL;

void insert_endIO_list (int pid)
{ EndIOnode *node;
  
  sem_wait (&pmutex);
  node = (EndIOnode *) malloc (sizeof (EndIOnode));
  node->pid = pid;
  node->next = NULL;
  if (endIOtail == NULL) // endIOhead would be NULL also
    { endIOtail = node; endIOhead = node; }
  else // insert to tail
    { endIOtail->next = node; endIOtail = node; }
  sem_post (&pmutex);
}

// move all processes in endIO list to ready queue, empty the list
// need to set exeStatus from eWait to eReady

void endIO_moveto_ready ()
{ EndIOnode *node;
  
  sem_wait (&pmutex);
  while (endIOhead != NULL)
  { node = endIOhead;
    
    insert_ready_process (node->pid);
    PCB[node->pid]->exeStatus = eReady;
    endIOhead = node->next;
    free (node);
  }
  endIOtail = NULL;
  sem_post (&pmutex);
}

void dump_endIO_list (FILE *outf)
{ EndIOnode *node;

  node = endIOhead;
  fprintf (outf, "endIO List = ");
  while (node != NULL)
    { fprintf (outf, "%d, ", node->pid); node = node->next; }
  fprintf (outf, "\n");
}

//=========================================================================
// Some support functions for PCB 
// PCB related definitions are in simos.h
//=========================================================================

void init_PCB_ptrarry ()
{ PCB = (typePCB **) malloc (maxProcess*addrSize); }

int new_PCB ()
{ int pid;

  pid = currentPid;
  currentPid++;
  if (pid >= maxProcess)
  { fprintf (infF, "\aExceeding maximum number of processes: %d\n", pid);
    // because we do not reuse pid, pid may run out, use max to
    // protect against potential integer overflow (though very unlikely)
    return (-1);
  }
  PCB[pid] = (typePCB *) malloc ( sizeof(typePCB) );
  PCB[pid]->Pid = pid;
  PCB[pid]->timeUsed = 0;
  PCB[pid]->numPF = 0;
  return (pid);
}

void free_PCB (int pid)
{
  free (PCB[pid]);
  if (cpuDebug) fprintf (bugF, "Free PCB: %d\n", PCB[pid]);
  PCB[pid] = NULL;
}

void dump_PCB (FILE *outf, int pid)
{
  fprintf (outf, "******************** PCB Dump for Process %d\n", pid);
  fprintf (outf, "Pid = %d\n", PCB[pid]->Pid);
  fprintf (outf, "PC = %d\n", PCB[pid]->PC);
  fprintf (outf, "AC = "mdOutFormat"\n", PCB[pid]->AC);
  fprintf (outf, "PTptr = %x\n", PCB[pid]->PTptr);
  fprintf (outf, "exeStatus = %d\n", PCB[pid]->exeStatus);
}

void dump_PCB_list (FILE *outf)
{ int pid;

  fprintf (outf, "Dump all PCB: From 0 to %d\n", currentPid);
  for (pid=idlePid; pid<currentPid; pid++)
    if (PCB[pid] != NULL) dump_PCB (outf, pid);
}

void dump_PCB_memory (FILE *outf)
{ int pid;

  fprintf (outf,
           "Dump memory/swap of all processes: From 1 to %d\n", currentPid-1);
  // dump_process_memory (outf, idlePid);
  for (pid=idlePid+1; pid<currentPid; pid++)
    if (PCB[pid] != NULL) dump_process_memory (outf, pid);
}


//=========================================================================
// process management
//=========================================================================

//#include "idle.c"

void clean_process (int pid)
{
  free_process_memory (pid);
  free_PCB (pid);  // PCB has to be freed last, other frees use PCB info
} 

void exiting_process (int pid)
{ PCB[pid]->exeStatus = CPU.exeStatus;
  // PCB[pid] is not updated, no point to do a full context switch

  // send exiting process printout to term.c, str will be freed by term.c
  char *str = (char *) malloc (80);
  if (CPU.exeStatus == eError)
  { fprintf (infF, "\aProcess %d has an error, dumping its states\n", pid);
    dump_PCB (infF, pid);
    dump_process_memory (infF, pid); 
    sprintf (str, "Process %d had encountered error in execution!!!\n", pid);
  }
  else  // was eEnd
  { fprintf (infF, "Process %d had completed successfully: Time=%d, PF=%d\n",
             pid, PCB[pid]->timeUsed, PCB[pid]->numPF);
    sprintf (str, "Process %d had completed successfully: Time=%d, PF=%d\n",
             pid, PCB[pid]->timeUsed, PCB[pid]->numPF);
  }
  insert_termIO (pid, str, exitProgIO);

  // invoke io to print str, process has terminated, so no wait state

  numUserProcess--;
  clean_process (pid); 
    // cpu will clean up process pid without waiting for printing to finish
    // so, io should not access PCB[pid] for end process printing
}

void initialize_process_manager ()
{
  init_PCB_ptrarry ();
  init_level_ptrarry ();

  currentPid = 2;  // the next pid value to be used
  numUserProcess = 0;  // the actual number of processes in the system

  init_idle_process ();
  sem_init (&pmutex, 0, 1);
}

//================================================================
// submit_process always works on a new pid and the new pid will not be 
// used by anyone else till submit_process finishes working on it
// currentPid is not used by anyone else but the dump functions
// So, no conflict for PCB and Pid related data
// -----------------
// During insert_ready_process, there is potential of conflict accesses
//================================================================

int submit_process (char *fname)
{ int pid, ret, i;
  
  // if there are too many processes s.t. each cannot get sufficient memory
  // then reject the process
  if ( ((numFrames-OSpages)/(numUserProcess+1)) < 2 )
    fprintf (infF, 
  "\aToo many processes => they may not execute properly due to page faults\n");
  else
  { pid = new_PCB ();
    if (pid > idlePid)
    { ret = load_process (pid, fname);   // return #pages loaded
      if (ret > 0)  // loaded successfully
      { PCB[pid]->PC = 0;
        PCB[pid]->AC = 0;
        PCB[pid]->exeStatus = eReady;
        level[pid] = (typeLevel *) malloc ( sizeof(typeLevel) );
	level[pid]->l = 1;
	// swap manager will put the process to endIO list and then
        // process.c will eventually move it to ready queue
        // at this point, the process may not be loaded yet, but no problem
        numUserProcess++;
        return (pid);  // the only case of successful process creation
      }
      // else new_PCB returned -1, PCB has not been created
  } }
  // failed, PCB has not been created, exitProg
  char *str = (char *) malloc (80);
  fprintf (infF, "\aProgram %s has loading problem!!!\n", fname);
  sprintf (str, "Program %s has loading problem!!!\n", fname);
  insert_termIO (pid, str, exitProgIO);
  return (-1);
}

//================================================================
// execute_process: prepare; execute instruction; subsequent processing
// -----------------
// During insert_ready_process, there is potential of conflict accesses
//================================================================

void execute_process (FILE *outf)
{ int pid, l, intime;
  genericPtr event;
  pid = get_ready_process ();
  if (pid != nullReady)
    // execute the ready process (with pid# = pid)
    //   before and after the execution, need to do:
    //   (1) context switch,
    //   (2) set execution status + check status to do subsequent actions
    //   (3) set timer to stop execution at the time quantum
    //   (4) accounting: add execution time to PCB[?]->timeUsed,
  { context_in (pid);   // === (1) 
    CPU.exeStatus = eRun;   // === (2) 
    intime = CPU.numCycles;   // ===(4) 
    l = level[pid]->l;
    event = add_timer (cpuQuantum * l, CPU.Pid,  // == (3) 
                       actTQinterrupt, oneTimeTimer);
    cpu_execution ();
    context_out (pid);  // === (1)
    
    PCB[pid]->timeUsed += (CPU.numCycles - intime);  // ===(4)
    if (CPU.exeStatus == eReady) 
    { 
      if (l < 4)
      { l = level[pid]->l;
        l++;
        level[pid]->l = l;
      }
      
      insert_ready_process (pid);  // === (2)
    }
    else if (CPU.exeStatus == ePFault || CPU.exeStatus == eWait) 
      // eWait: should have been handled by instruction execution
      // ePFault: calculate_memory_address should have set pFaultException,
      //   which is subsequently handled by page_fault_handler
      deactivate_timer (event);
    else // CPU.exeStatus == eError or eEnd, exiting
    {
	    exiting_process (pid);
	    deactivate_timer (event); 
    }
    // Why deactivate_timer?
    //   If exeStatus != eReady ==> process is not stopped by time quantum
    //   but timer is still there ==> should be deactivated
    //   To deactivate: need the returned ptr from set timer: "event"
    //   If not: it will impact the execution of the next process
    // But if time quantum just expires and exeStatus != eReady
    //   No problem! eReady is only set upon tqInterrupt (in cpu.c)
    //    if the process had ePFault/eWait, it will not be set to eReady
  }
  else execute_idle_process ();
    // no ready process in the system, so execute idle process
    // ===== see https://en.wikipedia.org/wiki/System_Idle_Process
}

