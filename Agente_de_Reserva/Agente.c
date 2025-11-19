/******************************************************************
*            Pontificia Universidad Javeriana                     *
*                         ---=---                                 *
*                                                                 *
* Autor: Danna Gabriela Rojas & María Fernanda Velandia           *
* Materia: Sistemas Operativos                                    *
* Docente: J. Corredor                                            *
* Fecha: 18 Noviembre del 2025                                    *
* Tema: Proyecto Parque Berlín                                    *
* Archivo: Agente.c                                               *
* Objetivo: Implementar el proceso agente encargado de enviar     *
*           solicitudes de reserva al controlador mediante pipes. *
* Descripción:                                                    *
*    - Inicializa el agente y crea su pipe propio.                *
*    - Lee solicitudes desde archivo CSV.                         *
*    - Se registra ante el controlador y espera respuestas.       *
*    - Envía solicitudes y procesa las respuestas recibidas.      *
******************************************************************/

#include "Agente.h" // Incluye las definiciones y estructuras del agente
//Librerías de entradas y salidas, memoria y cadenas
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// Librerías para manejo de procesos, pipes y errores
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

//Variables globales
static char nombreAgente[MAX_NOMBRE]; // Nombre del proceso agente
static char archivoSolicitudes[MAX_NOMBRE]; // Ruta del archivo .csv
static char pipeEnvio[MAX_NOMBRE]; // Nombre del pipe para enviar mensajes al controlador
static char pipeRecepcion[MAX_NOMBRE]; // Nombre del pipe por donde el agente recibe respuesta
static int horaActualSimulacion = 0; // Hora actual

// Máximo para el nombre del agente
#define MAX_NAME_PART (MAX_NOMBRE - 20) 


// Imprime el error 
void imprimirMensajeError(const char* mensaje);
// Lee el archivo .csv extrayendo la familia, hora y cantidad de personas
int leerLineaCSV(FILE* archivo, char* familia, int* hora, int* personas);
// Envíar solicitud al controlador
void enviarSolicitud(char* familia, int hora, int personas);
// Imprime la respuesta que se recibio
void imprimirRespuesta(Respuesta* resp, const char* familia);
// Registra el ante frente al controlador
int registrarseConControlador();
// Procesa las solicitudes del archivo
void procesarSolicitudes();

// Se inicializa el agente
int inicializarAgente(ParametrosAgente* params) {
    // Copia el nombre del agente desde parámetros
    strncpy(nombreAgente, params->nombreAgente, MAX_NOMBRE - 1);
    nombreAgente[MAX_NOMBRE - 1] = '\0';
    //Copia el archivo de solicitudes
    strncpy(archivoSolicitudes, params->archivoSolicitudes, MAX_NOMBRE - 1);
    archivoSolicitudes[MAX_NOMBRE - 1] = '\0';
    // Copia el nombre del pipe por donde se envían las solicitudes
    strncpy(pipeEnvio, params->pipeEnvio, MAX_NOMBRE - 1);
    pipeEnvio[MAX_NOMBRE - 1] = '\0';
    // Construye un pipe único
    snprintf(pipeRecepcion, MAX_NOMBRE, "/tmp/pipe_%.*s_%d", 
             MAX_NAME_PART, nombreAgente, getpid());
    // Se muestra mensaje de inicio en el agente
    printf("[AGENTE %s] Iniciado\n", nombreAgente);
    // Se elimina si antes ya existía un pipe
    unlink(pipeRecepcion);
    // Se crea el pipe para recibir los mensajes
    if (mkfifo(pipeRecepcion, 0666) == -1) {
        perror("Error al crear pipe de recepción");// Mensaje de error
        return 0; // Falló la creación del pipe
    }
    
    return 1; //Se inicializó correctamente
}

// Ejecución del agente
void ejecutarAgente() {
    // Intenta registrarse con el controlador
    if (!registrarseConControlador()) {
        imprimirMensajeError("No se pudo registrar con el controlador");
        return;
    }
    // Procesa todas las solicitudes del archivo
    procesarSolicitudes();
}

// Terminar la ejecución del agente
void finalizarAgente() {
    printf("[AGENTE %s] Termina\n", nombreAgente);
    unlink(pipeRecepcion);// ELimina el pipe de si mismo
}

// Registro frente al controlador
int registrarseConControlador() {
    // Descripción de escritura
    int fdEnvio = -1;
    // Cantidad de intento para conectar con el pipe
    int intentos = 0;
    // Abriri el pipe del controlador en modo NO BLOCK
    while (fdEnvio == -1 && intentos < 10) {
        fdEnvio = open(pipeEnvio, O_WRONLY | O_NONBLOCK);
        if (fdEnvio == -1) {
            if (errno == ENXIO) {

                usleep(500000); // Tiempo de esperar para intentar nuevamente
                intentos++;
            } else {
            	// Errores que se pueden tener
                perror("Error al abrir pipe de envío");
                return 0;
            }
        }
    }
    // Se cancela el registro
    if (fdEnvio == -1) {
        imprimirMensajeError("No se pudo conectar con el controlador después de varios intentos");
        return 0;
    }
    // Desactiva el O_NONBLOCK para escribir
    int flags = fcntl(fdEnvio, F_GETFL);
    fcntl(fdEnvio, F_SETFL, flags & ~O_NONBLOCK);
    // Mensaje de registro
    Solicitud sol;
    memset(&sol, 0, sizeof(Solicitud));
    sol.tipo = MSG_REGISTRO;
    strncpy(sol.nombreAgente, nombreAgente, MAX_NOMBRE - 1);
    sol.nombreAgente[MAX_NOMBRE - 1] = '\0';
    strncpy(sol.pipeRespuesta, pipeRecepcion, MAX_NOMBRE - 1);
    sol.pipeRespuesta[MAX_NOMBRE - 1] = '\0';
    sol.hora = 0;
    sol.numPersonas = 0;
    sol.nombreFamilia[0] = '\0';
    // Se escribe el mensaje de registro al pipe del controlador
    if (write(fdEnvio, &sol, sizeof(Solicitud)) == -1) {
        perror("Error al enviar registro");
        close(fdEnvio);
        return 0;
    }
    
    close(fdEnvio);// Se cierra la escritura
    // Abre pipe de respuesta
    int fdRecepcion = open(pipeRecepcion, O_RDONLY);
    if (fdRecepcion == -1) {
        perror("Error al abrir pipe de recepción");
        return 0;
    }
    // Lee las respuestas de registro
    Respuesta resp;
    ssize_t bytesLeidos = read(fdRecepcion, &resp, sizeof(Respuesta));
    if (bytesLeidos == sizeof(Respuesta)) {
    	// Actualizar hora de simulación
        horaActualSimulacion = resp.horaActual;
        printf("[AGENTE %s] Registrado. Hora actual del sistema: %d:00\n", 
               nombreAgente, horaActualSimulacion);
    } else {
        close(fdRecepcion);
        return 0; //Registro fallido
    }
    
    close(fdRecepcion);
    return 1; // Registro con éxito
}

