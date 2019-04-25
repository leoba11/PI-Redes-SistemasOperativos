#include <cstring>
#include <iostream>
#include <string>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <thread>
#include <vector>

using namespace std;

// Estructura para enviar mensajes a una cola

#pragma pack(1)
struct msgbuffer
{
    long mtype;
    unsigned char mtext[128];
};
#pragma pack(0)

// Estructura para guardar la lectura de 512 bytes

struct bigbuffer
{
    int bandera = 0;
    unsigned char bytes[512];
};

// Estructura para guardar la division de 512 bytes a de 128 bytes

struct smallbuffer
{
    int bandera = 0;
    unsigned char bytes[512];
};

void leer_bytes(vector<bigbuffer> &vector512, string direccion)
{
    FILE *imagen;
    int index = 0;
    struct bigbuffer bigBuffer;
    unsigned char buffer[512];
    size_t lectura = 1;

    imagen = fopen(direccion.c_str(), "rb");

    if (imagen == NULL)
    {
        cout << "Error al leer la imagen" << endl;
        exit(1);
    }

    // Leemos todo el archivo en paquetes de 512 bytes y lo guardamos

    while (lectura != 0)
    {
        lectura = fread(buffer, 512, 1, imagen);

        while (true)
        {
            if (vector512.at(index).bandera == 0)
            {
                memcpy(bigBuffer.bytes, buffer, sizeof(buffer));
                bigBuffer.bandera = 1;
                vector512.at(index) = bigBuffer;

                index = (index + 1) % 2;

                break;
            }
            else
            {
                continue;
            }
        }
    }

    // Guardamos un paquete indicando que se terminó de leer el archivo

    while (true)
    {
        if (vector512.at(index).bandera == 0)
        {
            memcpy(bigBuffer.bytes, "Fin", sizeof("Fin"));
            bigBuffer.bandera = 2;
            vector512.at(index) = bigBuffer;

            break;
        }
        else
        {
            continue;
        }
    }

    fclose(imagen);
}

void dividir_bytes(vector<bigbuffer> &vector512, vector<smallbuffer> &vector128)
{
    int bigIndex = 0;
    int smallIndex = 0;
    struct smallbuffer smallBuffer;

    // Dividimos cada paquete de 512 bytes en 4 paquetes de 128 bytes

    while (vector512.at(bigIndex).bandera != 2)
    {
        if (vector512.at(bigIndex).bandera == 1)
        {
            for (int i = 0; i < 512; i += 128)
            {
                while (true)
                {
                    if (vector128.at(smallIndex).bandera == 0)
                    {
                        memcpy(smallBuffer.bytes, vector512.at(bigIndex).bytes + i, 128);
                        smallBuffer.bandera = 1;
                        vector128.at(smallIndex) = smallBuffer;

                        smallIndex = (smallIndex + 1) % 6;

                        break;
                    }
                    else
                    {
                        continue;
                    }
                }
            }

            vector512.at(bigIndex).bandera = 0;

            bigIndex = (bigIndex + 1) % 2;
        }
        else
        {
            continue;
        }
    }

    // Enviamos un paquete indicando que se terminó de dividir todos los paquetes de 512

    while (true)
    {
        if (vector128.at(smallIndex).bandera == 0)
        {
            memcpy(smallBuffer.bytes, "Fin", sizeof("Fin"));
            smallBuffer.bandera = 2;
            vector128.at(smallIndex) = smallBuffer;

            break;
        }
        else
        {
            continue;
        }
    }
}

void enviar_bytes(vector<smallbuffer> &vector128, long mtype, int colaPaquetes)
{
    int smallIndex = 0;
    struct msgbuffer mensaje;

    // Enviamos todos los paquetes del archivo a una cola de paquetes

    while (vector128.at(smallIndex).bandera != 2)
    {
        while (true)
        {
            if (vector128.at(smallIndex).bandera == 1)
            {
                memcpy(mensaje.mtext, vector128.at(smallIndex).bytes, 128);
                mensaje.mtype = mtype;
                msgsnd(colaPaquetes, &mensaje, sizeof(mensaje.mtext), 0);

                vector128.at(smallIndex).bandera = 0;

                smallIndex = (smallIndex + 1) % 6;

                break;
            }
            else
            {
                continue;
            }
        }
    }
}

