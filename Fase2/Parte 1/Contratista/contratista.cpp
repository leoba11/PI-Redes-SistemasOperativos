#include <cstring>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>

using namespace std;

struct msgbuffer
{
    long mtype;
    unsigned char mtext[128];
};

void contratista(string direccion, long mtype)
{
    FILE *imagen;
    int msqid;
    key_t key = 0x999;
    struct msgbuffer mensaje;
    unsigned char buffer[512];
    unsigned char paquete[128];
    unsigned char fin[17];

    imagen = fopen(direccion.c_str(), "rb");

    if (imagen == NULL)
    {
        cout << "Error al procesar la imagen" << endl;
        exit(1);
    }

    msqid = msgget(key, 0666);

    if (msqid < 0)
    {
        cout << "Error al conectar con la cola de mensajes" << endl;
        exit(1);
    }

    strcpy((char *)mensaje.mtext, direccion.c_str());
    mensaje.mtype = mtype;
    msgsnd(msqid, &mensaje, sizeof(mensaje.mtext), 0);

    while (fread(buffer, 512, 1, imagen) == 1)
    {
        for (int i = 0; i < 512; i += 128)
        {
            memcpy(paquete, buffer + i, 128);
            memcpy(mensaje.mtext, paquete, sizeof(paquete));
            mensaje.mtype = mtype;
            msgsnd(msqid, &mensaje, sizeof(mensaje.mtext), 0);
        }
    }

    strcpy((char *)mensaje.mtext, "Fin de la imagen");
    mensaje.mtype = mtype;
    msgsnd(msqid, &mensaje, sizeof(mensaje.mtext), 0);

    strcpy((char *)fin, "Fin de la imagen");

    while (true)
    {
        if (msgrcv(msqid, &mensaje, sizeof(mensaje.mtext), mtype + 1, 0) > 0)
        {
            if (memcmp(fin, mensaje.mtext, 17) == 0)
            {
                //Aquí disminuimos el contador de memoria compartida
                cout << "La copia se guardo correctamente" << endl;
                break;
            }
        }
    }

    fclose(imagen);
}

int main(void)
{
    contratista("nombre_de_la_imagen.png", 1);
    return 0;
}
