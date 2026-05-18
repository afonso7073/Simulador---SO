#include "sim.h"

/* Imprime uma linha de separação */
static void separador(void)
{
    printf("─────────────────────────────────────────────────────\n");
}

/* Imprime os dados de um processo */
static void imprimirProcesso(int idx)
{
    PCB *p = &pcbTabela[idx];
    printf("  PID=%-3d PPID=%-3d PRIOR=%-2d VALOR=%-6d "
           "PROG=%-15s PC=%-4d CPU=%-4d INICIO=%-4d\n",
           p->pid, p->ppid, p->prioridade, p->valor,
           p->programa, p->pc, p->tempo_cpu, p->tempo_inicio);
}

/* Relatório do estado atual do sistema */
void report(void)
{
    separador();
    printf("TEMPO ACTUAL: %d\n", tempo);
    separador();

    /* processo em execução */
    printf("PROCESSO EM EXECUÇÃO:\n");
    if (runningIdx == -1)
        printf("  (nenhum)\n");
    else
        imprimirProcesso(runningIdx);

    /* processos prontos */
    separador();
    printf("PROCESSOS PRONTOS (%d):\n", prontos.count);
    if (filaVazia(&prontos))
    {
        printf("  (nenhum)\n");
    }
    else
    {
        /* percorre a fila sem a destruir */
        for (int i = 0; i < prontos.count; i++)
        {
            int idx = prontos.dados[(prontos.head + i) % FILA_MAX];
            imprimirProcesso(idx);
        }
    }

    /* processos bloqueados */
    separador();
    printf("PROCESSOS BLOQUEADOS (%d):\n", bloqueados.count);
    if (filaVazia(&bloqueados))
    {
        printf("  (nenhum)\n");
    }
    else
    {
        for (int i = 0; i < bloqueados.count; i++)
        {
            int idx = bloqueados.dados[(bloqueados.head + i) % FILA_MAX];
            imprimirProcesso(idx);
        }
    }

    /* processos terminados */
    separador();
    printf("PROCESSOS TERMINADOS:\n");
    int algum = 0;
    for (int i = 0; i < MAX_PROCS; i++)
    {
        if (pcbTabela[i].estado == TERMINATED)
        {
            imprimirProcesso(i);
            algum = 1;
        }
    }
    if (!algum)
        printf("  (nenhum)\n");

    separador();
    printf("\n");
}

/* Estatísticas globais no fim da simulação */
void reportFinal(void)
{
    separador();
    printf("ESTATÍSTICAS FINAIS  (tempo total = %d)\n", tempo);
    separador();

    int total       = 0;
    int terminados  = 0;
    int bloqueadosN = 0;
    int prontoN     = 0;
    int totalCpu    = 0;

    for (int i = 0; i < MAX_PROCS; i++)
    {
        if (pcbTabela[i].estado == FREE)
            continue;
        total++;
        totalCpu += pcbTabela[i].tempo_cpu;

        switch (pcbTabela[i].estado)
        {
            case TERMINATED: terminados++;  break;
            case BLOCKED:    bloqueadosN++; break;
            case READY:      prontoN++;     break;
            default: break;
        }
    }

    printf("  Processos criados:    %d\n", total);
    printf("  Terminados:           %d\n", terminados);
    printf("  Bloqueados:           %d\n", bloqueadosN);
    printf("  Prontos (por acabar): %d\n", prontoN);
    printf("  Total instruções CPU: %d\n", totalCpu);

    /* turnaround médio dos processos terminados */
    separador();
    printf("TURNAROUND POR PROCESSO:\n");
    for (int i = 0; i < MAX_PROCS; i++)
    {
        if (pcbTabela[i].estado == TERMINATED)
        {
            int turnaround = tempo - pcbTabela[i].tempo_inicio;
            printf("  PID=%-3d %-15s turnaround=%-4d cpu=%-4d\n",
                   pcbTabela[i].pid,
                   pcbTabela[i].programa,
                   turnaround,
                   pcbTabela[i].tempo_cpu);
        }
    }
    separador();
}
