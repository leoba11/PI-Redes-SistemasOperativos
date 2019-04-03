#include <cstring>
#include <iostream>
#include <string>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

using namespace std;

struct msgbuffer
{
    long mtype;
    unsigned char mtext[128];
};

void emisor(string nombre, long mtype)
{
    FILE *imagen;
    int msqid;
    key_t key = 0x9999;
    struct msgbuffer mensaje;
    unsigned char fin[17];

    msqid = msgget(key, 0666);

    if (msqid < 0)
    {
        cout << "Error al conectar con la cola de mensajes" << endl;
        exit(1);
    }

    imagen = fopen(nombre.c_str(), "wb");

    if (imagen == NULL)
    {
        cout << "Error al crear la imagen" << endl;
        exit(1);
    }

    strcpy((char *)fin, "Fin de la imagen");

    while (true)
    {
        if (msgrcv(msqid, &mensaje, sizeof(mensaje.mtext), mtype, 0) > 0)
        {
            if (memcmp(fin, mensaje.mtext, 17) == 0)
            {
                break;
            }
            else
            {
                fwrite(mensaje.mtext, 1, 128, imagen);
            }
        }
    }

    strcpy((char *)mensaje.mtext, "Fin de la imagen");
    mensaje.mtype = mtype + 1;
    msgsnd(msqid, &mensaje, sizeof(mensaje.mtext), 0);

    fclose(imagen);
}

int main(void)
{
    int mtype = 2;
    string nombre;
    struct msgbuffer mensaje;
    int cola_mtype;
    int cola_paquetes;
    key_t llave_cola_mtype = 0x4444;
    key_t llave_cola_paquetes = 0x9999;

    cola_mtype = msgget(llave_cola_mtype, IPC_CREAT | 0666);

    if (cola_mtype < 0)
    {
        cout << "Error al crear la cola de mensajes" << endl;
        exit(1);
    }

    strcpy((char *)mensaje.mtext, to_string(mtype).c_str());
    mensaje.mtype = 1;
    msgsnd(cola_mtype, &mensaje, sizeof(mensaje.mtext), 0);

    cola_paquetes = msgget(llave_cola_paquetes, IPC_CREAT | 0666);

    if (cola_paquetes < 0)
    {
        cout << "Error al crear la cola de mensajes" << endl;
        exit(1);
    }

    while (true)
    {
        if (msgrcv(cola_paquetes, &mensaje, sizeof(mensaje.mtext), mtype, 0) > 0)
        {
            nombre = (char *)mensaje.mtext;

            pid_t pid = fork();

            if (pid == 0)
            {
                emisor(nombre, mtype);
            }
            else
            {
                mtype += 2;
                strcpy((char *)mensaje.mtext, to_string(mtype).c_str());
                mensaje.mtype = 1;
                msgsnd(cola_mtype, &mensaje, sizeof(mensaje.mtext), 0);
            }
        }
    }

    return 0;
}
