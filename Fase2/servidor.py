import re
import socket
import sys
import threading
import time
from queue import Queue

host = ''
port = 5000

try:
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    print('Socket creado correctamente')
except socket.error as fail:
    print('No se pudo crear el socket. Error: %s' % (fail,))
    sys.exit(1)

try:
    s.bind((host, port))
    print('Enlace establecido correctamente en el puerto %s' % (port,))
except socket.error as fail:
    print('No se pudo establecer el enlace. Error: %s' % (fail,))
    sys.exit(1)

s.listen(1)
print('Socket listo')

def receptor(conexion, cola_oraciones):
    mensaje = ''
    while True and consola_thread.isAlive():
        while not '\n' in mensaje:
            try:
                conexion.settimeout(1)
                auxiliar = conexion.recv(1024).decode('utf-8')
                if auxiliar == '':
                    mensaje = 'exit\n'
                    break
                else:
                    mensaje = mensaje + auxiliar
            except socket.timeout:
                if consola_thread.isAlive():
                    pass
                else:
                    mensaje = 'exit\n'
                    break
            except socket.error:
                mensaje = 'exit\n'
                break
        if mensaje == 'exit\n':
            break
        i = mensaje.find('\n')
        oracion = mensaje[:i]
        mensaje = mensaje[i + 1:]
        cola_oraciones.put(oracion)
    conexion.close()

def contador(cola_oraciones, cola_numeros, indice, registro, tiempo):
    while receptor_thread.isAlive() and consola_thread.isAlive():
        while not cola_oraciones.empty() and receptor_thread.isAlive() and consola_thread.isAlive():
            oracion = cola_oraciones.get()
            contador = oracion
            contador = contador.replace(';', '; ')
            contador = contador.replace(',', ', ')
            contador = contador.replace('.', '. ')	
            numero = len(re.split('; |, |. |\t', contador))
            time.sleep(int(tiempo))
            registro[int(indice)].append((str(oracion),str(numero)))
            cola_numeros.put(str(numero))

def mensajero(conexion, cola_numeros):
    numero = ''
    while receptor_thread.isAlive() and consola_thread.isAlive():
        while not cola_numeros.empty():
            numero = cola_numeros.get()
            cola_numeros.task_done()
            numero = str(numero) + '\n'
            try:
                conexion.settimeout(1)
                conexion.send(numero.encode('utf-8'))
            except socket.timeout:
                if consola_thread.isAlive():
                    pass
                else:
                    numero = 'exit\n'
                    break
            except socket.error:
                numero = 'exit\n'
                break
        if numero == 'exit\n':
            break
    conexion.close()

def consola(registro):
    while True:
        print('\nDigite el número que corresponde a la opción que desea seleccionar\n1) Buscar datos por IP y puerto\n2) Lista de IPs y puertos\n3) Salir')
        option = input()
        if ('1' == option):
            print("\nDigite el IP del cliente")
            ip_search = input()
            print("Digite el puerto del cliente")
            port_search = input()
            check = False
            for i in range(len(registro)):
                if registro[i][0][0] == str(ip_search) and registro[i][0][1] == int(port_search):
                    if len(registro[i]) == 1:
                         print ('\nNo existen datos para la IP ' + str(ip_search) + ' y el puerto ' + str(port_search))
                    else:
                        for j in range(len(registro[i]) - 1):
                            print('\nOración: ' + str(registro[i][j + 1][0]) + '\nNúmero de palabras: ' + str(registro[i][j + 1][1]))
                    check = True
                    break
            if check == False:
                print ('\nLa IP ' + str(ip_search) + ' con el puerto ' + str(port_search) + ' no ha sido conectada a este servidor')
        elif ('2' == option):
            if len(registro) == 0:
                print ('\nNo existen datos de IPs y puertos registrados aún')
            else:
                for i in range(len(registro)):
                    print('\nIP: ' + str(registro[i][0][0]) + ' Puerto: ' + str(registro[i][0][1]))
        elif ('3' == option):
            break

registro = []
tiempo = 0

while True:
		print('\nIngrese el tiempo de espera para el contador:')
		tiempo = input()
		if(tiempo.isdigit()):
		    break
		else:
		    print('El valor ingresado debe ser un número')

consola_thread = threading.Thread(target = consola, args = (registro,))
consola_thread.start()

while consola_thread.isAlive():
    try:
        s.settimeout(1)
        conexion, addr = s.accept()
    except socket.timeout:
        pass
    else:
        indice = -1
        for i in range(len(registro)):
            if registro[i][0][0] == addr[0] and registro[i][0][1] == addr[1]:
                indice = i
        if indice == -1:
            indice = len(registro)
            registro.append([(addr[0], addr[1])])
        mensaje = 'Conexión establecida con el servidor'
        conexion.send(mensaje.encode('utf-8'))
        cola_oraciones = Queue()
        cola_numeros = Queue()
        receptor_thread = threading.Thread(target = receptor, args = (conexion, cola_oraciones,))
        receptor_thread.start()
        mensajero_thread = threading.Thread(target = mensajero, args = (conexion, cola_numeros,))
        mensajero_thread.start()
        contador_thread = threading.Thread(target = contador, args = (cola_oraciones, cola_numeros, indice, registro, tiempo,))
        contador_thread.start()

s.close
