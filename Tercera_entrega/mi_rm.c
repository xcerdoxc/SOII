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
    //Comprobamos si se trata de un fichero
    
    if (argv[2][strlen(argv[2]) - 1] == '/')
    {
        fprintf(stderr, "Error: no es un fichero\n");
        exit(EXIT_FAILURE);
    }
    //Verificamos si no se quiere borrar el directorio raiz
    if (argv[2][0] == '/' && strlen(argv[2]) > 1)
    {
        //Eliminamos el directorio
        mi_unlink(argv[2]);
    }
    else
    {
        fprintf(stderr, "No se puede borrar el directorio raiz\n");
        return EXIT_FAILURE;
    }
    //Desmontamos el dispositivo
    bumount();
    return EXIT_SUCCESS;
}