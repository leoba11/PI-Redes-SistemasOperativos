import socket
import sys
import threading
from queue import Queue

print('Digite el IP del servidor:')
host = input()

while True:
    print('Digite el puerto del servidor:')
    port = input()
    if(port.isdigit()):
        break
    else:
        print('El valor ingresado debe ser un número')

try:
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    print('\nSocket creado correctamente')
except socket.error as fail:
    print('\nNo se pudo crear el socket. Error: %s' % (fail,))
    sys.exit(1)

try:
    s.connect((host, int(port)))
except socket.error as fail:
    print('No se pudo establecer la conexión. Error: %s' % (fail,))
    sys.exit(1)

def lector(cola, lista_oraciones, lista_numeros):
    mensaje = ''
    while mensaje != 'exit':
        print('\nEscriba una oración:')
        mensaje = input()
        if ('1' == mensaje):
            print(lista_oraciones)
            print(lista_numeros)
            for i in range(len(lista_numeros)):
                print('\nOración: ' + str(lista_oraciones[i]) + '\nNúmero de palabras: ' + str(lista_numeros[i]))
        else:
            cola.put(mensaje)
        
def mensajero(cola, lista_oraciones):
    mensaje = ''
    while mensaje != 'exit' and lector_thread.isAlive():
        mensaje = cola.get()
        lista_oraciones.append(str(mensaje))
        mensaje = mensaje + '\n'
        try:
            s.settimeout(1)
            s.send(mensaje.encode('utf-8'))
        except socket.timeout:
            if lector_thread.isAlive():
                pass
            else:
                break
        except socket.error:
            break

def receptor(cola, lista_numeros):
    mensaje = ''
    while True and lector_thread.isAlive():
        while not '\n' in mensaje:
            try:
                s.settimeout(1)
                auxiliar = s.recv(1024).decode('utf-8')
                if auxiliar == '':
                    mensaje = 'exit\n'
                    break
                else:
                    mensaje = mensaje + auxiliar
            except socket.timeout:
                if lector_thread.isAlive():
                    pass
                else:
                    break
            except socket.error:
                break
        if mensaje == 'exit\n':
            break
        i = mensaje.find('\n')
        numero = mensaje[:i]
        mensaje = mensaje[i + 1:]
        lista_numeros.append(str(numero))

cola = Queue()
lista_oraciones = []
lista_numeros = []
mensaje = s.recv(1024)
print(mensaje.decode('utf-8'))
lector_thread = threading.Thread(target = lector, args = (cola, lista_oraciones, lista_numeros,))
lector_thread.start()
receptor_thread = threading.Thread(target = receptor, args = (cola, lista_numeros,))
receptor_thread.start()
mensajero_thread = threading.Thread(target = mensajero, args = (cola, lista_oraciones,))
mensajero_thread.start()
s.close
