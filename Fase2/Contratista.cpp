#include <cstring>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <sys/msg.h>
#include <errno.h>

using namespace std;

// Estructura para los mensajes que se quieren enviar y recibir
typedef struct Mi_Tipo_Mensaje
{
	long Id_Mensaje;
	char Mensaje[128];
};

void readImage(string direccion_img)
{
    FILE *imagen = fopen(direccion_img.c_str(), "r");
    key_t llave = 39;
	int cola_Mensajes;
	Mi_Tipo_Mensaje mensaje;
    char path[512];
    char mensaje_para_enviar[128];

    // Si la imagen no es cargada correctamente ERROR
    if( imagen == NULL) 
    {
        cout << "ERROR cargango imagen" << endl;
        exit(-1);
    }

	//	Se crea la cola de mensajes y se obtiene un identificador para ella.
	//  El IPC_CREAT indica que cree la cola de mensajes si no lo está ya.
	//  0600 permisos de lectura y escritura
	
	cola_Mensajes = msgget (llave, 0600 | IPC_CREAT);
	if (cola_Mensajes == -1)
	{
		cout << "Error al obtener identificador para cola mensajes" << endl;
		exit (-1);
	}

    strcpy(mensaje.Mensaje, direccion_img.c_str());

	//	Enviar el mensaje. Parámetros:
	//	- Id de la cola de mensajes.
	//	- Dirección al mensaje
	//	- Tamaño del mensaje
	//	- IPC_NOWAIT indica que si el mensaje no se puede enviar
	//	(habitualmente porque la cola de mensajes esta llena), que no espere
	//	y de un error. Si no se pone este flag, el programa queda bloqueado
	//	hasta que se pueda enviar el mensaje.
	//
	msgsnd (cola_Mensajes, &mensaje, sizeof(mensaje.Mensaje), IPC_NOWAIT);
}



int main(int argc, char *argv[])
{
    cout << "Start...." << endl;

    cout << "End...." << endl;
}

