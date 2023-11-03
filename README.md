# SimOS - Operating System Simulator
This consists of CPU Scheduling and its implementation in the SimOS along with the explanation of the updates made in process.c. Also covers the understanding of demand paging and loader and its implementation in paging.c and loader.c.

CONTENTS OF THIS FILE
---------------------

* System configuration and compiler specification
* Information and Files related to CPU Scheduling
* Information and Files related to Simple paging 
* Information and Files related to Demand paging 
* Information and Files related to loader for Simple and Demand paging
* Additional information and Files



System Configuration and compiler specification
-----------------------------------------------
- All the source code files have been written in of C and compiled with Makefile attached to the zip file.
- The Makefile consists of the C specification and different targets used for different task.
- Command required is 'make'


Information and Files related to CPU scheduling
---------------------------------------
"process.c"

- To compile "process.c" please use the target "process.o" of the Makefile (shell command - make process). This creates the "simos.exe" that can be executed.


Information and Files related to Simple paging
---------------------------------------
"paging.c"

- To compile "paging.c" please use the target "paging.o" of the Makefile (shell command - make process). This creates the "simos.exe" that can be executed.

Information and Files related to Demand paging
---------------------------------------
"paging.c"

- To compile "paging.c" please use the target "paging.o" of the Makefile (shell command - make process). This creates the "simos.exe" that can be executed.

Information and Files related to loader for Simple and Demand paging
---------------------------------------
“loader.c”

- To compile "loader.c" please use the target "loader.o" of the Makefile (shell command make - load). This creates the "simos.exe" that can be executed.


Additional information and Files
--------------------------------
- Makefile consists of many targets for compiling files for different tasks.

- The source code files "memory_final.c" have also been attached as there were some changes made to them same.

“admin.c”

- To compile "admin.c" please use the target "admin.o" of the Makefile (shell command make - load). This creates the "simos.exe" that can be executed.

simos.h

- It does not require any recompilation.
