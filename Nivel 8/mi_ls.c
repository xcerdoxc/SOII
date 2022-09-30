#include "directorios.h"

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Sintaxis: ./mi_ls <disco> </ruta_directorio>\n");
        return EXIT_FAILURE;
    }
    bmount(argv[1]);
    char buffer[TAMBUFFER];
    //Inicializacmos el buffer a 0
    memset(buffer, 0, TAMBUFFER);
    int total = mi_dir(argv[2], buffer);
    printf("Total: %d \n", total);
    printf("Tipo\tModo\t\tmTime\t\t\tTamaño\t\tNombre\n");
    printf("--------------------------------------------------------------------------------------------\n");

    printf("%s\n", buffer);

    bumount();
}