#include "ficheros.h"

int main(int argc, char **argv)
{
    //falta validacion de sintaxis
    if (argc != 4)
    {
        fprintf(stderr, "Sintaxis: permitir <nombre_dispositivo> <ninodo> <permisos>\n");
        return EXIT_FAILURE;
    }
    else if (atoi(argv[3]) < 0 || atoi(argv[3]) > 8)
    {
        fprintf(stderr, "ERROR: modo inválido: <<%d>>\n", atoi(argv[2]));
        return EXIT_FAILURE;
    }
    //Declaración de las variables
    int ninodo, permisos;
    //montamos el dispositivo
    bmount(argv[1]);
    ninodo = atoi(argv[2]);
    permisos = atoi(argv[3]);
    mi_chmod_f(ninodo, permisos);
    //desmontamos el dispositivo
    bumount();
}