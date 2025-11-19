/******************************************************************
*            Pontificia Universidad Javeriana                     *
*                         ---=---                                 *
*                                                                 *
* Autor: Danna Gabriela Rojas & María Fernanda Velandia           *
* Materia: Sistemas Operativos                                    *
* Docente: J. Corredor                                            *
* Fecha: 18 Noviembre del 2025                                    *
* Tema: Proyecto Parque Berlín                                    *
* Archivo: Sistema.h                                              *
* Objetivo: Declarar las constantes, estructuras compartidas y    *
*           definiciones globales utilizadas por agentes y        *
*           controlador en el sistema de reservas.                *
* Descripción:                                                    *
*    - Define los tipos de mensajes y respuestas.                 *
*    - Contiene las estructuras Solicitud, Respuesta y Reserva.   *
*    - Define funciones auxiliares para validaciones y mensajes.  *
******************************************************************/

#ifndef SISTEMA_H
#define SISTEMA_H
// Librerías a utilizar
#include <stdio.h> // Manejo de entrada y salida
#include <stdlib.h> // Funciones como exit
#include <string.h> // Manejo de cadenas
#include <unistd.h> // Lamadas a POSIX
#include <fcntl.h> // Control de archivo
#include <sys/stat.h> // Permisos y creación de pipe
#include <sys/types.h> //Definiciones de tipos al sistema
#include <signal.h> // Señales mandadas
#include <pthread.h> // Manejo de hilos
#include <time.h> // Para tiempos y fechas
#include <errno.h> // Errores presentados

// Constantes globales
#define MAX_NOMBRE 100 // Tamaño máximo
#define MAX_BUFFER 512 // Tamño del buffer 
#define HORAS_DIA 12  // Horas del día de 7 a 19
#define HORA_MIN 7 // Hora mínima de reserva
#define HORA_MAX 19 // Hora máxima de reserva
#define DURACION_RESERVA 2 // Duración en el parque
#define MAX_RESERVAS 1000 // Máximo de reservas

// Tipos de mensaje que se puede mandar
typedef enum {
    MSG_REGISTRO, // Agente se registra
    MSG_SOLICITUD, // Solicitud de reserva
    MSG_RESPUESTA, // Respuesta del controlador
    MSG_FIN // Finalización
} TipoMensaje;

// Tipos de respuestas que el controlador envía al agente
typedef enum {
    RESP_OK, // Reserva aprobada sin cambios
    RESP_REPROGRAMADA, // Reserva aprobada pero con cambio de hora
    RESP_NEGADA_EXTEMPORANEA, // Hora fuera del rango
    RESP_NEGADA_VOLVER // Reserva negada por aforo
} TipoRespuesta;

// Estructrua que envia el agente
typedef struct {
    TipoMensaje tipo; // Tipo de mensaje enviado
    char nombreAgente[MAX_NOMBRE]; // Nombre del agente
    char nombreFamilia[MAX_NOMBRE]; // Familia solicitante
    char pipeRespuesta[MAX_NOMBRE]; // Pipe que recibe respuesta
    int hora; // Hora de reserva
    int numPersonas; // Personas por familia
} Solicitud;

// Estructura que envía el controlador
typedef struct {
    TipoRespuesta tipo;// Respuesta generada
    int horaAsignada; // Hora final designada
    int horaActual; // Hora actual de la simulación
    char mensaje[MAX_BUFFER]; // Mensaje descriptivo
} Respuesta;

// Reserva
typedef struct {
    char nombreFamilia[MAX_NOMBRE]; // Familia que reservó
    char nombreAgente[MAX_NOMBRE]; // Agente que realizó la reserva
    int numPersonas; // Cantidad integrantes
    int horaInicio; // Hora de entrada
    int horaFin; // Hora de salida
} Reserva;

// Estructuras de contadores globales
typedef struct {
    int solicitudesAceptadas; // Cantidad reservas aceptadas
    int solicitudesReprogramadas;// Cantidad reservas cambiadas por hora
    int solicitudesNegadas; // Cantidad reservas rechazas
    int personasPorHora[HORAS_DIA + 1]; // Cantidad de personas por hora
} Estadisticas;

// Imprime un mensaje de error
void imprimirMensajeError(const char* mensaje);
// Mensaje de informe
void imprimirMensajeInfo(const char* mensaje);
// Verifica que la hora este en el rango
int validarHora(int hora);
// Verifica intervalo en el rango
int validarRangoHoras(int horaIni, int horaFin);

#endif // SISTEMA_H
