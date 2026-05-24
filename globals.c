#include "sim.h"
ProcessoPendente pendentes[MAX_PENDENTES];
Instruction memory[MEM_SIZE];
PCB         pcbTabela[MAX_PROCS];
int numPendentes = 0;
int         tempo      = 0;
int         runningIdx = -1;
int         nextPid    = 1;
Fila        prontos;
Fila        bloqueados;
