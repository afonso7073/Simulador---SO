#ifndef SIM_H
#define SIM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ─── MEMÓRIA ─────────────────────────────────────────── */
#define MEM_SIZE   1000   /* total de slots de instrução   */
#define MAX_PROCS   50    /* máximo de processos           */
#define NOME_MAX    15    /* tamanho máximo do nome ficheiro*/

typedef struct {
    char ins;           /* tipo: M A S B T C L            */
    int  n;             /* argumento numérico              */
    char nome[NOME_MAX];/* usado só pela instrução L       */
} Instruction;

extern Instruction memory[MEM_SIZE];

/* ─── PCB ─────────────────────────────────────────────── */
typedef enum { FREE, READY, RUNNING, BLOCKED, TERMINATED } Estado;

typedef struct {
    int   pid;
    int   ppid;
    int   prioridade;
    int   pc;           /* próxima instrução a executar    */
    int   start;        /* índice no memory[] onde começa  */
    int   size;         /* número de instruções            */
    int   valor;        /* a única variável do processo    */
    int   tempo_inicio;
    int   tempo_cpu;
    char  programa[NOME_MAX];
    Estado estado;
} PCB;

extern PCB    pcbTabela[MAX_PROCS];
extern int    tempo;        /* relógio global da simulação */
extern int    runningIdx;   /* índice em pcbTabela do processo a correr */
extern int    nextPid;      /* próximo PID a atribuir      */

/* ─── FILAS ───────────────────────────────────────────── */
#define FILA_MAX 50

typedef struct {
    int dados[FILA_MAX];
    int head, tail, count;
} Fila;

extern Fila prontos;
extern Fila bloqueados;


#define MAX_PENDENTES 50

typedef struct {
    char ficheiro[NOME_MAX];
    int  chegada;
    int  prioridade;
} ProcessoPendente;

extern ProcessoPendente pendentes[MAX_PENDENTES];
extern int numPendentes;
/* ─── PROTÓTIPOS ──────────────────────────────────────── */

/* loader.c */
int  carregarPrograma(const char *ficheiro);   /* devolve start em memory[], -1 se erro */
int  programaEmMemoria(const char *ficheiro);  /* devolve start se já está, -1 se não   */
void libertarMemoria(int start, int size);

/* filas.c */
void filaInit(Fila *f);
int  filaVazia(Fila *f);
void filaEnqueue(Fila *f, int idx);
int  filaDequeue(Fila *f);

/* executor.c */
void executar(int idx, int quantum);

/* scheduler.c */
int  escalonarFCFS(void);
int  escalonarPriority(void);
int  escalonarSJFS(void);

/* gestor.c */
void iniciarSimulador(const char *planFile);
void correrSimulador(const char *controlFile);

/* report.c */
void report(void);
void reportFinal(void);

#endif
