#include "sim.h"

Instruction memory[MEM_SIZE];
PCB         pcbTabela[MAX_PROCS];
int         tempo      = 0;
int         runningIdx = -1;
int         nextPid    = 1;
Fila        prontos;
Fila        bloqueados;
