#include "sim.h"

/* Cria um novo processo filho a partir do processo pai (idx).
   O filho é uma cópia do pai mas com PC logo após a instrução C.
   O pai salta 'n' instruções. */
static void criarFilho(int paiIdx, int n)
{
    /* procura slot livre na pcbTabela */
    int filhoIdx = -1;
    for (int i = 0; i < MAX_PROCS; i++)
    {
        if (pcbTabela[i].estado == FREE)
        {
            filhoIdx = i;
            break;
        }
    }
    if (filhoIdx == -1)
    {
        fprintf(stderr, "ERRO: tabela de processos cheia\n");
        return;
    }

    PCB *pai   = &pcbTabela[paiIdx];
    PCB *filho = &pcbTabela[filhoIdx];

    /* copia o contexto do pai */
    *filho = *pai;

    /* atribui novo PID */
    filho->pid   = nextPid++;
    filho->ppid  = pai->pid;
    filho->estado = READY;

    /* filho executa a instrução a seguir ao C */
    filho->pc = pai->pc;  /* pai->pc já avançou para depois do C */

    /* pai salta n instruções */
    pai->pc += n;

    /* insere filho na fila de prontos */
    filaEnqueue(&prontos, filhoIdx);

    printf("[t=%d] Processo %d criado (filho de %d)\n",
           tempo, filho->pid, pai->pid);
}

/* Carrega novo programa no processo idx (instrução L) */
static void carregarNovoPrograma(int idx, const char *ficheiro)
{
    PCB *p = &pcbTabela[idx];

    /* liberta o programa atual da memória se mais nenhum processo o usa */
    int emUso = 0;
    for (int i = 0; i < MAX_PROCS; i++)
    {
        if (i != idx &&
            pcbTabela[i].estado != FREE &&
            pcbTabela[i].start  == p->start)
        {
            emUso = 1;
            break;
        }
    }
    if (!emUso)
        libertarMemoria(p->start, p->size);

    /* verifica se o novo programa já está em memória */
    int start = programaEmMemoria(ficheiro);
    if (start == -1)
        start = carregarPrograma(ficheiro);

    if (start == -1)
    {
        fprintf(stderr, "ERRO: não foi possível carregar '%s'\n", ficheiro);
        return;
    }

    /* conta instruções do novo programa */
    int size = 0;
    for (int i = start; i < MEM_SIZE && memory[i].ins != 0; i++)
        size++;

    /* atualiza PCB */
    strncpy(p->programa, ficheiro, NOME_MAX - 1);
    p->start = start;
    p->size  = size;
    p->pc    = start;   /* recomeça do início */
    p->valor = 0;

    printf("[t=%d] Processo %d carregou '%s'\n", tempo, p->pid, ficheiro);
}

/* Executa o processo idx durante no máximo 'quantum' unidades de tempo.
   Para antes se o processo terminar ou bloquear. */
void executar(int idx, int quantum)
{
    PCB *p = &pcbTabela[idx];
    p->estado = RUNNING;
    runningIdx = idx;

    for (int q = 0; q < quantum; q++)
    {
        /* verifica se o PC está dentro dos limites do programa */
        if (p->pc >= p->start + p->size || memory[p->pc].ins == 0)
        {
            printf("[t=%d] Processo %d chegou ao fim do programa sem T\n",
                   tempo, p->pid);
            p->estado = TERMINATED;
            runningIdx = -1;
            return;
        }

        Instruction instr = memory[p->pc];
        p->pc++;        /* avança o PC antes de executar */
        tempo++;        /* uma instrução = uma unidade de tempo */
        p->tempo_cpu++;

        switch (instr.ins)
        {
            case 'M':
                p->valor = instr.n;
                break;

            case 'A':
                p->valor += instr.n;
                break;

            case 'S':
                p->valor -= instr.n;
                break;

            case 'B':
                printf("[t=%d] Processo %d bloqueado\n", tempo, p->pid);
                p->estado = BLOCKED;
                filaEnqueue(&bloqueados, idx);
                runningIdx = -1;
                return;

            case 'T':
                printf("[t=%d] Processo %d terminou (valor=%d)\n",
                       tempo, p->pid, p->valor);
                p->estado = TERMINATED;
                runningIdx = -1;
                return;

            case 'C':
                criarFilho(idx, instr.n);
                /* após C o pai continua no mesmo quantum */
                break;

            case 'L':
                carregarNovoPrograma(idx, instr.nome);
                /* após L o processo continua no mesmo quantum
                   mas o PC já foi resetado para o novo programa */
                break;

            default:
                fprintf(stderr, "ERRO: instrução desconhecida '%c'\n",
                        instr.ins);
                break;
        }

        /* se o processo mudou de estado durante a execução, para */
        if (p->estado != RUNNING)
            return;
    }

    /* quantum esgotado — volta para a fila de prontos */
    printf("[t=%d] Processo %d esgotou quantum (valor=%d)\n",
           tempo, p->pid, p->valor);
    p->estado = READY;
    filaEnqueue(&prontos, idx);
    runningIdx = -1;
}
