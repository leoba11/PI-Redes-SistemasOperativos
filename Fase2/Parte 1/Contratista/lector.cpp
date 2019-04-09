#include <cstring>
#include <dirent.h>
#include <iostream>
#include <string>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include "contratista.cpp"

using namespace std;

void lector()
{
    const char *const_ruta;
    DIR *directorio;
    int memoria_compartida;
    int semaforo;
    key_t llave_memoria_compartida = 0x4949;
    key_t llave_semaforo = 0x1111;
    string ruta;
    vector<string> imagenes;
    void *contador;

    memoria_compartida = shmget(llave_memoria_compartida, sizeof(int), IPC_CREAT | 0666);

    if (memoria_compartida < 0)
    {
        cout << "Error al crear la memoria compartida" << endl;
        exit(1);
    }

    semaforo = semget(llave_semaforo, 1, IPC_CREAT | 0666);

    if (semaforo < 0)
    {
        cout << "Error al crear el semaforo" << endl;
        exit(1);
    }

    contador = shmat(memoria_compartida, NULL, 0);
    *((int *)contador) = 0;
    shmdt(contador);

    union semun {
        int val;
        struct semid_ds *buf;
        unsigned short *array;
        struct seminfo *__buf;
    };

    union semun bloqueo;
    bloqueo.val = 1;
    semctl(semaforo, 0, SETVAL, bloqueo);

    cout << "Digite la ruta: " << endl;
    getline(cin, ruta);

    const_ruta = ruta.c_str();

    if (directorio = opendir(const_ruta))
    {
        while (dirent *entrada = readdir(directorio))
        {
            int len = strlen(entrada->d_name);

            if (len > 4 && memcmp(entrada->d_name + len - 4, ".png", 4) == 0)
            {
                char imagen[PATH_MAX];
                snprintf(imagen, sizeof(imagen), "%s/%s", const_ruta, entrada->d_name);
                struct stat st;
                if (stat(imagen, &st) == 0 && S_ISREG(st.st_mode))
                {
                    imagenes.push_back(entrada->d_name);
                }
            }
            else if (len > 4 && memcmp(entrada->d_name + len - 4, ".jpg", 4) == 0)
            {
                char imagen[PATH_MAX];
                snprintf(imagen, sizeof(imagen), "%s/%s", const_ruta, entrada->d_name);
                struct stat st;
                if (stat(imagen, &st) == 0 && S_ISREG(st.st_mode))
                {
                    imagenes.push_back(entrada->d_name);
                }
            }
            else if (len > 4 && memcmp(entrada->d_name + len - 4, ".gif", 4) == 0)
            {
                char imagen[PATH_MAX];
                snprintf(imagen, sizeof(imagen), "%s/%s", const_ruta, entrada->d_name);
                struct stat st;
                if (stat(imagen, &st) == 0 && S_ISREG(st.st_mode))
                {
                    imagenes.push_back(entrada->d_name);
                }
            }
            else if (len > 4 && memcmp(entrada->d_name + len - 4, ".bmp", 4) == 0)
            {
                char imagen[PATH_MAX];
                snprintf(imagen, sizeof(imagen), "%s/%s", const_ruta, entrada->d_name);
                struct stat st;
                if (stat(imagen, &st) == 0 && S_ISREG(st.st_mode))
                {
                    imagenes.push_back(entrada->d_name);
                }
            }
            else if (len > 4 && memcmp(entrada->d_name + len - 4, ".tif", 4) == 0)
            {
                char imagen[PATH_MAX];
                snprintf(imagen, sizeof(imagen), "%s/%s", const_ruta, entrada->d_name);
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

    contador = shmat(memoria_compartida, NULL, 0);

    struct sembuf operacionSemaforo;

    while (0 != imagenes.size())
    {
        if (*((int *)contador) < 2)
        {
            operacionSemaforo.sem_num = 0;
            operacionSemaforo.sem_op = -1;
            operacionSemaforo.sem_flg = 0;
            semop(semaforo, &operacionSemaforo, 1);

            *((int *)contador) += 1;
            string nombre = imagenes.back();
            imagenes.pop_back();

            operacionSemaforo.sem_num = 0;
            operacionSemaforo.sem_op = 1;
            operacionSemaforo.sem_flg = 0;
            semop(semaforo, &operacionSemaforo, 1);

            if (fork())
            {
                continue;
            }
            else
            {
                contratista(nombre, ruta);
                exit(0);
            }
        }
    }

    shmdt(contador);
}

int main(void)
{
    lector();
    return 0;
}
