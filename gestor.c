#include "sim.h"

#define ALGORITMO 0   /* 0=FCFS  1=Priority  2=SJFS  3=RM  4=EDF */
#define QUANTUM   3

static void inicializarTabela(void)
{
    for (int i = 0; i < MAX_PROCS; i++)
        pcbTabela[i].estado = FREE;
}

static int criarProcesso(const char *ficheiro, int ppid, int prioridade, int periodo)
{
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

    int start = programaEmMemoria(ficheiro);
    if (start == -1)
        start = carregarPrograma(ficheiro);
    if (start == -1)
        return -1;

    int size = 0;
    for (int i = start; i < MEM_SIZE && memory[i].ins != 0; i++)
        size++;

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
    p->periodo      = periodo;
    p->deadline     = (periodo > 0) ? tempo + periodo : 0;
    p->estado       = READY;
    strncpy(p->programa, ficheiro, NOME_MAX - 1);

    filaEnqueue(&prontos, idx);

    printf("[t=%d] Processo %d criado ('%s', prioridade=%d, periodo=%d)\n",
           tempo, p->pid, ficheiro, prioridade, periodo);

    return idx;
}

static int escalonar(void)
{
    switch (ALGORITMO)
    {
        case 1:  return escalonarPriority();
        case 2:  return escalonarSJFS();
        case 3:  return escalonarRM();
        case 4:  return escalonarEDF();
        default: return escalonarFCFS();
    }
}

static void verificarChegadas(void)
{
    for (int i = 0; i < numPendentes; i++)
    {
        if (pendentes[i].chegada <= tempo && pendentes[i].ficheiro[0] != '\0')
        {
            criarProcesso(pendentes[i].ficheiro, 0,
                          pendentes[i].prioridade,
                          pendentes[i].periodo);
            pendentes[i].ficheiro[0] = '\0';
        }
    }
}

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

void iniciarSimulador(const char *planFile)
{
    inicializarTabela();
    filaInit(&prontos);
    filaInit(&bloqueados);
    memset(memory, 0, sizeof(memory));
    srand(42);
    numPendentes = 0;

    FILE *f = fopen(planFile, "r");
    if (f == NULL)
    {
        fprintf(stderr, "ERRO: não foi possível abrir '%s'\n", planFile);
        return;
    }

    char ficheiro[NOME_MAX];
    int  chegada    = 0;
    int  prioridade = 1;
    int  periodo    = 0;
    char linha[64];

    while (fgets(linha, sizeof(linha), f))
    {
        if (linha[0] == '\n' || linha[0] == '\0')
            continue;

        prioridade = 1;
        periodo    = 0;

        int lidos = sscanf(linha, "%14s %d %d %d",
                           ficheiro, &chegada, &prioridade, &periodo);
        if (lidos < 2)
            continue;

        if (chegada == 0)
        {
            criarProcesso(ficheiro, 0, prioridade, periodo);
        }
        else
        {
            strncpy(pendentes[numPendentes].ficheiro, ficheiro, NOME_MAX - 1);
            pendentes[numPendentes].chegada    = chegada;
            pendentes[numPendentes].prioridade = prioridade;
            pendentes[numPendentes].periodo    = periodo;
            numPendentes++;
        }
    }
    fclose(f);
}

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
                verificarChegadas();
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
                escalonadorLongoPrazo();
                break;

            case 'R':
                report();
                break;

            case 'T':
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

    printf("\n=== FIM DO FICHEIRO DE CONTROLO ===\n");
    reportFinal();

    if (f != stdin)
        fclose(f);
}
