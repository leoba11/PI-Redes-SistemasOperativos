#include <algorithm>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <string>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
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

void imagenes_terminadas(int sock, int colaCompletados)
{
    struct msgbuffer confirmacion;

    // Esperamos por el servidor TCP por un mensaje indicando que alguna imagen termino de copiarse y cual

    while (true)
    {
        recv(sock, &confirmacion, sizeof(struct msgbuffer), 0);
        msgsnd(colaCompletados, &confirmacion, sizeof(confirmacion.mtext), 0);
    }
}

void emisor()
{
    int colaCompletados;
    int colaTipos;
    int colaPaquetes;
    int conexion;
    int port;
    int sock;
    long mtype = 2;
    key_t llaveColaCompletados = 0x9191;
    key_t llaveColaTipos = 0x4444;
    key_t llaveColaPaquetes = 0x9999;
    sockaddr_in direccion;
    string host;
    string nombre;
    struct msgbuffer mensaje;
    vector<long> tiposUsados;

    cout << "Digite el IP del servidor: " << endl;
    cin >> host;

    while (true)
    {
        cout << "Digite el puerto del servidor: " << endl;
        cin >> port;
        if (cin.fail())
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "El valor ingresado debe ser un número." << endl;
        }
        else
        {
            if (port < 1024)
            {
                cout << "El puerto debe ser un número mayor a 1024." << endl;
            }
            else
            {
                break;
            }
        }
    }

    // Creamos el socket

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock == -1)
    {
        cout << "No se pudo crear el socket" << endl;
        exit(1);
    }
    else
    {
        cout << "Socket creado correctamente" << endl;
    }

    direccion.sin_family = AF_INET;
    direccion.sin_port = htons(port);
    inet_pton(AF_INET, host.c_str(), &direccion.sin_addr);

    // Conectamos con el servidor

    conexion = connect(sock, (sockaddr *)&direccion, sizeof(direccion));

    if (conexion == -1)
    {
        cout << "No se pudo establecer la conexión" << endl;
        exit(1);
    }
    else
    {
        cout << "Conexión establecida con el servidor" << endl;
    }

    // Creamos la cola de imagenes completadas

    colaCompletados = msgget(llaveColaCompletados, IPC_CREAT | 0666);

    if (colaCompletados < 0)
    {
        cout << "Error al crear la cola de mensajes (completados)" << endl;
        exit(1);
    }

    // Creamos la cola de tipos de mensaje

    colaTipos = msgget(llaveColaTipos, IPC_CREAT | 0666);

    if (colaTipos < 0)
    {
        cout << "Error al crear la cola de mensajes (tipos)" << endl;
        exit(1);
    }

    // Creamos la cola de paquetes

    colaPaquetes = msgget(llaveColaPaquetes, IPC_CREAT | 0666);

    if (colaPaquetes < 0)
    {
        cout << "Error al crear la cola de mensajes (paquetes)" << endl;
        exit(1);
    }

    // Enviamos a la cola de tipos, que tipo de mensaje vamos a leer para un contratista

    strcpy((char *)mensaje.mtext, to_string(mtype).c_str());
    mensaje.mtype = 1;
    msgsnd(colaTipos, &mensaje, sizeof(mensaje.mtext), 0);

    // Creamos un hilo el cual espera por imagenes completadas del servidor TCP

    thread t1(imagenes_terminadas, sock, colaCompletados);

    while (true)
    {
        // Leemos todos los paquetes de todos los contratistas

        if (msgrcv(colaPaquetes, &mensaje, sizeof(mensaje.mtext), 0, 0) > 0)
        {
            // Enviamos los paquetes al servidor

            if (send(sock, &mensaje, sizeof(struct msgbuffer), 0) == -1)
            {
                cout << "Error al enviar paquetes";
                exit(1);
            }

            // Guardamos el tipo de mensaje en caso de ser el primero y ponemos a dispocision mas tipos de mensaje para los contratistas

            if (find(tiposUsados.begin(), tiposUsados.end(), mensaje.mtype) != tiposUsados.end())
            {
                continue;
            }
            else
            {
                tiposUsados.push_back(mensaje.mtype);
                mtype += 2;
                strcpy((char *)mensaje.mtext, to_string(mtype).c_str());
                mensaje.mtype = 1;
                msgsnd(colaTipos, &mensaje, sizeof(mensaje.mtext), 0);
            }
        }
    }

    t1.join();
}

int main()
{
    emisor();
    return 0;
}
