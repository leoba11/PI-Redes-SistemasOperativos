#include <cstring>
#include <dirent.h>
#include <iostream>
#include <string>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
#include "contratista.cpp"

using namespace std;

void lector()
{
    const char *constRuta;
    DIR *directorio;
    int estado;
    int memoriaCompartida;
    int semaforo;
    key_t llaveMemoriaCompartida = 0x4949;
    key_t llaveSemaforo = 0x1111;
    pid_t pid;
    string ruta;
    struct sembuf operacionSemaforo;
    vector<string> imagenes;
    void *contador;

    // Creamos la memoria compartida

    memoriaCompartida = shmget(llaveMemoriaCompartida, sizeof(int), IPC_CREAT | 0666);

    if (memoriaCompartida < 0)
    {
        cout << "Error al crear la memoria compartida" << endl;
        exit(1);
    }

    // Creamos un semaforo

    semaforo = semget(llaveSemaforo, 1, IPC_CREAT | 0666);

    if (semaforo < 0)
    {
        cout << "Error al crear el semaforo" << endl;
        exit(1);
    }

    // Creamos un contador en memoria compartida y lo inicializamos en 0

    contador = shmat(memoriaCompartida, NULL, 0);
    *((int *)contador) = 0;
    shmdt(contador);

    union semun {
        int val;
        struct semid_ds *buf;
        unsigned short *array;
        struct seminfo *__buf;
    };

    // Inicializamos un semaforo disponible

    union semun bloqueo;
    bloqueo.val = 1;
    semctl(semaforo, 0, SETVAL, bloqueo);

    cout << "Digite la ruta: " << endl;
    getline(cin, ruta);

    constRuta = ruta.c_str();

    // Buscamos en el directorio ingresado por el usuario archivos con formato png, jpg, gif, bmp y tif

    if (directorio = opendir(constRuta))
    {
        while (dirent *entrada = readdir(directorio))
        {
            int len = strlen(entrada->d_name);

            if (len > 4 && memcmp(entrada->d_name + len - 4, ".png", 4) == 0)
            {
                char imagen[PATH_MAX];
                snprintf(imagen, sizeof(imagen), "%s/%s", constRuta, entrada->d_name);
                struct stat st;
                if (stat(imagen, &st) == 0 && S_ISREG(st.st_mode))
                {
                    imagenes.push_back(entrada->d_name);
                }
            }
            else if (len > 4 && memcmp(entrada->d_name + len - 4, ".jpg", 4) == 0)
            {
                char imagen[PATH_MAX];
                snprintf(imagen, sizeof(imagen), "%s/%s", constRuta, entrada->d_name);
                struct stat st;
                if (stat(imagen, &st) == 0 && S_ISREG(st.st_mode))
                {
                    imagenes.push_back(entrada->d_name);
                }
            }
            else if (len > 4 && memcmp(entrada->d_name + len - 4, ".gif", 4) == 0)
            {
                char imagen[PATH_MAX];
                snprintf(imagen, sizeof(imagen), "%s/%s", constRuta, entrada->d_name);
                struct stat st;
                if (stat(imagen, &st) == 0 && S_ISREG(st.st_mode))
                {
                    imagenes.push_back(entrada->d_name);
                }
            }
            else if (len > 4 && memcmp(entrada->d_name + len - 4, ".bmp", 4) == 0)
            {
                char imagen[PATH_MAX];
                snprintf(imagen, sizeof(imagen), "%s/%s", constRuta, entrada->d_name);
                struct stat st;
                if (stat(imagen, &st) == 0 && S_ISREG(st.st_mode))
                {
                    imagenes.push_back(entrada->d_name);
                }
            }
            else if (len > 4 && memcmp(entrada->d_name + len - 4, ".tif", 4) == 0)
            {
                char imagen[PATH_MAX];
                snprintf(imagen, sizeof(imagen), "%s/%s", constRuta, entrada->d_name);
                struct stat st;
                if (stat(imagen, &st) == 0 && S_ISREG(st.st_mode))
                {
                    imagenes.push_back(entrada->d_name);
                }
            }
        }

        closedir(directorio);
    }
    else if (ENOENT == errno)
    {
        cout << "La ruta no existe" << endl;
    }
    else
    {
        cout << "No es posible abrir la ruta" << endl;
    }

    if (ruta.back() != '/')
    {
        ruta.append("/");
    }

    // Conectamos el contador de la memoria compartida

    contador = shmat(memoriaCompartida, NULL, 0);

    while (0 != imagenes.size())
    {
        if (*((int *)contador) < 2)
        {
            // Bloqueamos el aceso a la memoria compartida

            operacionSemaforo.sem_num = 0;
            operacionSemaforo.sem_op = -1;
            operacionSemaforo.sem_flg = 0;
            semop(semaforo, &operacionSemaforo, 1);

            // Aumentamos el contador para no crear mas de 2 contratistas

            *((int *)contador) += 1;
            string nombre = imagenes.back();
            imagenes.pop_back();

            // Desbloqueamos el aceso a la memoria compartida

            operacionSemaforo.sem_num = 0;
            operacionSemaforo.sem_op = 1;
            operacionSemaforo.sem_flg = 0;
            semop(semaforo, &operacionSemaforo, 1);

            // Creamos un proceso contratista

            pid = fork();

            if (pid == 0)
            {
                contratista(nombre, ruta);
                exit(0);
            }
            else
            {
                continue;
            }
        }
    }

    // Desconectamos la memoria compartida

    shmdt(contador);

    estado = 0;

    // Esperamos por todos los contratistas antes de cerrar el programa

    while ((pid = wait(&estado)) > 0)
        ;
}

int main(void)
{
    lector();
    return 0;
}
