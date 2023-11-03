simos.exe: admin.o loader.o process.o
	gcc -o simos.exe admin.o cpu.o idle.o term.o swap.o submit.o memory.o paglng.o loader.o process.o clock.o system.o -lpthread

admin.o : admin.c
	gcc -c admin.c

paging.o : paging.c
	gcc -c paging.c

loader.o : loader.c
	gcc -c loader.c

process.o : process.c
	gcc -c process.c