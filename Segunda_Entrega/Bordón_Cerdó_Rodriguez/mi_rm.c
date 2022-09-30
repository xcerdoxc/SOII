#include "directorios.h"

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Sintaxis: ./mi_rm disco /ruta \n");
        return EXIT_FAILURE;
    }
    if (bmount(argv[1]) < 0)
    {
        return EXIT_FAILURE;
    }

    //stcmp(argv[2],"/")==0;
    if (argv[2][0] == '/' && strlen(argv[2]) > 1)
    {
        mi_unlink(argv[2]);
    }
    else
    {
        fprintf(stderr, "No se puede borrar el directorio raiz\n");
        return EXIT_FAILURE;
    }
    bumount();
    return EXIT_SUCCESS;
}