#include "ficheros.h"

int main(int argc, char **argv)
{ //Comprobamos primero que todo si la sintaxis es correcta
    if (argc != 4)
    {
        fprintf(stderr, "Sintaxis: escribir <nombre_dispositivo> <'$(cat fichero)'> <diferentes_inodos>\n");
        fprintf(stderr,"Offsets: 9000,20900,30725000,409605000,480000000\n");
        fprintf(stderr,"Si diferentes_inodos = 0 se reserva un solo inodo para todos los offsets\n");
        return EXIT_FAILURE;
    }
    //Declaramos las variables
    struct STAT stat;
    int ninodo, tamTexto;
    int offset[] = {9000, 20900, 30725000, 409605000, 480000000}; //Definimos un array de offsets
    char *buffer = argv[2]; //Inicializamos el buffer con el texto que recibimos por parametro
    //Montamos el fichero
    bmount(argv[1]);
    tamTexto = strlen(buffer);
    printf("longitud del texto: %d\n\n", tamTexto);

    if (atoi(argv[3]) == 0)
    {
        //Se reserva un solo inodo para todos los offsets
        ninodo = reservar_inodo('f', 6);

        //Iteramos para cada offset
        for (int i = 0; i < 5; i++)
        {
            fprintf(stderr, "Nº inodo reservado: %d\n", ninodo);
            fprintf(stderr, "Offset: %d\n", offset[i]);
            int r = mi_write_f(ninodo, buffer, offset[i], tamTexto);
            fprintf(stderr,"Bytes escritos: %d\n", r);
            //Leemos el inodo
            mi_stat_f(ninodo, &stat);
            fprintf(stderr,"stat.tamEnBytesLog: %d\n", stat.tamEnBytesLog);
            fprintf(stderr,"stat.numBloquesOcupados: %d\n\n", stat.numBloquesOcupados);
        }
    }
    else if (atoi(argv[3]) == 1)
    {
        //Reservamos un inodo para cada offsets
        //Iteramos para cada offset
        for (int i = 0; i < 5; i++)
        {
            ninodo = reservar_inodo('f', 6);
            fprintf(stderr,"Nº inodo reservado: %d\n", ninodo);
            fprintf(stderr,"Offset: %d\n", offset[i]);
            int r = mi_write_f(ninodo, buffer, offset[i], tamTexto);
            fprintf(stderr,"Bytes escritos: %d\n", r);
            //Leemos el inodo
            mi_stat_f(ninodo, &stat);
            fprintf(stderr,"stat.tamEnBytesLog: %d\n", stat.tamEnBytesLog);
            fprintf(stderr,"stat.numBloquesOcupados: %d\n\n", stat.numBloquesOcupados);
        }
    }else{
        fprintf(stderr,"<diferentes_inodos> tiene un valor erroneo\n");
        return EXIT_FAILURE;
    }

    //Desmontamos el fichero
    bumount();
    return EXIT_SUCCESS;
}