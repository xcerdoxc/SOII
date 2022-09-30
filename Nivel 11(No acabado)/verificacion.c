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
    struct STAT stat;
    struct entrada entrada;
    struct INFORMACION informacion;
    int cant_registros_buffer_escrituras = 256; //
    struct REGISTRO buffer_escrituras[cant_registros_buffer_escrituras];
    char dir_simul[100];
    char prueba[200];
    char buffer[1500];
    int cant_entradas;
    char *obtencion_pid;
    //Copiamos el directorio de simulacion que recibimos por parametro
    strcpy(dir_simul, argv[2]);
    printf("dir_simu: %s\n", dir_simul);
    //Calcular el nº de entradas del directorio de simulación a partir del stat de su inodo
    mi_stat(dir_simul, &stat);
    cant_entradas = stat.tamEnBytesLog / sizeof(struct entrada);
    printf("numentradas: %d\tNUMPROCESOS: %d\n", cant_entradas, NUMPROCESOS);
    //Verificamos si cant_entrads es igual a NUMPROCESO
    if (cant_entradas != NUMPROCESOS)
    {
        fprintf(stderr, "Error: el número de entradas no coincide con el Número de procesos\n");
        return EXIT_FAILURE;
    }
    //Creamos el fichero informe.txt
    strcat(dir_simul, "informe.txt");
    mi_creat(dir_simul, 6);
    //Leer los directorios correspondientes a los procesos
    mi_read(dir_simul, &entrada, 0, NUMPROCESOS * sizeof(struct entrada));
    //Iteramos para cada entrada del proceso
    for (int i = 1; i <= NUMPROCESOS; i++)
    {
        //Extraer el PID a partir del nombre de la entrada y guardarlo en el registro info
        obtencion_pid = strchr(entrada.nombre, '_');
        //Inicializamos a 0 el struct informacion
        memset(&informacion, 0, sizeof(struct INFORMACION));
        //Actualizamos el campo PID
        informacion.pid = atoi(obtencion_pid + 1);
        //Actualizamos el campo de numero de escrituras
        informacion.nEscrituras = 0;
        //Recorrer secuencialmente el fichero prueba.dat utilizando buffer de N registros de escrituras:

        int offset = 0;
        sprintf(prueba, "%s%s/prueba.dat", dir_simul, entrada.nombre);

        //Inicializamos el buffer de escritura
        memset(buffer_escrituras, 0, cant_registros_buffer_escrituras);
        while (mi_read(prueba, buffer_escrituras, offset, sizeof(buffer_escrituras)) > 0)
        {
            for (int j = 0; j < cant_registros_buffer_escrituras; j++)
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
                        if (buffer_escrituras[j].nEscritura < informacion.PrimeraEscritura.nEscritura) //Anterior
                        {
                            informacion.PrimeraEscritura = buffer_escrituras[j];
                        }
                        if (buffer_escrituras[j].nEscritura > informacion.UltimaEscritura.nEscritura) //POsterior
                        {
                            informacion.UltimaEscritura = buffer_escrituras[j];
                        }
                        if (buffer_escrituras[j].nRegistro < informacion.MenorPosicion.nRegistro)
                        {
                            informacion.MenorPosicion = buffer_escrituras[j];
                        }
                        if (buffer_escrituras[j].nRegistro > informacion.MayorPosicion.nRegistro)
                        {
                            informacion.MayorPosicion = buffer_escrituras[j];
                        }
                    }
                    //Incrementamos el contador de escrituras validas
                    informacion.nEscrituras++;
                }
            }
        }
        //Obtenemos escritura de la úñltima posición
        mi_stat(prueba, &stat);
        mi_read(prueba, &informacion.MayorPosicion, stat.tamEnBytesLog - sizeof(struct REGISTRO), sizeof(struct REGISTRO));

        //Añadir la información del struct info al fichero informe.txt por el final
        escritura_informacion(buffer, informacion);
        mi_write(dir_simul, buffer, stat.tamEnBytesLog, strlen(buffer));
        printf("%d)\t%d escrituras validadas en %s\n", i , informacion.nEscrituras, prueba);
    }
    //Desmontamos el dispositivo
    bumount();
}

void escritura_informacion(char *buffer, struct INFORMACION info){

       char buffer_secundario[250];
   sprintf(buffer, "PID:%d\n", info.pid);
   sprintf(buffer_secundario, "Numero de escrituras:%d\n",  info.nEscrituras);
   strcat(buffer, buffer_secundario);
   memset(buffer_secundario, 0, strlen(buffer_secundario));
   sprintf(buffer_secundario, "Primera Escritura\t%d\t%d\t%s", info.PrimeraEscritura.nEscritura, info.PrimeraEscritura.nRegistro,
           asctime(localtime(&info.PrimeraEscritura.fecha)));
   strcat(buffer, buffer_secundario);
   memset(buffer_secundario, 0, strlen(buffer_secundario));
   sprintf(buffer_secundario, "Ultima Escritura\t%d\t%d\t%s", info.UltimaEscritura.nEscritura, info.UltimaEscritura.nRegistro,
           asctime(localtime(&info.UltimaEscritura.fecha)));
   strcat(buffer, buffer_secundario);
   memset(buffer_secundario, 0, strlen(buffer_secundario));
   sprintf(buffer_secundario, "Menor Posición\t\t%d\t%d\t%s", info.MenorPosicion.nEscritura, info.MenorPosicion.nRegistro,
           asctime(localtime(&info.MenorPosicion.fecha)));
   strcat(buffer, buffer_secundario);
   memset(buffer_secundario, 0, strlen(buffer_secundario));
   sprintf(buffer_secundario, "Mayor Posición\t\t%d\t%d\t%s\n", info.MayorPosicion.nEscritura, info.MayorPosicion.nRegistro,
           asctime(localtime(&info.MayorPosicion.fecha)));
   strcat(buffer, buffer_secundario);
}