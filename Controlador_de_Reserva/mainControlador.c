/******************************************************************
*            Pontificia Universidad Javeriana                     *
*                         ---=---                                 *
*                                                                 *
* Autor: Danna Gabriela Rojas & María Fernanda Velandia           *
* Materia: Sistemas Operativos                                    *
* Docente: J. Corredor                                            *
* Fecha: 18 Noviembre del 2025                                    *
* Tema: Proyecto Parque Berlín                                    *
* Archivo: mainControlador.c                                      *
* Objetivo: Implementar el punto de entrada del proceso           *
*           Controlador, encargado de iniciar los recursos y      *
*           ejecutar la lógica principal del sistema de reservas. *
* Descripción:                                                    *
*    - Procesa parámetros desde línea de comandos.                *
*    - Configura la estructura ParametrosControlador.             *
*    - Inicializa el controlador, ejecuta los hilos de trabajo    *
*      y libera recursos al finalizar.                            *
******************************************************************/

// Incluye estructuras del controlador
#include "Controlador.h"
// FUnción principal
int main(int argc, char* argv[]) {
    // Estructuras donde se almacenan los parámetros
    ParametrosControlador params;
    // Valores por defecto
    params.horaInicio = 7; // Hora mínima de reserva
    params.horaFin = 19; // Hora máximo de reserva
    params.segHoras = 5; // Segundos que simulan hora
    params.aforoMaximo = 50; // Máximo personas en el parque por hora
    strcpy(params.pipeRecibe, "/tmp/pipeReserva"); // Pipe para procesar solicitudes
    // Procesa los argumentos
    for (int i = 1; i < argc; i += 2) {
    	// Valor después del parámetro
        if (i + 1 >= argc) {
            fprintf(stderr, "Error: Falta valor para el parámetro %s\n", argv[i]);
            fprintf(stderr, "Uso: %s -i <horaIni> -f <horaFin> -s <segHoras> -t <total> -p <pipeRecibe>\n", argv[0]);
            exit(EXIT_FAILURE);
        }
        if (strcmp(argv[i], "-i") == 0) {
            params.horaInicio = atoi(argv[i + 1]); // Carga hora de inicio
        } else if (strcmp(argv[i], "-f") == 0) {
            params.horaFin = atoi(argv[i + 1]); // Carga hora final
        } else if (strcmp(argv[i], "-s") == 0) {
            params.segHoras = atoi(argv[i + 1]); // Carga de simulación hora
        } else if (strcmp(argv[i], "-t") == 0) {
            params.aforoMaximo = atoi(argv[i + 1]); // Carga el aforo máximo
        } else if (strcmp(argv[i], "-p") == 0) {
            strcpy(params.pipeRecibe, argv[i + 1]); // Carga el nombre del pipe
        } else {
            // Si ingresan un parámetro diferente
            fprintf(stderr, "Error: Parámetro desconocido %s\n", argv[i]);
            fprintf(stderr, "Uso: %s -i <horaIni> -f <horaFin> -s <segHoras> -t <total> -p <pipeRecibe>\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    // Inicializa los recursos del controlador
    if (!inicializarControlador(&params)) {
        imprimirMensajeError("No se pudo inicializar el controlador");
        exit(EXIT_FAILURE);
    }
    // Ejecuta los hilos principales
    ejecutarControlador();
    // Libera los recursos
    finalizarControlador();
    
    return EXIT_SUCCESS; // FInalizo bien 
}
