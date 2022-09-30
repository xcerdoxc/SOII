#include "simulacion.h"

int acabados = 0;
int main(int argc, char **argv)
{
    //Asociar la señal SIGCHLD al enterrador
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
    char tmp[100];
    char simul[100] = "/simul_";

    sprintf(tmp, "%d%02d%02d%02d%02d%02d/", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
    strcat(simul, tmp);
    //Creamos el directorio
    mi_creat(simul, 6);
    signal(SIGCHLD, reaper);
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
            char buffer_dir[200];
            memset(buffer_dir, 0, 200);

            //Creamos el directorio del proceso añadiendo el PID al nombre
            sprintf(buffer_dir, "%sproceso_%d/", simul, pidChild);
            if (mi_creat(buffer_dir, 6) < 0)
            {
                printf("Error al crear el directorio %s\n", buffer_dir);
                bumount();
                exit(0);
            }
            //Creamos el fichero prueba.dat
            char buff_fic[300];
            memset(buff_fic, 0, 300);
            sprintf(buff_fic, "%sprueba.dat", buffer_dir);
            if (mi_creat(buff_fic, 6) < 0)
            {
                printf("Error al crear el fichero.dat %s\n", buff_fic);
                bumount();
                exit(0);
            }
            //Inicializar la semilla de números aleatorios
            srand(time(NULL) + pidChild);
            for (int nescritura = 1; nescritura <= NUMESCRITURAS; nescritura++)
            {
                //Inicializamos el struct REGISTRO
                registro.fecha = time(NULL);
                registro.pid = pidChild;
                registro.nEscritura = nescritura;
                registro.nRegistro = rand() % REGMAX;
                //Escribimos el registro
                mi_write(buff_fic, &registro, registro.nRegistro * sizeof(struct REGISTRO), sizeof(struct REGISTRO));
                //printf("[simulación.c → Escritura %d en %s]\n", nescritura, buff_fic);
                //Esperamos 0,05 segundos para la siguiente escritura
                usleep(50000);
            }
            //Desmontamos el hijo
            printf("[Proceso %d: Completadas %d escrituras en %s]\n", i, NUMESCRITURAS, buff_fic);
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
    exit(0);
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
