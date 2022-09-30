#include "directorios.h"

int main(int argc, char **argv)
{
    //Verificamos la sintaxis 
    if (argc != 4)
    {
        fprintf(stderr,"Sintaxis: ./mi_mkdir <nombre_dispositivo> <permisos> </ruta_directorio/>\n");
        return EXIT_FAILURE;
    }
    //Verificamos si los datos del permiso son correctos
    if (atoi (argv[2])<0 || atoi (argv[2])>7){
        fprintf(stderr,"ERROR: modo inválido: <<%d>>\n", atoi(argv[2]));
        return EXIT_FAILURE;
    }
    bmount(argv[1]);

    //Verificamos si es un directorio o no 
    if (argv[3][strlen(argv[3]) - 1] == '/'){
        mi_creat(argv[3],atoi(argv[2]));
    }else{
        mostrar_error_buscar_entrada(ERROR_CAMINO_INCORRECTO);
        return EXIT_FAILURE;
    }

    bumount();
    
}