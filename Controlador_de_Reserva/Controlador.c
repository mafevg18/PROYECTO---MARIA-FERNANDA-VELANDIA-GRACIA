/******************************************************************
*            Pontificia Universidad Javeriana                     *
*                         ---=---                                 *
*                                                                 *
* Autor: Danna Gabriela Rojas & María Fernanda Velandia           *
* Materia: Sistemas Operativos                                    *
* Docente: J. Corredor                                            *
* Fecha: 18 Noviembre del 2025                                    *
* Tema: Proyecto Parque Berlín                                    *
* Archivo: Controlador.c                                          *
* Objetivo: Implementar la lógica principal del proceso           *
*           Controlador, encargado de gestionar reservas,         *
*           manejar el reloj simulado, controlar el aforo,        *
*           procesar solicitudes de los agentes y generar         *
*           estadísticas finales del día.                         *
* Descripción:                                                    *
*    - Contiene funciones para inicializar, ejecutar y finalizar  *
*      el controlador.                                            *
*    - Gestiona hilos para simular el reloj y procesar pipes.     *
*    - Procesa solicitudes de registro y reserva desde agentes.    *
*    - Controla el aforo del parque por hora.                     *
*    - Al finalizar genera un informe con estadísticas del día.   *
******************************************************************/

// Incluye las declaraciones
#include "Controlador.h"
// Variables globales
static int horaActual; // Hora actual de la simulación
static int horaInicio; // Hora de apertura
static int horaFin; // Hora de cierre
static int segHoras; // Segundos simulan horas
static int aforoMaximo; // Capacidad máxima por hora
static char pipeRecibe[MAX_NOMBRE]; // Nombre donde llegan las solicitudes
static Estadisticas stats; // Estadisticas del día
static Reserva reservas[MAX_RESERVAS]; // Reservas realizadas
static int numReservas = 0; // Cantidad de reservas
static pthread_mutex_t mutexReservas = PTHREAD_MUTEX_INITIALIZER; // Mutex que protege el acceso concurrente de reservas
static pthread_mutex_t mutexHora = PTHREAD_MUTEX_INITIALIZER; // Mutex que protege la hora actual
static volatile int debeTerminar = 0; // Flag para que el sistema termine

int inicializarControlador(ParametrosControlador* params) {
    // Valida que las horas esten en el rango
    if (!validarRangoHoras(params->horaInicio, params->horaFin)) {
        imprimirMensajeError("Horas fuera del rango válido (7-19)");
        return 0;
    }
    // Asigna los parámetros recibidos del controlador
    horaInicio = params->horaInicio;
    horaFin = params->horaFin;
    horaActual = horaInicio;
    segHoras = params->segHoras;
    aforoMaximo = params->aforoMaximo;
    strcpy(pipeRecibe, params->pipeRecibe);
    // Inicializa en 0 estadistica
    memset(&stats, 0, sizeof(Estadisticas));
    // Mensaje con información inicial
    printf("=== Controlador de Reserva Iniciado ===\n");
    printf("Hora inicial: %d:00\n", horaActual);
    printf("Hora final: %d:00\n", horaFin);
    printf("Aforo máximo: %d personas\n", aforoMaximo);
    printf("Segundos por hora: %d\n", segHoras);
    printf("======================================\n\n");
    // Revisa que no exista un pipe antes con el mismo nombre
    unlink(pipeRecibe);
    // Se crea el pipe con sus respectivos permisos
    if (mkfifo(pipeRecibe, 0666) == -1) {
        perror("Error al crear pipe");
        return 0; // Falló la creación
    }
    // Señales para limpiar
    signal(SIGINT, manejarSenalFin);
    signal(SIGTERM, manejarSenalFin);
    
    return 1; // Inicio con éxito
}

void ejecutarControlador() {
    // Identificador de los hilos
    pthread_t tidReloj, tidPeticiones;
    // Crea el hilo del reloj
    if (pthread_create(&tidReloj, NULL, threadReloj, NULL) != 0) {
        perror("Error al crear hilo de reloj"); 
        exit(EXIT_FAILURE); // Error creando
    }
    // Crea el hilo de las peticiones de agente
    if (pthread_create(&tidPeticiones, NULL, threadPeticiones, NULL) != 0) {
        perror("Error al crear hilo de peticiones");
        exit(EXIT_FAILURE);// Error creando
    }
    // Esperar que el reloj termine
    pthread_join(tidReloj, NULL);
    // Tiempo de espera
    sleep(2);
    // El hilo peticiones debe terminar
    debeTerminar = 1;
    // El hilo peticiones se limpia
    pthread_join(tidPeticiones, NULL);
}

