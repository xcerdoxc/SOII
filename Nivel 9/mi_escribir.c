#include "directorios.h"

int main(int argc, char **argv)
{
    if (argc != 5)
    {
        fprintf(stderr,"Sintaxis: ./mi_escribir <disco> </ruta_fichero> <texto> <offset>\n");
        return EXIT_FAILURE;
    }
    if(bmount(argv[1])<0){
        return EXIT_FAILURE;
    }
    char *buffer_texto=argv[3];
    int longitud=strlen(buffer_texto);

    if(argv[2][strlen(argv[2])-1]!='/'){

    char *camino=argv[2];
    unsigned int offset=atoi(argv[4]);
    int escritos=0;
    fprintf(stderr,"longitud texto %d\n", longitud);

    escritos= mi_write(camino,buffer_texto,offset,longitud);
    
    fprintf(stderr,"Bytes escritos %d\n",escritos);
    }
    bumount();
    return EXIT_SUCCESS;
}