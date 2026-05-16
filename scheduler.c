#include "sim.h"

/* ─── FCFS ─────────────────────────────────────────────────────
   First Come First Served — retira simplesmente o primeiro
   processo da fila de prontos.
   Devolve o índice do PCB escolhido, ou -1 se fila vazia.      */
int escalonarFCFS(void)
{
    if (filaVazia(&prontos))
        return -1;
    return filaDequeue(&prontos);
}

/* ─── PRIORITY ─────────────────────────────────────────────────
   Escolhe o processo com maior prioridade (valor mais alto).
   Percorre toda a fila e reconstrói-a sem o escolhido.
   Devolve o índice do PCB escolhido, ou -1 se fila vazia.      */
int escalonarPriority(void)
{
    if (filaVazia(&prontos))
        return -1;

    /* copia a fila para um array temporário */
    int tmp[FILA_MAX];
    int n = prontos.count;
    for (int i = 0; i < n; i++)
        tmp[i] = filaDequeue(&prontos);

    /* encontra o índice com maior prioridade */
    int melhor = 0;
    for (int i = 1; i < n; i++)
    {
        if (pcbTabela[tmp[i]].prioridade > pcbTabela[tmp[melhor]].prioridade)
            melhor = i;
    }

    int escolhido = tmp[melhor];

    /* reinsere os restantes na fila pela ordem original */
    for (int i = 0; i < n; i++)
    {
        if (i != melhor)
            filaEnqueue(&prontos, tmp[i]);
    }

    return escolhido;
}

/* ─── SJFS ──────────────────────────────────────────────────────
   Shortest Job First (não preemptivo dentro do quantum).
   Como não há loops, o tempo restante é: start + size - pc
   Escolhe o processo com menos instruções restantes.
   Devolve o índice do PCB escolhido, ou -1 se fila vazia.      */
int escalonarSJFS(void)
{
    if (filaVazia(&prontos))
        return -1;

    /* copia a fila para array temporário */
    int tmp[FILA_MAX];
    int n = prontos.count;
    for (int i = 0; i < n; i++)
        tmp[i] = filaDequeue(&prontos);

    /* encontra o processo com menos instruções restantes */
    int melhor = 0;
    int menorResto = pcbTabela[tmp[0]].start
                   + pcbTabela[tmp[0]].size
                   - pcbTabela[tmp[0]].pc;

    for (int i = 1; i < n; i++)
    {
        int resto = pcbTabela[tmp[i]].start
                  + pcbTabela[tmp[i]].size
                  - pcbTabela[tmp[i]].pc;
        if (resto < menorResto)
        {
            menorResto = resto;
            melhor = i;
        }
    }

    int escolhido = tmp[melhor];

    /* reinsere os restantes */
    for (int i = 0; i < n; i++)
    {
        if (i != melhor)
            filaEnqueue(&prontos, tmp[i]);
    }

    return escolhido;
}
