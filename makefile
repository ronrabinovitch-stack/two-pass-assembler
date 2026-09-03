main: main.o Second_Pass.o First_Pass.o Pre_Assembler.o Commands.o Helpers.o List.o
	gcc -ansi -Wall -pedantic main.o Second_Pass.o First_Pass.o Pre_Assembler.o Commands.o Helpers.o List.o -o main


main.o: main.c main.h Pre_Assembler.h
	gcc -c -ansi -Wall -pedantic main.c -o main.o



Second_Pass.o: Second_Pass.c second_pass.h First_Pass.h
	gcc -c -ansi -Wall -pedantic Second_Pass.c -o Second_Pass.o 



First_Pass.o: First_Pass.c First_Pass.h main.h
	gcc -c -ansi -Wall -pedantic First_Pass.c -o First_Pass.o



Pre_Assembler.o: Pre_Assembler.c Pre_Assembler.h 
	gcc -c -ansi -Wall -pedantic Pre_Assembler.c -o Pre_Assembler.o
	


Commands.o: Commands.c Commands.h
	gcc -c -ansi -Wall -pedantic Commands.c -o Commands.o



Helpers.o: Helpers.c main.h
	gcc -c -ansi -Wall -pedantic Helpers.c -o Helpers.o


	
List.o: List.c Pre_Assembler.h second_pass.h
	gcc -c -ansi -Wall -pedantic List.c -o List.o
