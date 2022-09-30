#include "directorios.h"

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        fprintf(stderr, "Sintaxis: ./mi_rn <disco> </ruta/antiguo> <nuevo> \n");
        return EXIT_FAILURE;
    }
    if (bmount(argv[1]) < 0)
    {
        return EXIT_FAILURE;
    }
    //no dejamos renombrar la raiz
    if (strcmp(argv[2], "/") == 0)
    {
        printf("NO PUEDES RENOMBRAR EL DIRECTORIO RAIZ");
        return EXIT_FAILURE;
    }
    mi_rn(argv[2],argv[3]);

    bumount();
    return EXIT_SUCCESS;
}