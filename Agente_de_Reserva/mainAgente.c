/******************************************************************
*            Pontificia Universidad Javeriana                     *
*                         ---=---                                 *
*                                                                 *
* Autor: Danna Gabriela Rojas & María Fernanda Velandia           *
* Materia: Sistemas Operativos                                    *
* Docente: J. Corredor                                            *
* Fecha: 18 Noviembre del 2025                                    *
* Tema: Proyecto Parque Berlín                                    *
* Archivo: mainAgente.c                                           *
* Objetivo: Implementar el punto de entrada del proceso Agente,   *
*           encargado de leer parámetros, inicializar el agente   *
*           y ejecutar su ciclo de operación.                     *
* Descripción:                                                    *
*    - Procesa los argumentos de línea de comandos.               *
*    - Configura la estructura ParametrosAgente.                  *
*    - Llama a las funciones de inicialización, ejecución y fin.  *
******************************************************************/

// Incluye funciones y estructuras de Agente
#include "Agente.h"

int main(int argc, char* argv[]) {
    // Estructura donde guardar los parámetros
    ParametrosAgente params;
    strcpy(params.nombreAgente, "AgenteDefault"); // Nombre por defecto
    strcpy(params.archivoSolicitudes, "solicitudes.txt"); // Archivo csv por defecto
    strcpy(params.pipeEnvio, "/tmp/pipeReserva"); // Pipe por defecto
    // Recorrer argumentos recibidos
    for (int i = 1; i < argc; i += 2) {
    	// Verifica que exista valor posterior al drgumento
        if (i + 1 >= argc) {
            fprintf(stderr, "Error: Falta valor para el parámetro %s\n", argv[i]);
            fprintf(stderr, "Uso: %s -s <nombre> -a <fileSolicitud> -p <pipeRecibe>\n", argv[0]);
            exit(EXIT_FAILURE); // Error de uso
        }
        // Si es nombre del agente
        if (strcmp(argv[i], "-s") == 0) {
            strcpy(params.nombreAgente, argv[i + 1]); // Guarda el nuevo nombre
        // Si es el archivo
        } else if (strcmp(argv[i], "-a") == 0) {
            strcpy(params.archivoSolicitudes, argv[i + 1]); // Guarda el archivo
        // Si es el pipe
        } else if (strcmp(argv[i], "-p") == 0) {
            strcpy(params.pipeEnvio, argv[i + 1]); // Guarda el pipe
        // Parámetro no existente
        } else {
            fprintf(stderr, "Error: Parámetro desconocido %s\n", argv[i]);
            fprintf(stderr, "Uso: %s -s <nombre> -a <fileSolicitud> -p <pipeRecibe>\n", argv[0]);
            exit(EXIT_FAILURE); // Parámetro inválido, termina
        }
    }
    // Llama la inicialización
    if (!inicializarAgente(&params)) {
        imprimirMensajeError("No se pudo inicializar el agente"); // Si falla muestra los errores
        exit(EXIT_FAILURE); // Si no puede iniciar termina la ejecución
    }
    // Realiza el registro
    ejecutarAgente();
    // Limpia y elima el pipe
    finalizarAgente();
    
    return EXIT_SUCCESS; // Finalización con éxito
}
