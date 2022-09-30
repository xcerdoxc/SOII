#include "directorios.h"

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        fprintf(stderr,"Sintaxis: ./mi_chmod <nombre_dispositivo> <permisos> </ruta>\n");
        return EXIT_FAILURE;
    }
    if (atoi (argv[2])<0 || atoi (argv[2])>7){
        fprintf(stderr,"ERROR: modo inválido: <<%d>>\n", atoi(argv[2]));
        return EXIT_FAILURE;
    }
    bmount(argv[1]);

    mi_chmod(argv[3],atoi(argv[2]));

    bumount();
    return EXIT_SUCCESS;
}