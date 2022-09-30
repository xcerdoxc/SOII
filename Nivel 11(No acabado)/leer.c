#include "ficheros.h"

int main(int argc, char **argv)
{
    //Declaramos las variable
    struct STAT stat;
    int ninodo, offset, leidos, tambuffer = 1500;
    char string[128];
    int contador = 0;
    char buffer_texto[tambuffer];
    //Montamos el fichero
    bmount(argv[1]);
    ninodo = atoi(argv[2]);
    offset = 0;

    memset(buffer_texto, 0, tambuffer);
    leidos = mi_read_f(ninodo, buffer_texto, offset, tambuffer);
    contador = leidos;
    while (leidos > 0)
    {
        //Mostramos los resultados por la pantalla
        write(1, buffer_texto, leidos);
        memset(buffer_texto, 0, tambuffer);
        offset = offset + tambuffer; //incrementamos el offset en funcion del tamaño del buffer
        leidos = mi_read_f(ninodo, buffer_texto, offset, tambuffer);
        contador = contador + leidos;
    }
    //Mostramos el número de bytes leídos y del tamaño en bytes lógicos del inodo
    sprintf(string, "total_leido %d\n", contador);
    write(2, string, strlen(string));
    //Lemos tamaño en bytes lógicos del STAT
    mi_stat_f(ninodo, &stat);
    sprintf(string, "tamEnBytesLog %d\n", stat.tamEnBytesLog);
    write(2, string, strlen(string));

    //Desmontamos el fichero
    bumount();
}