#ifndef SIM_H
#define SIM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ─── MEMÓRIA ─────────────────────────────────────────────── */
#define MEM_SIZE   1000
#define MAX_PROCS   50
#define NOME_MAX    15

typedef struct {
    char ins;
    int  n;
    char nome[NOME_MAX];
} Instruction;

extern Instruction memory[MEM_SIZE];

/* ─── PCB ─────────────────────────────────────────────────── */
typedef enum { FREE, READY, RUNNING, BLOCKED, TERMINATED } Estado;

typedef struct {
    int   pid;
    int   ppid;
    int   prioridade;
    int   pc;
    int   start;
    int   size;
    int   valor;
    int   tempo_inicio;
    int   tempo_cpu;
    int   periodo;      /* 0 = não periódico */
    int   deadline;     /* tempo_inicio + periodo */
    char  programa[NOME_MAX];
    Estado estado;
} PCB;

extern PCB    pcbTabela[MAX_PROCS];
extern int    tempo;
extern int    runningIdx;
extern int    nextPid;

/* ─── FILAS ───────────────────────────────────────────────── */
#define FILA_MAX 50

typedef struct {
    int dados[FILA_MAX];
    int head, tail, count;
} Fila;

extern Fila prontos;
extern Fila bloqueados;

/* ─── CHEGADA DINÂMICA ────────────────────────────────────── */
#define MAX_PENDENTES 50

typedef struct {
    char ficheiro[NOME_MAX];
    int  chegada;
    int  prioridade;
    int  periodo;
} ProcessoPendente;

extern ProcessoPendente pendentes[MAX_PENDENTES];
extern int numPendentes;

/* ─── PROTÓTIPOS ──────────────────────────────────────────── */

/* loader.c */
int  carregarPrograma(const char *ficheiro);
int  programaEmMemoria(const char *ficheiro);
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
int  escalonarRM(void);
int  escalonarEDF(void);

/* gestor.c */
void iniciarSimulador(const char *planFile);
void correrSimulador(const char *controlFile);
void escalonadorLongoPrazo(void);

/* report.c */
void report(void);
void reportFinal(void);

#endif
