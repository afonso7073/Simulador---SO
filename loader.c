#include "sim.h"

static int encontrarSlot(int size)
{
    int i = 0;
    while (i < MEM_SIZE)
    {
        if (memory[i].ins == 0)
        {
            int livre = 1;
            for (int j = i; j < i + size && j < MEM_SIZE; j++)
            {
                if (memory[j].ins != 0)
                {
                    livre = 0;
                    break;
                }
            }
            if (livre)
                return i;
        }
        i++;
    }
    return -1;
}

int programaEmMemoria(const char *ficheiro)
{
    for (int i = 0; i < MAX_PROCS; i++)
    {
        if (pcbTabela[i].estado != FREE &&
            strcmp(pcbTabela[i].programa, ficheiro) == 0)
            return pcbTabela[i].start;
    }
    return -1;
}

void libertarMemoria(int start, int size)
{
    for (int i = start; i < start + size; i++)
    {
        memory[i].ins     = 0;
        memory[i].n       = 0;
        memory[i].nome[0] = '\0';
    }
}

int carregarPrograma(const char *ficheiro)
{
    /* primeira passagem — conta instruções */
    FILE *f = fopen(ficheiro, "r");
    if (f == NULL)
    {
        fprintf(stderr, "ERRO: não foi possível abrir '%s'\n", ficheiro);
        return -1;
    }

    int  size = 0;
    char linha[32];
    while (fgets(linha, sizeof(linha), f))
    {
        if (linha[0] != '\n' && linha[0] != '\0')
            size++;
    }
    fclose(f);

    if (size == 0)
    {
        fprintf(stderr, "ERRO: ficheiro '%s' vazio\n", ficheiro);
        return -1;
    }

    int start = encontrarSlot(size);
    if (start == -1)
    {
        fprintf(stderr, "ERRO: memória cheia para '%s'\n", ficheiro);
        return -1;
    }

    /* segunda passagem — carrega as instruções */
    f = fopen(ficheiro, "r");
    int pos = start;
    while (fgets(linha, sizeof(linha), f))
    {
        if (linha[0] == '\n' || linha[0] == '\0')
            continue;

        char ins = '\0';
        int  n   = 0;
        char nome[NOME_MAX];
        nome[0] = '\0';

        /* remove newline no fim */
        linha[strcspn(linha, "\n")] = '\0';

        sscanf(linha, " %c", &ins);

        if (ins == 'L')
        {
            sscanf(linha, " %c %14s", &ins, nome);
        }
        else if (ins == 'B' || ins == 'T')
        {
            n = 0;
        }
        else
        {
            sscanf(linha, " %c %d", &ins, &n);
        }

        memory[pos].ins = ins;
        memory[pos].n   = n;
        strncpy(memory[pos].nome, nome, NOME_MAX - 1);
        memory[pos].nome[NOME_MAX - 1] = '\0';
        pos++;
    }
    fclose(f);

    return start;
}