void contratista(string nombre, string direccion)
{
    int colaCompletados;
    int colaTipos;
    int colaPaquetes;
    int memoriaCompartida;
    int semaforo;
    key_t llaveColaCompletados = 0x9191;
    key_t llaveColaTipos = 0x4444;
    key_t llaveColaPaquetes = 0x9999;
    key_t llaveMemoriaCompartida = 0x4949;
    key_t llaveSemaforo = 0x1111;
    long mtype;
    struct msgbuffer mensaje;
    struct sembuf operacionSemaforo;
    unsigned char fin[17];
    vector<bigbuffer> vector512;
    vector<smallbuffer> vector128;
    void *contador;

    // Conectamos con la cola de mensajes de imagenes completadas

    colaCompletados = msgget(llaveColaCompletados, 0666);

    if (colaCompletados < 0)
    {
        cout << "Error al conectar con la cola de mensajes (completados)" << endl;
        exit(1);
    }

    // Conectamos con la cola de mensajes de tipos de mensajes

    colaTipos = msgget(llaveColaTipos, 0666);

    if (colaTipos < 0)
    {
        cout << "Error al conectar con la cola de mensajes (tipos)" << endl;
        exit(1);
    }

    // Conectamos con la cola de mensajes de paquetes

    colaPaquetes = msgget(llaveColaPaquetes, 0666);

    if (colaPaquetes < 0)
    {
        cout << "Error al conectar con la cola de mensajes (paquetes)" << endl;
        exit(1);
    }

    // Conectamos con la memoria compartida

    memoriaCompartida = shmget(llaveMemoriaCompartida, sizeof(int), 0666);

    if (memoriaCompartida < 0)
    {
        cout << "Error al conectar con la memoria compartida" << endl;
        exit(1);
    }

    // Conectamos con el semaforo

    semaforo = semget(llaveSemaforo, 1, 0666);

    if (semaforo < 0)
    {
        cout << "Error al conectar con el semaforo" << endl;
        exit(1);
    }

    // Pedimos a la cola de tipos que nos asigne un tipo de mensaje para enviar a la cola de paquetes

    while (true)
    {
        if (msgrcv(colaTipos, &mensaje, sizeof(mensaje.mtext), 1, 0) > 0)
        {
            mtype = stoi(reinterpret_cast<char *>(mensaje.mtext));
            break;
        }
    }

    // Enviamos a la cola de paquetes como primer paquete el nombre de la imagen

    strcpy((char *)mensaje.mtext, nombre.c_str());
    mensaje.mtype = mtype;
    msgsnd(colaPaquetes, &mensaje, sizeof(mensaje.mtext), 0);

    vector512.resize(2);
    vector128.resize(6);
    direccion.append(nombre);

    // Creamos los hilos para leer, dividir y enviar paquetes de bytes

    thread t1(leer_bytes, ref(vector512), direccion);
    thread t2(dividir_bytes, ref(vector512), ref(vector128));
    thread t3(enviar_bytes, ref(vector128), mtype, colaPaquetes);

    // Esperamos a que los hilos terminen

    t1.join();
    t2.join();
    t3.join();

    // Enviamos un paquete indicando el final de imagen

    strcpy((char *)mensaje.mtext, "Fin de la imagen");
    mensaje.mtype = mtype + 1;
    msgsnd(colaPaquetes, &mensaje, sizeof(mensaje.mtext), 0);

    strcpy((char *)fin, "Fin de la imagen");

    // Conectamos con el contador de la memoria compartida

    contador = shmat(memoriaCompartida, NULL, 0);

    while (true)
    {
        if (msgrcv(colaCompletados, &mensaje, sizeof(mensaje.mtext), mtype + 1, 0) > 0)
        {
            if (memcmp(fin, mensaje.mtext, 17) == 0)
            {
                // Bloqueamos el aceso a la memoria compartida

                operacionSemaforo.sem_num = 0;
                operacionSemaforo.sem_op = -1;
                operacionSemaforo.sem_flg = 0;
                semop(semaforo, &operacionSemaforo, 1);

                // Disminuimos el contador para no crear mas contratistas

                *((int *)contador) -= 1;

                // Desbloqueamos el aceso a la memoria compartida

                operacionSemaforo.sem_num = 0;
                operacionSemaforo.sem_op = 1;
                operacionSemaforo.sem_flg = 0;
                semop(semaforo, &operacionSemaforo, 1);

                cout << "La imagen " << nombre << " fue enviada con exito." << endl;

                break;
            }
        }
    }

    // Desconectamos la memoria compartida

    shmdt(contador);
}
