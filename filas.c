#include "sim.h"

/* Inicializar uma fila */
void filaInit(Fila *f)
{
    f->head  = 0;
    f->tail  = 0;
    f->count = 0;
}

/* Verificar se a fila está vazia */
int filaVazia(Fila *f)
{
    return f->count == 0;
}

/* Inserir no fim da fila */
void filaEnqueue(Fila *f, int idx)
{
    if (f->count >= FILA_MAX)
    {
        fprintf(stderr, "ERRO: fila cheia\n");
        return;
    }
    f->dados[f->tail] = idx;
    f->tail = (f->tail + 1) % FILA_MAX;
    f->count++;
}

/* Retirar do início da fila — devolve -1 se vazia */
int filaDequeue(Fila *f)
{
    if (filaVazia(f))
        return -1;
    int idx = f->dados[f->head];
    f->head = (f->head + 1) % FILA_MAX;
    f->count--;
    return idx;
}
