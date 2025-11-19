ro# Proyecto – Sistema de Reservas Parque Berlín

**Autoras:**  
Danna Gabriela Rojas Bernal  
María Fernanda Velandia Gracia

**Materia:** Sistemas Operativos  
**Tema:** Sincronización, procesos, hilos y comunicación con Pipes en C

---

## Propósito de este proyecto

Este repositorio contiene el código fuente y documentación del proyecto **Sistema de Reservas del Parque Berlín**, desarrollado en lenguaje C para demostrar el uso de:

- Procesos POSIX  
- Hilos pthread  
- Pipes con comunicación entre procesos  
- Sincronización mediante mutex  
- Simulación de aforo y gestión de solicitudes

El programa administra solicitudes de familias que desean ingresar al parque, verificando disponibilidad por hora y aforo. Dependiendo de la capacidad existente, una solicitud puede:

- Ser aceptada  
- Reprogramarse automáticamente  
- Ser rechazada  

Al finalizar la jornada, el sistema genera estadísticas generales del uso del parque.

---

## Modo de uso

Para ejecutar el programa se requieren dos terminales abiertas en la carpeta del proyecto.

### Terminal 1 – Controlador

1. make controlador
2. ./controlador -i horaIni -f horaFin -s segHora -t total -p pipeRecibe

---

### Terminal 2 – Agente

1. make agente
2. ./agente -s nombreAgente -a archivo.csv -p pipeRecibe
   
---

## Archivos de solicitudes

El archivo debe:

- Tener formato `.csv`
- Contener las columnas: familia,hora,numPersonas
  Ejemplo:
  López,9,4
  Gómez,11,3

- El repositorio incluye **6 archivos de prueba** con diferentes combinaciones de reserva. Dentro de este repositorio estan en la carpeta **Pruebas** pero para ejecutar no deben estar dentro de dicha carpeta, deben estar al igual que el Makefile.

---

## Anotaciones importantes

Si aparece un mensaje indicando que el ejecutable ya existe o debe recompilarse, usar: make clean
