/******************************************************************
*            Pontificia Universidad Javeriana                     *
*                         ---=---                                 *
*                                                                 *
* Autor: Danna Gabriela Rojas & María Fernanda Velandia           *
* Materia: Sistemas Operativos                                    *
* Docente: J. Corredor                                            *
* Fecha: 18 Noviembre del 2025                                    *
* Tema: Proyecto Parque Berlín                                    *
* Archivo: Agente.h                                               *
* Objetivo: Declarar las estructuras y funciones necesarias para  *
*           el funcionamiento del proceso Agente.                 *
* Descripción:                                                    *
*    - Define la estructura ParametrosAgente.                     *
*    - Declara funciones de inicialización, ejecución y cierre.   *
*    - Declara funciones para procesar solicitudes y comunicar    *
*      al agente con el controlador mediante pipes.               *
******************************************************************/

#ifndef AGENTE_H
#define AGENTE_H
// Incluye las estructuras compartidas
#include "Sistema.h"

// Estructura de los parámetros del agente
typedef struct {
    char nombreAgente[MAX_NOMBRE]; // Nombre que identifica el agente
    char archivoSolicitudes[MAX_NOMBRE]; // Ruta de las solicitudes
    char pipeEnvio[MAX_NOMBRE]; // Nombre del pipe
} ParametrosAgente;

// Funciones principales
// Inicializa el agente
int inicializarAgente(ParametrosAgente* params);
// Ejecuta el registro y procesamiento de solicitudes
void ejecutarAgente();
// Libera recursos del agente y su pipe
void finalizarAgente();

// Funciones de operación
// Envia mensaje al controlador 
int registrarseConControlador();
// Lee el archivo y envía cada solicitud
void procesarSolicitudes();
//Constuye y envía tipo MSG_SOLICITUD
void enviarSolicitud(char* familia, int hora, int personas);

// Lee una línea del archivo y extrae la información
int leerLineaCSV(FILE* archivo, char* familia, int* hora, int* personas);
// Se muestra la respuesta recibida
void imprimirRespuesta(Respuesta* resp, const char* familia);

#endif // AGENTE_H