void finalizarControlador() {
    //Imprime el reporte
    imprimirReporte();
    // ELimina el pipe
    unlink(pipeRecibe);
    // Se destuyen los mutex
    pthread_mutex_destroy(&mutexReservas);
    pthread_mutex_destroy(&mutexHora);
    
    printf("\n=== Controlador de Reserva Finalizado ===\n");
}
// Hilo del reloj de simulación
void* threadReloj(void* arg) {
    (void)arg;
    // Avanza la hora hasta la de cierre
    while (1) {
        pthread_mutex_lock(&mutexHora);
        if (horaActual > horaFin) {
            // Si es superior se desbloquea y sale
            pthread_mutex_unlock(&mutexHora);
            break;
        }
        pthread_mutex_unlock(&mutexHora);
        // Espera los segundos de simulación
        sleep(segHoras);
        avanzarHora();
    }
    return NULL;
}

void* threadPeticiones(void* arg) {
    (void)arg;
    // Abre en lectura
    int fdLectura;
    Solicitud sol;
    // Se abre el pipe en lectura NO_BLOCK
    while ((fdLectura = open(pipeRecibe, O_RDONLY | O_NONBLOCK)) == -1) {
        if (errno != ENXIO && errno != EINTR) {
            perror("Error al abrir pipe de recepción");
            pthread_exit(NULL); //Se corta el hilo
        }
        usleep(100000);
        //Se sale del hilo
        if (debeTerminar) pthread_exit(NULL);
    }
    // Modo bloqueado para lecturas
    int flags = fcntl(fdLectura, F_GETFL);
    fcntl(fdLectura, F_SETFL, flags & ~O_NONBLOCK);
    while (!debeTerminar) {
    	// Lectura de solicitud
        ssize_t bytesLeidos = read(fdLectura, &sol, sizeof(Solicitud));
        // Solicitud completa
        if (bytesLeidos == sizeof(Solicitud)) {
            // Se abre el pipe que dio el agente
            int fdEscritura = open(sol.pipeRespuesta, O_WRONLY);
            if (fdEscritura == -1) {
                perror("Error al abrir pipe de respuesta");
                continue;
            }
            // Si es registro, se procesa
            if (sol.tipo == MSG_REGISTRO) {
                procesarRegistro(&sol, fdEscritura);
            // Si es solicitud de reserva, se procesa
            } else if (sol.tipo == MSG_SOLICITUD) {
                procesarSolicitud(&sol, fdEscritura);
            }
            // Cierra el pipe
            close(fdEscritura);
        // Error de lectura
        } else if (bytesLeidos == -1) {
            if (errno == EINTR) continue;
            if (debeTerminar) break;
        // NO hay datos
        } else if (bytesLeidos == 0) {
            usleep(100000);
        }
    }
    // Cierra el pipe
    close(fdLectura);
    return NULL;
}

void procesarRegistro(Solicitud* sol, int fdEscritura) {
    // Mensaje de consola
    printf("[REGISTRO] Agente '%s' conectado\n", sol->nombreAgente);
    
    Respuesta resp;
    // Se obtiene la hora actual
    pthread_mutex_lock(&mutexHora);
    resp.horaActual = horaActual;
    pthread_mutex_unlock(&mutexHora);
    // Configuración del mensaje
    resp.tipo = RESP_OK;
    resp.horaAsignada = 0;
    strcpy(resp.mensaje, "Registro exitoso");
    // Se envía la respuesta al agente
    if (write(fdEscritura, &resp, sizeof(Respuesta)) == -1) {
        perror("Error al enviar respuesta de registro");
    }
}

