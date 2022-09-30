#include "simulacion.h"

int acabados = 0;
int main(int argc, char **argv)
{
    //Asociar la señal SIGCHLD al enterrador
    signal(SIGCHLD, reaper);
    pid_t child;
    //sintaxis
    if (argc != 2)
    {
        fprintf(stderr, "Sintaxis: ./simulacion <disco>\n");
        return EXIT_FAILURE;
    }
    //montamos dispositivo
    if (bmount(argv[1]) < 0)
    {
        return EXIT_FAILURE;
    }
    struct REGISTRO registro;
    time_t tim = time(NULL);
    struct tm *tm = localtime(&tim);
    char tmp[100], buff[26];
    char simul[100] = "/simul_";

    sprintf(tmp, "%d%02d%02d%02d%02d%02d/", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
    strcat(simul, tmp);
    //Creamos el directorio
    mi_creat(simul, 6);
    printf("*** SIMULACIÓN DE %d PROCESOS REALIZANDO CADA UNO %d ESCRITURAS ***\n", NUMPROCESOS, NUMESCRITURAS);

    for (int i = 1; i <= NUMPROCESOS; i++)
    {
        child = fork();
        //Verificamos si se trata del hijo
        if (child == 0)
        {
            int pidChild = getpid();
            //Montamos el dispositivo hijo
            bmount(argv[1]);
            //Creamos el directorio del proceso añadiendo el PID al nombre
            sprintf(buff, "proceso_PID%d/", pidChild);
            strcat(simul, buff);
            mi_creat(simul, 6);
            //Creamos el fichero prueba.dat
            strcat(simul, "prueba.dat");
            mi_creat(simul, 6);
            //Inicializar la semilla de números aleatorios
            srand(time(NULL) + getpid());
            for (int nescritura = 1; nescritura <= NUMESCRITURAS; nescritura++)
            {
                //Inicializamos el struct REGISTRO
                registro.fecha = time(NULL);
                registro.pid = getpid();
                registro.nEscritura = nescritura;
                registro.nRegistro = rand() % REGMAX;
                //Escribimos el registro
                mi_write(simul, &registro, registro.nRegistro * sizeof(struct REGISTRO), sizeof(struct REGISTRO));
              //  printf("[simulación.c → Escritura %d en %s]\n", nescritura, simul);
                //Esperamos 0,05 segundos para la siguiente escritura
                usleep(50000);
            }
            //Desmontamos el hijo
            printf("[Proceso %d: Completadas %d escrituras en %s]\n", i, NUMESCRITURAS, simul);
            bumount();
            exit(0);
        }
        //Esperamos 0,2 seg para lanzar el siguiente proceso
        usleep(200000);
    }
    //Permitimos que el padre espere por todos los hijos
    while (acabados < NUMPROCESOS)
    {
        pause();
    }

    //Desmontamos el dispositivo padre
    bumount();
    return EXIT_SUCCESS;
}

void reaper()
{
    pid_t ended;
    signal(SIGCHLD, reaper);
    while ((ended = waitpid(-1, NULL, WNOHANG)) > 0)
    {
        acabados++;
    }
}
