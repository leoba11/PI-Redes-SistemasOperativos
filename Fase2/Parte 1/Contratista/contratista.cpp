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

struct msgbuffer
{
    long mtype;
    unsigned char mtext[128];
};

struct big_buffer
{
    int bandera = 0;
    unsigned char bytes[512];
};

struct small_buffer
{
    int bandera = 0;
    unsigned char bytes[512];
};

void leer_bytes(vector<big_buffer> &vector512, string direccion)
{
    FILE *imagen;
    int index = 0;
    struct big_buffer bigBuffer;
    unsigned char buffer[512];

    imagen = fopen(direccion.c_str(), "rb");

    if (imagen == NULL)
    {
        cout << "Error al leer la imagen" << endl;
        exit(1);
    }

    while (fread(buffer, 512, 1, imagen) == 1)
    {
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

void dividir_bytes(vector<big_buffer> &vector512, vector<small_buffer> &vector128)
{
    int bigIndex = 0;
    int smallIndex = 0;
    struct small_buffer smallBuffer;

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

void enviar_bytes(vector<small_buffer> &vector128, long mtype, int cola_paquetes)
{
    int smallIndex = 0;
    struct msgbuffer mensaje;

    while (vector128.at(smallIndex).bandera != 2)
    {
        while (true)
        {
            if (vector128.at(smallIndex).bandera == 1)
            {
                memcpy(mensaje.mtext, vector128.at(smallIndex).bytes, 128);
                mensaje.mtype = mtype;
                msgsnd(cola_paquetes, &mensaje, sizeof(mensaje.mtext), 0);

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
    unsigned char fin[17];
    vector<big_buffer> vector512;
    vector<small_buffer> vector128;
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

    vector512.resize(2);
    vector128.resize(6);
    direccion.append(nombre);

    thread t1(leer_bytes, ref(vector512), direccion);
    thread t2(dividir_bytes, ref(vector512), ref(vector128));
    thread t3(enviar_bytes, ref(vector128), mtype, cola_paquetes);

    t1.join();
    t2.join();
    t3.join();

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
}