void procesarSolicitud(Solicitud* sol, int fdEscritura) {
    Respuesta resp;
    // Se obtiene la hora simulada
    pthread_mutex_lock(&mutexHora);
    int hora = horaActual;
    pthread_mutex_unlock(&mutexHora);
    // Mensaje de consola
    printf("[SOLICITUD] Agente: %s | Familia: %s | Hora: %d:00 | Personas: %d\n",
           sol->nombreAgente, sol->nombreFamilia, sol->hora, sol->numPersonas);
    // Protección a reservas
    pthread_mutex_lock(&mutexReservas);
    // Solicitud superior al aforo
    if (sol->numPersonas > aforoMaximo) {
        resp.tipo = RESP_NEGADA_VOLVER;
        resp.horaAsignada = 0;
        sprintf(resp.mensaje, "Reserva negada: Número de personas (%d) supera aforo máximo (%d). Vuelva otro día", 
                sol->numPersonas, aforoMaximo);
        stats.solicitudesNegadas++;
    }
    // Solicitud con hora sobrepasada
    else if (sol->hora < hora) {
        int horaDisponible;
        // Busca espacio disponible
        if (buscarBloqueDisponible(sol->numPersonas, &horaDisponible)) {
            resp.tipo = RESP_NEGADA_EXTEMPORANEA;
            resp.horaAsignada = horaDisponible;
            sprintf(resp.mensaje, "Hora extemporánea. Reprogramada para %d:00", horaDisponible);
            // Se registra la reserva reprogramada
            strcpy(reservas[numReservas].nombreFamilia, sol->nombreFamilia);
            strcpy(reservas[numReservas].nombreAgente, sol->nombreAgente);
            reservas[numReservas].numPersonas = sol->numPersonas;
            reservas[numReservas].horaInicio = horaDisponible;
            reservas[numReservas].horaFin = horaDisponible + DURACION_RESERVA;
            numReservas++;
            
            stats.solicitudesReprogramadas++;
        // No hay disponibilidad
        } else {
            resp.tipo = RESP_NEGADA_VOLVER;
            resp.horaAsignada = 0;
            strcpy(resp.mensaje, "Reserva negada: No hay disponibilidad");
            stats.solicitudesNegadas++;
        }
    }
    // Solicitud fuera del rango
    else if (sol->hora > horaFin) {
        resp.tipo = RESP_NEGADA_VOLVER;
        resp.horaAsignada = 0;
        strcpy(resp.mensaje, "Reserva negada: Hora fuera del período de simulación");
        stats.solicitudesNegadas++;
    }
    // Solicitud con hora correcta pero validar aforo
    else {
        int disponible = 1;
        int horaFinReserva = sol->hora + DURACION_RESERVA;
        // Ajusta los limites
        if (horaFinReserva > horaFin + 1) {
            horaFinReserva = horaFin + 1;
        }
        // Verificación de aforo
        for (int h = sol->hora; h < horaFinReserva; h++) {
            if (calcularPersonasEnHora(h) + sol->numPersonas > aforoMaximo) {
                disponible = 0;
                break;
            }
        }
        // Reserva OK
        if (disponible) {
            resp.tipo = RESP_OK;
            resp.horaAsignada = sol->hora;
            sprintf(resp.mensaje, "Reserva aprobada para %d:00", sol->hora);
            // Se guarda la reserva
            strcpy(reservas[numReservas].nombreFamilia, sol->nombreFamilia);
            strcpy(reservas[numReservas].nombreAgente, sol->nombreAgente);
            reservas[numReservas].numPersonas = sol->numPersonas;
            reservas[numReservas].horaInicio = sol->hora;
            reservas[numReservas].horaFin = sol->hora + DURACION_RESERVA;
            numReservas++;
            
            stats.solicitudesAceptadas++;
            // Buscar reprogramación
        } else {
            int horaDisponible;
            if (buscarBloqueDisponible(sol->numPersonas, &horaDisponible)) {
                resp.tipo = RESP_REPROGRAMADA;
                resp.horaAsignada = horaDisponible;
                sprintf(resp.mensaje, "Reprogramada para %d:00", horaDisponible);
                
                strcpy(reservas[numReservas].nombreFamilia, sol->nombreFamilia);
                strcpy(reservas[numReservas].nombreAgente, sol->nombreAgente);
                reservas[numReservas].numPersonas = sol->numPersonas;
                reservas[numReservas].horaInicio = horaDisponible;
                reservas[numReservas].horaFin = horaDisponible + DURACION_RESERVA;
                numReservas++;
                
                stats.solicitudesReprogramadas++;
                // NO disponible ningún horario
            } else {
                resp.tipo = RESP_NEGADA_VOLVER;
                resp.horaAsignada = 0;
                strcpy(resp.mensaje, "Reserva negada: No hay disponibilidad");
                stats.solicitudesNegadas++;
            }
        }
    }
    
    pthread_mutex_unlock(&mutexReservas);
    // Envío al agente
    if (write(fdEscritura, &resp, sizeof(Respuesta)) == -1) {
        perror("Error al enviar respuesta de solicitud");
    }
}

int calcularPersonasEnHora(int hora) {
    int total = 0;
    // Revisa cada reserva
    for (int i = 0; i < numReservas; i++) {
    	// Hora dentro del intervalo
        if (hora >= reservas[i].horaInicio && hora < reservas[i].horaFin) {
            total += reservas[i].numPersonas;
        }
    }
    return total;
}

