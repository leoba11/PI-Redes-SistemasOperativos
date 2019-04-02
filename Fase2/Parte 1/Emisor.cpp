#include <cstring>
#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/msg.h>

using namespace std;

struct msgbuffer
{
    long mtype;
    unsigned char mtext[128];
};

int emisor(string nombre, long mtype)
{
    FILE *imagen;
    int msqid;
    key_t key = 99;
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
    int msqid;
    int mtype = 1;
    key_t key = 99;
    string nombre;
    struct msgbuffer mensaje;

    msqid = msgget(key, IPC_CREAT | 0666);

    if (msqid < 0)
    {
        cout << "Error al crear la cola de mensajes" << endl;
        exit(1);
    }

    while (true)
    {
        if (msgrcv(msqid, &mensaje, sizeof(mensaje.mtext), mtype, 0) > 0)
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
            }
        }
    }

    return 0;
}