// Procesar archivo de solicitudes
void procesarSolicitudes() {
    // Abre el archivo
    FILE* archivo = fopen(archivoSolicitudes, "r");
    if (archivo == NULL) {
        perror("Error al abrir archivo de solicitudes");// Mensaje de error
        return;
    }
    char familia[MAX_NOMBRE];
    int hora, personas;
    // Leer línea por línea y generar solicitud
    while (leerLineaCSV(archivo, familia, &hora, &personas)) {
        // Envía la solicitud al controlador
        enviarSolicitud(familia, hora, personas);
        // TIempo entre solicitudes
        sleep(2);
    }
    fclose(archivo);
}

// Enviar solicitud al controlador
void enviarSolicitud(char* familia, int hora, int personas) {
    // Estado de la solicitud
    printf("[AGENTE %s] Enviando solicitud: Familia %s | Hora %d:00 | Personas %d\n",
           nombreAgente, familia, hora, personas);
    // Apertura pipe hacia el controlador
    int fdEnvio = open(pipeEnvio, O_WRONLY);
    if (fdEnvio == -1) {
        perror("Error al abrir pipe de envío");
        return;
    }
    // Construye la solicitud
    Solicitud sol;
    memset(&sol, 0, sizeof(Solicitud));
    sol.tipo = MSG_SOLICITUD;
    strncpy(sol.nombreAgente, nombreAgente, MAX_NOMBRE - 1);
    sol.nombreAgente[MAX_NOMBRE - 1] = '\0';
    strncpy(sol.nombreFamilia, familia, MAX_NOMBRE - 1);
    sol.nombreFamilia[MAX_NOMBRE - 1] = '\0';
    strncpy(sol.pipeRespuesta, pipeRecepcion, MAX_NOMBRE - 1);
    sol.pipeRespuesta[MAX_NOMBRE - 1] = '\0';
    sol.hora = hora;
    sol.numPersonas = personas;
    // Escritura al pipe controlador
    if (write(fdEnvio, &sol, sizeof(Solicitud)) == -1) {
        perror("Error al enviar solicitud");
        close(fdEnvio);
        return;
    }
    close(fdEnvio);
    // Abrir pipe para leer la respuesta
    int fdRecepcion = open(pipeRecepcion, O_RDONLY);
    if (fdRecepcion == -1) {
        perror("Error al abrir pipe de recepción");
        return;
    }
    // Leer respuesta
    Respuesta resp;
    ssize_t bytesLeidos = read(fdRecepcion, &resp, sizeof(Respuesta));
    if (bytesLeidos == sizeof(Respuesta)) {
    	// Imprimir el resultado
        imprimirRespuesta(&resp, familia);
    } else {
        printf("[AGENTE %s] Error: No se recibió respuesta completa\n", nombreAgente);
    }
    close(fdRecepcion);
}
// Lectura de línea del archivo
int leerLineaCSV(FILE* archivo, char* familia, int* hora, int* personas) {
    // Buffer para línea
    char linea[MAX_BUFFER]; 
    // Leer línea a línea
    while (fgets(linea, sizeof(linea), archivo) != NULL) {
	// Ignorar vacíos o comentarios
        if (linea[0] == '\n' || linea[0] == '#') {
            continue;
        }
        // Extraer los valores
        if (sscanf(linea, "%99[^,],%d,%d", familia, hora, personas) == 3) {
            return 1; // Linea con éxito
        }
    }
    return 0; 
}

// Imprimir respuesta obtenida
void imprimirRespuesta(Respuesta* resp, const char* familia) {
    printf("[AGENTE %s] RESPUESTA para familia %s: %s\n", 
           nombreAgente, familia, resp->mensaje);
    // Según el tipo de respuesta
    switch (resp->tipo) {
        case RESP_OK:
            printf("Reserva confirmada para %d:00\n", resp->horaAsignada);
            break;
        case RESP_REPROGRAMADA:
            printf("Reserva reprogramada para %d:00\n", resp->horaAsignada);
            break;
        case RESP_NEGADA_EXTEMPORANEA:
            printf("Hora extemporánea, reprogramada para %d:00\n", resp->horaAsignada);
            break;
        case RESP_NEGADA_VOLVER:
            printf("Reserva negada\n");
            break;
        default:
            printf("Respuesta no catalagoda\n");
            break;
    }
}
