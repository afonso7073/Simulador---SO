#include "sim.h"

/* algoritmo de escalonamento ativo — muda conforme quiseres */
#define ALGORITMO 0   /* 0=FCFS  1=Priority  2=SJFS */
#define QUANTUM   3   /* unidades de tempo por fatia */

/* ─── FUNÇÕES AUXILIARES ──────────────────────────────── */

/* Inicializa toda a pcbTabela a FREE */
static void inicializarTabela(void)
{
    for (int i = 0; i < MAX_PROCS; i++)
        pcbTabela[i].estado = FREE;
}

/* Cria um processo novo a partir dum ficheiro .prg.
   Devolve o índice na pcbTabela, ou -1 em erro. */
static int criarProcesso(const char *ficheiro, int ppid, int prioridade)
{
    /* procura slot livre */
    int idx = -1;
    for (int i = 0; i < MAX_PROCS; i++)
    {
        if (pcbTabela[i].estado == FREE)
        {
            idx = i;
            break;
        }
    }
    if (idx == -1)
    {
        fprintf(stderr, "ERRO: tabela de processos cheia\n");
        return -1;
    }

    /* carrega o programa em memória */
    int start = programaEmMemoria(ficheiro);
    if (start == -1)
        start = carregarPrograma(ficheiro);
    if (start == -1)
        return -1;

    /* conta instruções */
    int size = 0;
    for (int i = start; i < MEM_SIZE && memory[i].ins != 0; i++)
        size++;

    /* preenche o PCB */
    PCB *p = &pcbTabela[idx];
    p->pid          = nextPid++;
    p->ppid         = ppid;
    p->prioridade   = prioridade;
    p->pc           = start;
    p->start        = start;
    p->size         = size;
    p->valor        = 0;
    p->tempo_inicio = tempo;
    p->tempo_cpu    = 0;
    p->estado       = READY;
    strncpy(p->programa, ficheiro, NOME_MAX - 1);

    filaEnqueue(&prontos, idx);

    printf("[t=%d] Processo %d criado ('%s', prioridade=%d)\n",
           tempo, p->pid, ficheiro, prioridade);

    return idx;
}

/* Chama o escalonador ativo e devolve o índice escolhido */
static int escalonar(void)
{
    switch (ALGORITMO)
    {
        case 1:  return escalonarPriority();
        case 2:  return escalonarSJFS();
        default: return escalonarFCFS();
    }
}

/* ─── ESCALONADOR DE LONGO PRAZO ──────────────────────── */
/* Percorre a fila de bloqueados e desbloqueia aleatoriamente
   um ou mais processos, movendo-os para prontos.            */
void escalonadorLongoPrazo(void)
{
    if (filaVazia(&bloqueados))
        return;

    int tmp[FILA_MAX];
    int n = bloqueados.count;
    for (int i = 0; i < n; i++)
        tmp[i] = filaDequeue(&bloqueados);

    for (int i = 0; i < n; i++)
    {
        /* 50% de probabilidade de desbloquear cada processo */
        if (rand() % 2 == 0)
        {
            pcbTabela[tmp[i]].estado = READY;
            filaEnqueue(&prontos, tmp[i]);
            printf("[t=%d] Processo %d desbloqueado\n",
                   tempo, pcbTabela[tmp[i]].pid);
        }
        else
        {
            filaEnqueue(&bloqueados, tmp[i]);
        }
    }
}

/* ─── INICIALIZAÇÃO ───────────────────────────────────── */
/* Lê o plan.txt e cria os processos iniciais.
   Formato de cada linha: ficheiro.prg tempo_chegada [prioridade] */
void iniciarSimulador(const char *planFile)
{
    inicializarTabela();
    filaInit(&prontos);
    filaInit(&bloqueados);
    memset(memory, 0, sizeof(memory));
    srand(42);

    FILE *f = fopen(planFile, "r");
    if (f == NULL)
    {
        fprintf(stderr, "ERRO: não foi possível abrir '%s'\n", planFile);
        return;
    }

    char ficheiro[NOME_MAX];
    int  chegada   = 0;
    int  prioridade = 1;
    char linha[64];

    while (fgets(linha, sizeof(linha), f))
    {
        /* ignora linhas vazias */
        if (linha[0] == '\n' || linha[0] == '\0')
            continue;

        prioridade = 1; /* valor por defeito */

        int lidos = sscanf(linha, "%14s %d %d", ficheiro, &chegada, &prioridade);
        if (lidos < 2)
            continue;

        criarProcesso(ficheiro, 0, prioridade);
    }
    fclose(f);
}
/* ─── LOOP PRINCIPAL ──────────────────────────────────── */
/* Lê comandos do control.txt (ou stdin) e executa-os.     */
void correrSimulador(const char *controlFile)
{
    FILE *f;
    if (controlFile != NULL)
        f = fopen(controlFile, "r");
    else
        f = stdin;

    if (f == NULL)
    {
        fprintf(stderr, "ERRO: não foi possível abrir '%s'\n", controlFile);
        return;
    }

    char cmd;
    while (fscanf(f, " %c", &cmd) == 1)
    {
        switch (cmd)
        {
            case 'E':
            {
                /* executa um processo durante QUANTUM unidades */
                int idx = escalonar();
                if (idx == -1)
                {
                    printf("[t=%d] Nenhum processo pronto\n", tempo);
                    break;
                }
                executar(idx, QUANTUM);
                break;
            }

            case 'I':
            {
                /* interrompe o processo em execução e bloqueia-o */
                if (runningIdx == -1)
                {
                    printf("[t=%d] Nenhum processo em execução\n", tempo);
                    break;
                }
                PCB *p = &pcbTabela[runningIdx];
                printf("[t=%d] Processo %d interrompido e bloqueado\n",
                       tempo, p->pid);
                p->estado = BLOCKED;
                filaEnqueue(&bloqueados, runningIdx);
                runningIdx = -1;
                break;
            }

            case 'D':
                /* escalonador de longo prazo — desbloqueia processos */
                escalonadorLongoPrazo();
                break;

            case 'R':
                /* relatório do estado atual */
                report();
                break;

            case 'T':
                /* terminação do simulador */
                printf("\n=== SIMULADOR TERMINADO ===\n");
                reportFinal();
                if (f != stdin)
                    fclose(f);
                return;

            default:
                fprintf(stderr, "Comando desconhecido: '%c'\n", cmd);
                break;
        }
    }

    /* se chegou ao fim do ficheiro sem T, imprime estatísticas na mesma */
    printf("\n=== FIM DO FICHEIRO DE CONTROLO ===\n");
    reportFinal();

    if (f != stdin)
        fclose(f);
}
