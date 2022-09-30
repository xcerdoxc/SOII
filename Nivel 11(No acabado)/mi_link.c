#include "directorios.h"

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        fprintf(stderr, "Sintaxis: ./mi_link disco /ruta_fichero_original /ruta_enlace \n");
        return EXIT_FAILURE;
    }
    if (bmount(argv[1]) < 0)
    {
        return EXIT_FAILURE;
    }


    if (argv[2][strlen(argv[2]) - 1] != '/' && argv[3][strlen(argv[3]) - 1] != '/' )
    {


        mi_link(argv[2],argv[3]);

    }
    bumount();
    return EXIT_SUCCESS;
}