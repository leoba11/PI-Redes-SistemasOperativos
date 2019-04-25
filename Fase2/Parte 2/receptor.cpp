#include <algorithm>
#include <arpa/inet.h>
#include <iostream>
#include <mutex>
#include <netdb.h>
#include <queue>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>

#define PORT 7777

using namespace std;

// Candados para sincronizar los hilos con un solo receptor de paquetes TCP

mutex mtxHilo1;
mutex mtxHilo2;

// Estructura de los paquetes

#pragma pack(1)
struct msgbuffer
{
	long mtype;
	unsigned char mtext[128];
};
#pragma pack(0)

void armar_imagen(queue<msgbuffer> &paquetes, int socketCliente, int numeroHilo)
{
	while (paquetes.empty() == true)
		;

	FILE *imagen;
	long tipoActual;
	string nombre;

	// Obtenemos el nombre de la imagen

	nombre = (char *)paquetes.front().mtext;
	tipoActual = paquetes.front().mtype;

	if (numeroHilo == 0)
	{
		mtxHilo1.lock();
		paquetes.pop();
		mtxHilo1.unlock();
	}
	else
	{
		mtxHilo2.lock();
		paquetes.pop();
		mtxHilo2.unlock();
	}

	// Creamos la imagen para escribir los paquetes

	imagen = fopen(nombre.c_str(), "wb");

	if (imagen == NULL)
	{
		cout << "Error al crear la imagen" << endl;
		exit(1);
	}

	// Guardamos todos los paquetes de la imagen

	while (true)
	{
		if (paquetes.empty() == true)
		{
			continue;
		}
		else if (paquetes.front().mtype != (tipoActual + 1))
		{
			fwrite(paquetes.front().mtext, 1, 128, imagen);

			if (numeroHilo == 0)
			{
				mtxHilo1.lock();
				paquetes.pop();
				mtxHilo1.unlock();
			}
			else
			{
				mtxHilo2.lock();
				paquetes.pop();
				mtxHilo2.unlock();
			}
		}
		else
		{
			break;
		}
	}

	// Cerramos la imagen

	fclose(imagen);

	// Enviamos un mensaje indicando que la imagen terminó de copiarse

	send(socketCliente, &paquetes.front(), sizeof(struct msgbuffer), 0);

	if (numeroHilo == 0)
	{
		mtxHilo1.lock();
		paquetes.pop();
		mtxHilo1.unlock();
	}
	else
	{
		mtxHilo2.lock();
		paquetes.pop();
		mtxHilo2.unlock();
	}
}

void receptor()
{
	int conexion;
	int sock;
	int socketCliente;
	int tiposHilo1;
	int tiposHilo2;
	queue<msgbuffer> colaHilo1;
	queue<msgbuffer> colaHilo2;
	sockaddr_in cliente;
	sockaddr_in direccion;
	socklen_t tamanoCliente;
	struct msgbuffer mensaje;
	thread hilos[2];
	vector<long> tiposUsados;

	// Creamos un socket

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
	direccion.sin_port = htons(PORT);
	inet_pton(AF_INET, "0.0.0.0", &direccion.sin_addr);

	// Hacemos blind

	if (bind(sock, (sockaddr *)&direccion, sizeof(direccion)) == -1)
	{
		cout << "No se pudo establecer el enlace" << endl;
		exit(1);
	}
	else
	{
		cout << "Enlace establecido en el puerto " << PORT << endl;
	}

	// Abrimos un puerto para escuchar

	if (listen(sock, 1) == -1)
	{
		cout << "No se pudo configurar el socket para escuchar clientes" << endl;
		exit(1);
	}

	tamanoCliente = sizeof(cliente);
	char host[NI_MAXHOST];
	char svc[NI_MAXSERV];

	// Conectamos con el cliente

	socketCliente = accept(sock, (sockaddr *)&cliente, &tamanoCliente);

	if (socketCliente == -1)
	{
		cout << "No se pudo conectar con el cliente" << endl;
		exit(1);
	}

	memset(host, 0, NI_MAXHOST);
	memset(svc, 0, NI_MAXSERV);

	// Mostramos la IP y el puesto de la coneccion con el cliente

	if (conexion = getnameinfo((sockaddr *)&cliente, tamanoCliente, host, NI_MAXHOST, svc, NI_MAXSERV, 0))
	{
		cout << host << " conectado en el puerto " << svc << endl;
	}
	else
	{
		inet_ntop(AF_INET, &cliente.sin_addr, host, NI_MAXHOST);
		cout << host << " conectado en el puerto " << ntohs(cliente.sin_port) << endl;
	}

	int indiceHilos = 0;
	indiceHilos = 0;

	while (true)
	{
		// Recibimos los paquetes multiplexados

		recv(socketCliente, &mensaje, sizeof(struct msgbuffer), 0);

		// Identificamos los paquetes nuevos y creamos un thread para armar la imagen o desmultiplexarlos

		if (((mensaje.mtype % 2) != 0) || find(tiposUsados.begin(), tiposUsados.end(), mensaje.mtype) != tiposUsados.end())
		{
			// Identificamos los paquetes y los desmultiplexamos

			while (true)
			{
				if (mensaje.mtype == tiposHilo1)
				{
					if (colaHilo1.size() < 5)
					{
						mtxHilo1.lock();
						colaHilo1.push(mensaje);
						mtxHilo1.unlock();
						break;
					}
				}
				else if (mensaje.mtype == tiposHilo2)
				{
					if (colaHilo2.size() < 5)
					{
						mtxHilo2.lock();
						colaHilo2.push(mensaje);
						mtxHilo2.unlock();
						break;
					}
				}
				else if (mensaje.mtype == tiposHilo1 + 1)
				{
					if (colaHilo1.size() < 5)
					{
						mtxHilo1.lock();
						colaHilo1.push(mensaje);
						mtxHilo1.unlock();
						break;
					}
				}
				else if (mensaje.mtype == tiposHilo2 + 1)
				{
					if (colaHilo2.size() < 5)
					{
						mtxHilo2.lock();
						colaHilo2.push(mensaje);
						mtxHilo2.unlock();
						break;
					}
				}
			}
		}

		// Creamos un thread para los paquetes recien identificados y empezar a generar la imagen

		else
		{
			if (indiceHilos == 0)
			{
				if (tiposUsados.size() > 1)
				{
					hilos[0].join();
				}
				hilos[indiceHilos] = thread(armar_imagen, ref(colaHilo1), socketCliente, indiceHilos);
				tiposHilo1 = mensaje.mtype;
				mtxHilo1.lock();
				colaHilo1.push(mensaje);
				mtxHilo1.unlock();
			}
			else
			{
				if (tiposUsados.size() > 1)
				{
					hilos[1].join();
				}
				hilos[indiceHilos] = thread(armar_imagen, ref(colaHilo2), socketCliente, indiceHilos);
				tiposHilo2 = mensaje.mtype;
				mtxHilo2.lock();
				colaHilo2.push(mensaje);
				mtxHilo2.unlock();
			}
			tiposUsados.push_back(mensaje.mtype);
			indiceHilos = (indiceHilos + 1) % 2;
		}
	}

	// Cerramos la conexion con el cliente

	close(socketCliente);

	// Cerramos el socket

	close(sock);
}

int main()
{
	receptor();
	return 0;
}
