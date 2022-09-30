#include "verificacion.h"

int main(int argc, char **argv)
{

    //Comprovamos la sintaxis
    if (argc != 3)
    {
        fprintf(stderr, "Sintaxis: ./verificacion <nombre_dispositivo> <directorio_simulación>\n");
        return EXIT_FAILURE;
    }
    //montamos dispositivo
    if (bmount(argv[1]) < 0)
    {
        return EXIT_FAILURE;
    }
    //Declaración de las variables
    struct STAT stat;
    struct entrada entradas[NUMPROCESOS * sizeof (struct entrada)];
    struct INFORMACION informacion;
    int cant_registros_buffer_escrituras = 256; 
    struct REGISTRO buffer_escrituras[cant_registros_buffer_escrituras];
    char dir_simul[100];
    char prueba[200];
    char buffer[300];
    int cant_entradas;
    char *obtencion_pid;
    //Copiamos el directorio de simulacion que recibimos por parametro
    strcpy(dir_simul, argv[2]);
    //printf("dir_simu: %s\n", dir_simul);
    //Calcular el nº de entradas del directorio de simulación a partir del stat de su inodo
    mi_stat(dir_simul, &stat);
    cant_entradas = stat.tamEnBytesLog / sizeof(struct entrada);
    printf("numentradas: %d     NUMPROCESOS: %d\n", cant_entradas, NUMPROCESOS);
    //Verificamos si cant_entrads es igual a NUMPROCESO
    if (cant_entradas != NUMPROCESOS)
    {
        fprintf(stderr, "Error: el número de entradas no coincide con el Número de procesos\n");
        return EXIT_FAILURE;
    }
    //Creamos el fichero informe.txt
    char aux[200];
    sprintf(aux, "%sinforme.txt", dir_simul);
    mi_creat(aux, 6);
    //Leer los directorios correspondientes a los procesos
    memset(entradas, 0, sizeof(entradas));
    mi_read(dir_simul, entradas, 0, sizeof(entradas));
    
    //Iteramos para cada entrada del proceso
    for (int i = 0; i < NUMPROCESOS; i++)
    {
        //printf("%s\n", entradas[i].nombre);
        //Extraer el PID a partir del nombre de la entrada y guardarlo en el registro info
         obtencion_pid = strchr(entradas[i].nombre, '_');
        //Inicializamos a 0 el struct informacion
        memset(&informacion, 0, sizeof(struct INFORMACION));
        //Actualizamos el campo PID
        informacion.pid = atoi(obtencion_pid + 1);
        //Actualizamos el campo de numero de escrituras
        informacion.nEscrituras = 0;
        //Recorrer secuencialmente el fichero prueba.dat utilizando buffer de N registros de escrituras:
        int offset = 0;
        //Inicializamos el camino prueba
        memset(prueba, 0, sizeof(prueba));
        sprintf(prueba, "%s%s/prueba.dat", dir_simul, entradas[i].nombre);

        //Inicializamos el buffer de escritura
        memset(buffer_escrituras, 0, sizeof(buffer_escrituras));
        //mientras haya escrituras en el .dat
        while (mi_read(prueba, buffer_escrituras, offset, sizeof(buffer_escrituras)) > 0)
        {   
            //leemos una por una las escituras
            for (int j = 0; j < cant_registros_buffer_escrituras && informacion.nEscrituras < NUMESCRITURAS; j++)
            {
                //Verificamos si la escritura es valida
                if (buffer_escrituras[j].pid == informacion.pid)
                {
                    //Verificamos si es la primera escritura válida
                    if (informacion.nEscrituras == 0)
                    {

                        //Inicializar los registros significativos con los datos de esa escritura
                        informacion.PrimeraEscritura = buffer_escrituras[j];
                        informacion.UltimaEscritura = buffer_escrituras[j];
                        informacion.MenorPosicion = buffer_escrituras[j];
                        informacion.MayorPosicion = buffer_escrituras[j];
                    }
                    else
                    {
                        //Comparar nº de escritura (para obtener primera y última) y actualizarla si es preciso
                        if (difftime(buffer_escrituras[j].fecha, informacion.PrimeraEscritura.fecha) < 0) 
                        {
                            informacion.PrimeraEscritura = buffer_escrituras[j];
                        }
                        if (difftime(buffer_escrituras[j].fecha, informacion.UltimaEscritura.fecha) > 0) 
                        {
                            informacion.UltimaEscritura = buffer_escrituras[j];
                        }
                    }
                    //Incrementamos el contador de escrituras validas
                    informacion.nEscrituras++;
                }
            }
            offset += cant_registros_buffer_escrituras;
            memset(buffer_escrituras, 0, sizeof(buffer_escrituras));
         }
        //Obtenemos escritura de la última posición
        mi_stat(prueba, &stat);
        mi_read(prueba, &informacion.MayorPosicion, stat.tamEnBytesLog - sizeof(struct REGISTRO), sizeof(struct REGISTRO));

        //Añadir la información del struct info al fichero informe.txt por el final
        //info al final de informe.txt
        mi_stat(aux, &stat);
        char buff_aux[400];
        //formato imforme.txt
        sprintf(buffer, "PID:%d \n", informacion.pid);
        sprintf(buff_aux, "Numero de escrituras:\t%d \n", informacion.nEscrituras);
        strcat(buffer, buff_aux);

        memset(buff_aux, 0, strlen(buff_aux));
        sprintf(buff_aux, "Primera Escritura \t%d\t%d\t%s", informacion.PrimeraEscritura.nEscritura, 
            informacion.PrimeraEscritura.nRegistro, asctime(localtime(&informacion.PrimeraEscritura.fecha)));
        strcat(buffer, buff_aux);

        memset(buff_aux, 0, strlen(buff_aux));
        sprintf(buff_aux, "Ultima Escritura \t%d\t%d\t%s", informacion.UltimaEscritura.nEscritura, informacion.UltimaEscritura.nRegistro,
            asctime(localtime(&informacion.UltimaEscritura.fecha)));
        strcat(buffer, buff_aux);

        memset(buff_aux, 0, strlen(buff_aux));
        sprintf(buff_aux, "Menor Posición \t\t%d\t%d\t%s", informacion.MenorPosicion.nEscritura, informacion.MenorPosicion.nRegistro,
            asctime(localtime(&informacion.MenorPosicion.fecha)));
        strcat(buffer, buff_aux);

        memset(buff_aux, 0, strlen(buff_aux));
        sprintf(buff_aux, "Mayor Posición \t\t%d\t%d\t%s \n", informacion.MayorPosicion.nEscritura, informacion.MayorPosicion.nRegistro,
            asctime(localtime(&informacion.MayorPosicion.fecha)));
        strcat(buffer, buff_aux);

        mi_write(aux, buffer, stat.tamEnBytesLog, strlen(buffer));
        printf("%d) %d escrituras validadas en %s \n", i + 1, informacion.nEscrituras, prueba);

    }
    //Desmontamos el dispositivo
    bumount();
    return EXIT_SUCCESS;
}