/******************************************************************
*            Pontificia Universidad Javeriana                     *
*                         ---=---                                 *
*                                                                 *
* Autor: Danna Gabriela Rojas & María Fernanda Velandia           *
* Materia: Sistemas Operativos                                    *
* Docente: J. Corredor                                            *
* Fecha: 18 Noviembre del 2025                                    *
* Tema: Proyecto Parque Berlín                                    *
* Archivo: Sistema.c                                              *
* Objetivo: Implementar las funciones auxiliares compartidas por  *
*           todos los procesos del sistema de reserva.            *
* Descripción:                                                    *
*    - Implementa funciones para imprimir mensajes de error e     *
*      información.                                               *
*    - Valida horas individuales y rangos de horas de reserva.    *
******************************************************************/

// Incluye las definiciones 
#include "Sistema.h"

// Imprime el mensaje de error en la salida
void imprimirMensajeError(const char* mensaje) {
    // Dentro de stderr se escribe para saber el error
    fprintf(stderr, "[ERROR] %s\n", mensaje);
}

// Mensaje con información
void imprimirMensajeInfo(const char* mensaje) {
    printf("[INFO] %s\n", mensaje);
}


int validarHora(int hora) {
    // Comprubea uqe hora sea mayor o igual a las mínimas y máximas
    return (hora >= HORA_MIN && hora <= HORA_MAX);
}


int validarRangoHoras(int horaIni, int horaFin) {
    // Verifica que ambas horas son válidas
    if (!validarHora(horaIni) || !validarHora(horaFin)) {
        return 0;
    }
    // Verifica que la hora de inicio no sea mayor a la final
    if (horaIni > horaFin) {
        return 0; // Rango inválido
    }
    return 1;// Rango válido
}
