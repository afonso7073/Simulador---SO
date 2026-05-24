#include "sim.h"

int main(int argc, char *argv[])
{
    const char *planFile    = "plan.txt";
    const char *controlFile = "control.txt";

    if (argc >= 2)
        planFile = argv[1];
    if (argc >= 3)
        controlFile = argv[2];

    printf("=== SIMULADOR DE GESTÃO DE PROCESSOS ===\n");
    printf("  plan:    %s\n", planFile);
    printf("  control: %s\n\n", controlFile);

    iniciarSimulador(planFile);
    correrSimulador(controlFile);

    return 0;
}
