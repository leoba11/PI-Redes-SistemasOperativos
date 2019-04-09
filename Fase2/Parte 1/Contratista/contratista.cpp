#include <cstring>
#include <iostream>
#include <string>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>

using namespace std;

struct msgbuffer
{
    long mtype;
    unsigned char mtext[128];
};

void contratista(string nombre, string direccion)
{
    FILE *imagen;
    int cola_mtype;
    int cola_paquetes;
    int memoria_compartida;
    int semaforo;
    key_t llave_cola_mtype = 0x4444;
    key_t llave_cola_paquetes = 0x9999;
    key_t llave_memoria_compartida = 0x4949;
    key_t llave_semaforo = 0x1111;
    long mtype;
    struct msgbuffer mensaje;
    unsigned char buffer[512];
    unsigned char fin[17];
    unsigned char paquete[128];
    void *contador;

    cola_mtype = msgget(llave_cola_mtype, 0666);

    if (cola_mtype < 0)
    {
        cout << "Error al conectar con la cola de mensajes (tipos)" << endl;
        exit(1);
    }

    cola_paquetes = msgget(llave_cola_paquetes, 0666);

    if (cola_paquetes < 0)
    {
        cout << "Error al conectar con la cola de mensajes (paquetes)" << endl;
        exit(1);
    }

    memoria_compartida = shmget(llave_memoria_compartida, sizeof(int), 0666);

    if (memoria_compartida < 0)
    {
        cout << "Error al conectar con la memoria compartida" << endl;
        exit(1);
    }

    semaforo = semget(llave_semaforo, 1, 0666);

    if (semaforo < 0)
    {
        cout << "Error al conectar con el semaforo" << endl;
        exit(1);
    }

    direccion.append(nombre);
    imagen = fopen(direccion.c_str(), "rb");

    if (imagen == NULL)
    {
        cout << "Error al crear la imagen" << endl;
        exit(1);
    }

    while (true)
    {
        if (msgrcv(cola_mtype, &mensaje, sizeof(mensaje.mtext), 1, 0) > 0)
        {
            mtype = stoi(reinterpret_cast<char *>(mensaje.mtext));
            break;
        }
    }

    strcpy((char *)mensaje.mtext, nombre.c_str());
    mensaje.mtype = mtype;
    msgsnd(cola_paquetes, &mensaje, sizeof(mensaje.mtext), 0);

    while (fread(buffer, 512, 1, imagen) == 1)
    {
        for (int i = 0; i < 512; i += 128)
        {
            memcpy(paquete, buffer + i, 128);
            memcpy(mensaje.mtext, paquete, sizeof(paquete));
            mensaje.mtype = mtype;
            msgsnd(cola_paquetes, &mensaje, sizeof(mensaje.mtext), 0);
        }
    }

    strcpy((char *)mensaje.mtext, "Fin de la imagen");
    mensaje.mtype = mtype;
    msgsnd(cola_paquetes, &mensaje, sizeof(mensaje.mtext), 0);

    strcpy((char *)fin, "Fin de la imagen");

    contador = shmat(memoria_compartida, NULL, 0);

    struct sembuf operacionSemaforo;

    while (true)
    {
        if (msgrcv(cola_paquetes, &mensaje, sizeof(mensaje.mtext), mtype + 1, 0) > 0)
        {
            if (memcmp(fin, mensaje.mtext, 17) == 0)
            {

                contador = shmat(memoria_compartida, NULL, 0);

                operacionSemaforo.sem_num = 0;
                operacionSemaforo.sem_op = -1;
                operacionSemaforo.sem_flg = 0;
                semop(semaforo, &operacionSemaforo, 1);

                *((int *)contador) -= 1;

                operacionSemaforo.sem_num = 0;
                operacionSemaforo.sem_op = 1;
                operacionSemaforo.sem_flg = 0;
                semop(semaforo, &operacionSemaforo, 1);

                break;
            }
        }
    }

    shmdt(contador);

    fclose(imagen);
}
