#include "ficheros.h"

int main(int argc, char **argv){
    if (argc != 4)
    {
        fprintf(stderr,"Sintaxis: truncar <nombre_dispositivo> <ninodo> <nbytes>\n");
        return EXIT_FAILURE;
    }
    //Montamos el dispositivo
    bmount(argv[1]);
    int ninodo = atoi(argv[2]);
    int nbytes = atoi(argv[3]);
    if(nbytes==0){
        liberar_inodo(ninodo);
    }
    else{
        mi_truncar_f(ninodo,nbytes);
    }
    
    struct STAT stat; //comprobación STAT
    mi_stat_f(ninodo,&stat);
    printf("\nDATOS INODO %d: \n", ninodo);
    printf( "tipo =  %c\n", stat.tipo);
    printf("permisos = %d\n", stat.permisos);
    struct tm *tm = localtime(&stat.atime); //traducción de segundos a fecha y tiempo
    printf("atime: %d-%02d-%02d %02d:%02d:%02d\n", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
    tm = localtime(&stat.atime);
    printf("ctime: %d-%02d-%02d %02d:%02d:%02d\n", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
    tm = localtime(&stat.mtime);
    printf("mtime: %d-%02d-%02d %02d:%02d:%02d\n", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
    printf("nlinks = %d\n", stat.nlinks);
    printf("tamEnBytesLog = %d\n", stat.tamEnBytesLog);
    printf("numBloquesOcupados = %d\n", stat.numBloquesOcupados);
    
    //desmontamos el dispositivo
    bumount();
}