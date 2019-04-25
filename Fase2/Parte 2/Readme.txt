Grupo 404, Fase 2, Parte 1, Entrega 3

Instrucciones:

1) Compilar el receptor: g++ receptor.cpp -pthread -o receptor
2) Ejecutar el receptor: ./receptor
3) Compilar el emisor: g++ emisor.cpp -pthread -o emisor
4) Ejecutar el emisor: ipcrm -a && ./emisor
5) Ingresar la IP y el puerto al emisor
6) Compilar el lector: g++ lector.cpp -pthread -o lector
7) Ejecutar el lector: ./lector
8) Ingresar una dirección al lecto
9) Repetir pasos 7 y 8 con las carpetas de desee

Notas:

1) Las imágenes serán copiadas donde sea que este el archivo receptor
2) ipcrm -a es necesario para limpiar las colas del sistema antes de iniciar el programa
4) En caso de cerrar el emisor o el receptor, deberá abrir todos los programas de nuevo para su respectiva sincronización
5) Para cerrar el emisor o el receptor, precionar ctrl + c
