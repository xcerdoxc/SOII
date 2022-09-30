#include "directorios.h"

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Sintaxis: ./mi_stat <disco> </ruta>\n");
        return EXIT_FAILURE;
    }

    bmount(argv[1]);
    struct STAT stat;

    mi_stat(argv[2], &stat);
    printf("tipo: %c\n", stat.tipo);
    printf("permisos: %d\n", stat.permisos);
    struct tm *tm = localtime(&stat.atime);
    printf("atime: %d-%02d-%02d %02d:%02d:%02d\n", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
    tm = localtime(&stat.ctime);
    printf("ctime: %d-%02d-%02d %02d:%02d:%02d\n", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
    tm = localtime(&stat.mtime);
    printf("mtime: %d-%02d-%02d %02d:%02d:%02d\n", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
    printf("nlinks: %d\n", stat.nlinks);
    printf("tamEnBytesLog: %d\n", stat.tamEnBytesLog);
    printf("numBloquesOcupados: %d\n", stat.numBloquesOcupados);

    bumount();
    return EXIT_SUCCESS;
}
