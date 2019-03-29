import os
import time
import multiprocessing
import re

semaforo1 = multiprocessing.Semaphore(1)
semaforo2 = multiprocessing.Semaphore(0)
lista = multiprocessing.Manager().list()
salir = multiprocessing.Manager().dict({'check' : False})

def lector():
	semaforo2.acquire()
	print('Ingrese la oración: ')
	oracion = input()
	if('1' == oracion):
		for indice in lista:
			print('Oración: ' + indice[0] + ', Número de palabras: ' + str(indice[1]))
		semaforo1.release()
	elif('exit' == oracion):
		salir['check'] = True
		semaforo1.release()
	else:
		semaforo1.release()
		numero = contador(oracion)
		lista.append((oracion,numero))
	os._exit(0)

def contador(oracion):
	time.sleep(int(tiempo))
	oracion = oracion.replace(';', '; ')
	oracion = oracion.replace(',', ', ')
	oracion = oracion.replace('.', '. ')
	numero = len(re.split('; |, |. |\n|\t',oracion))
	return numero

def main():
	global tiempo

	while True:
		print('Ingrese el tiempo de espera para el contador: ')
		tiempo = input()
		if(tiempo.isdigit()):
		    break
		else:
		    print('El valor ingresado debe ser un número')

	while False == salir['check']:
		semaforo1.acquire()
		if False == salir['check']:
			pid = os.fork()
			if 0 == pid:
				lector()
		semaforo2.release()

main()
