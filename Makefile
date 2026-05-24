CC     = gcc
CFLAGS = -Wall -Wextra -g

OBJS = globals.o filas.o loader.o executor.o scheduler.o gestor.o report.o main.o

simulador: $(OBJS)
	$(CC)	$(CFLAGS)	-o	simulador	$(OBJS)

globals.o:   globals.c sim.h
	$(CC)	$(CFLAGS)	-c	globals.c

filas.o:     filas.c sim.h
	$(CC)	$(CFLAGS)	-c	filas.c

loader.o:    loader.c sim.h
	$(CC)	$(CFLAGS)	-c	loader.c

executor.o:  executor.c sim.h
	$(CC)	$(CFLAGS)	-c	executor.c

scheduler.o: scheduler.c sim.h
	$(CC)	$(CFLAGS)	-c	scheduler.c

gestor.o:    gestor.c sim.h
	$(CC)	$(CFLAGS)	-c	gestor.c

report.o:    report.c sim.h
	$(CC)	$(CFLAGS)	-c	report.c

main.o:      main.c sim.h
	$(CC)	$(CFLAGS)	-c	main.c

clean:
	rm -f *.o simulador
