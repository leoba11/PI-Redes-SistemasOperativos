from queue import Queue
from threading import Thread
import re

def lector(cola):
	print('Ingrese la oración: ')
	oracion = input()
	cola.put(oracion)

def contador(cola):
	oracion = cola.get()
	print('La oración es: ' + oracion)
	oracion = oracion.replace(';', '; ')
	oracion = oracion.replace(',', ', ')
	oracion = oracion.replace('.', '. ')	
	numero = len(re.split('; |, |. |\n|\t',oracion))
	print('El número de palabras es: ' + str(numero))
	cola.task_done()

cola = Queue()
hilo1 = Thread(target = lector, args = (cola,))
hilo2 = Thread(target = contador, args = (cola,))

hilo1.start()
hilo2.start()