int buscarBloqueDisponible(int numPersonas, int* horaEncontrada) {
    // Obtiene hora actual protegida
    pthread_mutex_lock(&mutexHora);
    int hora = horaActual;
    pthread_mutex_unlock(&mutexHora);
    // Hora actual a cierre del parque
    for (int h = hora; h <= horaFin - DURACION_RESERVA + 1; h++) {
        int disponible = 1;
        for (int i = h; i < h + DURACION_RESERVA; i++) {
            // Si el aforo es mayor no se puede
            if (calcularPersonasEnHora(i) + numPersonas > aforoMaximo) {
                disponible = 0;
                break;
            }
        }
        // Se encontro un bloque
        if (disponible) {
            *horaEncontrada = h;
            return 1;
        }
    }
    return 0; // NO hay horario
}

void avanzarHora() {
    // Se bloquean las variables compartidas
    pthread_mutex_lock(&mutexHora);
    pthread_mutex_lock(&mutexReservas);
    // Avanza la hora en el parque
    horaActual++;
    
    printf("\n========== HORA: %d:00 ==========\n", horaActual);
    
    printf("SALEN DEL PARQUE:\n");
    int totalSalen = 0;
    // Se recorren todas las reservas
    for (int i = 0; i < numReservas; i++) {
        if (reservas[i].horaFin == horaActual) {
        // Se imprime la información de salida
            printf("  - Familia %s (%d personas)\n", 
                   reservas[i].nombreFamilia, reservas[i].numPersonas);
            // Total personas salen
            totalSalen += reservas[i].numPersonas;
        }
    }
    if (totalSalen == 0) printf("  (ninguna)\n");
    
    printf("ENTRAN AL PARQUE:\n");
    int totalEntran = 0;
    // Se recorren todas las reservas
    for (int i = 0; i < numReservas; i++) {
        if (reservas[i].horaInicio == horaActual) {
            printf("  - Familia %s (%d personas)\n", 
                   reservas[i].nombreFamilia, reservas[i].numPersonas);
            totalEntran += reservas[i].numPersonas;
        }
    }
    // Si ninguna persona entra
    if (totalEntran == 0) printf("  (ninguna)\n");
    // Total personas en el parque
    int personasActuales = calcularPersonasEnHora(horaActual);
    printf("Total personas en parque: %d / %d\n", personasActuales, aforoMaximo);
    printf("====================================\n\n");
    
    int indice = horaActual - HORA_MIN;
    if (indice >= 0 && indice <= HORAS_DIA) {
        stats.personasPorHora[indice] = personasActuales;
    }
    // Se liberan los recursos protegidos
    pthread_mutex_unlock(&mutexReservas);
    pthread_mutex_unlock(&mutexHora);
}

void imprimirReporte() {
    printf("\n\n");
    printf("============================================\n");
    printf("         REPORTE FINAL DEL DÍA\n");
    printf("============================================\n\n");

    // Encontrar máximos y mínimos durante el día
    int maxPersonas = 0, minPersonas = aforoMaximo + 1;

    for (int i = 0; i <= horaFin - HORA_MIN; i++) {
    	// Actualiza el máximo
    	if (stats.personasPorHora[i] > maxPersonas) {
        	maxPersonas = stats.personasPorHora[i];
    	}
    // Actualiza el nímo y se ignoran los 0
    if (stats.personasPorHora[i] < minPersonas && stats.personasPorHora[i] > 0) {
        minPersonas = stats.personasPorHora[i];
    }
}
    printf("HORAS PICO (mayor ocupación):\n");
    for (int i = 0; i <= horaFin - HORA_MIN; i++) {
        if (stats.personasPorHora[i] == maxPersonas) {
            printf("  - %d:00 (%d personas)\n", i + HORA_MIN, maxPersonas);
        }
    }

    printf("\nHORAS VALLE (menor ocupación):\n");
    for (int i = 0; i <= horaFin - HORA_MIN; i++) {
        if (stats.personasPorHora[i] == minPersonas) {
            printf("  - %d:00 (%d personas)\n", i + HORA_MIN, minPersonas);
        }
    }

    printf("\nESTADÍSTICAS DE SOLICITUDES:\n");
    printf("  Solicitudes aceptadas:      %d\n", stats.solicitudesAceptadas);
    printf("  Solicitudes reprogramadas:  %d\n", stats.solicitudesReprogramadas);
    printf("  Solicitudes negadas:        %d\n", stats.solicitudesNegadas);
    printf("  Total solicitudes:          %d\n",
           stats.solicitudesAceptadas + stats.solicitudesReprogramadas + stats.solicitudesNegadas);
    printf("\n============================================\n");
}


void manejarSenalFin(int sig) {
    (void)sig;
    printf("\n[SEÑAL] Finalizando controlador...\n");
    debeTerminar = 1;
}
