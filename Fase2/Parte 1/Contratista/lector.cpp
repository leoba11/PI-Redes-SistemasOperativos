#include <cstring>
#include <dirent.h>
#include <iostream>
#include <string>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>
#include <unistd.h>
#include "contratista.cpp"

using namespace std;

void lector()
{
    const char *const_ruta;
    DIR *dir;
    int shmid;
    key_t key = 0x4949;
    string ruta;
    vector<string> imagenes;

    shmid = shmget(key, sizeof(int), IPC_CREAT | 0666);

    if (shmid < 0)
    {
        cout << "Error al crear la memoria compartida" << endl;
        exit(1);
    }

    void *contador = shmat(shmid, NULL, 0);
    *((int *)contador) = 0;
    shmdt(contador);

    cout << "Digite la ruta: " << endl;
    getline(cin, ruta);

    const_ruta = ruta.c_str();

    if (dir = opendir(const_ruta))
    {
        while (dirent *entrada = readdir(dir))
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
        closedir(dir);
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

    while (0 != imagenes.size())
    {
        contador = shmat(shmid, NULL, 0);
        if (*((int *)contador) < 2)
        {

            *((int *)contador) += 1;

            string temp = imagenes.back();
            imagenes.pop_back();

            contratista(temp, ruta);
        }

        shmdt(contador);
    }
}

int main(void)
{
    lector();
    return 0;
}
