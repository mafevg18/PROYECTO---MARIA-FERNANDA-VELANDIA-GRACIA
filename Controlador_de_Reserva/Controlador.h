/******************************************************************
*            Pontificia Universidad Javeriana                     *
*                         ---=---                                 *
*                                                                 *
* Autor: Danna Gabriela Rojas & María Fernanda Velandia           *
* Materia: Sistemas Operativos                                    *
* Docente: J. Corredor                                            *
* Fecha: 18 Noviembre del 2025                                    *
* Tema: Proyecto Parque Berlín                                    *
* Archivo: Controlador.h                                          *
* Objetivo: Declarar las estructuras, funciones y operaciones     *
*           necesarias para el proceso Controlador del sistema.   *
* Descripción:                                                    *
*    - Define la estructura de parámetros del controlador.        *
*    - Declara funciones de inicialización, ejecución y cierre.   *
*    - Declara hilos de simulación de reloj y recepción de        *
*      solicitudes.                                               *
*    - Declara funciones para procesar registros, solicitudes y   *
*      generar reportes del sistema.                              *
******************************************************************/

#ifndef CONTROLADOR_H
#define CONTROLADOR_H
// Incluye estructuras en común
#include "Sistema.h"

//Estructura parámetros del controlador
typedef struct {
    int horaInicio; // Hora de apertura
    int horaFin; // Hora de cierre
    int segHoras; // Segundos que corresponden a un ahora
    int aforoMaximo; // Máximo de personas en el parque por hora
    char pipeRecibe[MAX_NOMBRE]; // Nombre del pipe que recibe las solicitudes
} ParametrosControlador;

// Inicializa el controlador con los paŕametros leidos
int inicializarControlador(ParametrosControlador* params);
// Ejecuta el controlador(hilos y las solicitudes a procesar)
void ejecutarControlador();
// Libera los recursos del controlador
void finalizarControlador();

// Hilo que simula el reloj(hora del sistema)
void* threadReloj(void* arg);
// Hilo que escucha las solicitudes del pipe y las procesa
void* threadPeticiones(void* arg);

// Procesa el mensaje de registro dado por el agente
void procesarRegistro(Solicitud* sol, int fdEscritura);
// Procesa la solicitud enviada por el agente
void procesarSolicitud(Solicitud* sol, int fdEscritura);

// Cuantas personas hay en una hora en especifíco
int calcularPersonasEnHora(int hora);
// BUsca una hora disponible para asignar la reserva
int buscarBloqueDisponible(int numPersonas, int* horaEncontrada);
// Avanza el tiempo del sistema
void avanzarHora();
// Imprime el reporte del día
void imprimirReporte();

// Maneja comandos para cerrar o detener el controlador
void manejarSenalFin(int sig);

#endif // CONTROLADOR_H
