# Donkey Kong Clásico Concurrente

## Universidad del Valle de Guatemala

### Facultad de Ingeniería

### CC3086 – Programación de Microprocesadores

---

# Integrantes

* Ricardo Escobar
* Miguel Sajquín
* José Ceferino

---

# Descripción General

Este proyecto consiste en una implementación concurrente del juego clásico **Donkey Kong** utilizando **C++17**, **POSIX Threads (Pthreads)** y **ncurses** para la representación gráfica en consola mediante ASCII-Art.

El objetivo principal del jugador es controlar a Mario para ascender por una estructura de plataformas y escaleras mientras evita barriles lanzados por Donkey Kong. El juego finaliza cuando el jugador alcanza la meta en la parte superior del escenario o pierde todas sus vidas.

Además de la funcionalidad del juego, el proyecto fue diseñado para demostrar conceptos de:

* Programación concurrente.
* Sincronización entre hilos.
* Exclusión mutua.
* Comunicación entre hilos.
* Sistemas multinúcleo.
* Prevención de condiciones de carrera.
* Coordinación temporal mediante eventos sincronizados.

---

# Arquitectura Concurrente

La aplicación se encuentra organizada mediante múltiples hilos especializados, cada uno con una responsabilidad claramente definida.

## GameClockThread

Responsable de generar los "ticks" globales del sistema.

Este hilo actúa como reloj central de la simulación y despierta periódicamente a los demás hilos mediante una variable de condición.

Funciones:

* Generar eventos temporales.
* Coordinar la evolución del estado del juego.
* Reducir el uso de espera activa (busy waiting).

---

## InputThread

Responsable de capturar la entrada del usuario.

Funciones:

* Lectura de teclado.
* Actualización de eventos de entrada.
* Comunicación con otros hilos mediante variables compartidas sincronizadas.

---

## PlayerThread

Representa al jugador (Mario).

Funciones:

* Movimiento horizontal.
* Salto.
* Gravedad.
* Subida y bajada de escaleras.
* Gestión de vidas.
* Gestión de puntuación.
* Recolección de martillo.
* Verificación de victoria.

---

## DonkeyKongThread

Representa a Donkey Kong como entidad independiente.

Funciones:

* Generación periódica de barriles.
* Control de aparición de obstáculos.
* Comunicación con otros componentes mediante semáforos y eventos.

---

## BarrelMovementThread

Gestiona todos los barriles activos del escenario.

Funciones:

* Movimiento de barriles.
* Colisiones con el jugador.
* Detección de barriles destruidos.
* Registro de barriles saltados.
* Eliminación de barriles inactivos.

---

## RenderThread

Responsable de la representación gráfica del estado del juego.

Funciones:

* Dibujado del mapa.
* Dibujado del jugador.
* Dibujado de Donkey Kong.
* Dibujado de barriles.
* Actualización de HUD.
* Visualización de eventos.

Para evitar condiciones de carrera, el hilo trabaja sobre copias sincronizadas de los datos compartidos.

---

# Modelo de Sincronización

El proyecto utiliza múltiples mecanismos de sincronización para coordinar la ejecución concurrente.

---

## Mutexes

### playerMutex

Protege:

* Posición del jugador.
* Vidas.
* Puntuación.
* Estado de salto.

---

### barrelMutex

Protege:

* Vector de barriles.
* Inserción de barriles.
* Eliminación de barriles.
* Movimiento de barriles.

---

### eventMutex

Protege:

* Mensajes de eventos mostrados en pantalla.

---

### inputMutex

Protege:

* Eventos relacionados con la entrada de usuario.

---

# Variables de Condición

## tickCond

Utilizada para sincronizar la evolución temporal de la simulación.

Los hilos de lógica permanecen bloqueados hasta recibir un tick generado por el GameClockThread.

Beneficios:

* Reducción de espera activa.
* Sincronización temporal consistente.
* Menor consumo de CPU.

---

## inputCond

Utilizada para coordinar eventos de entrada entre los hilos responsables del procesamiento del teclado y la lógica del juego.

---

# Semáforos

## barrelSemaphore

Utilizado para sincronizar la producción de barriles generados por Donkey Kong.

Permite representar el patrón Productor–Consumidor:

DonkeyKongThread → Produce barriles

BarrelMovementThread → Consume barriles

---

# Barreras

## frameBarrier

Se utiliza para garantizar que los hilos principales se encuentren completamente inicializados antes de iniciar la simulación.

Participantes:

* PlayerThread
* RenderThread
* DonkeyKongThread
* BarrelMovementThread

La ejecución del juego comienza únicamente cuando todos los componentes críticos se encuentran listos.

---

# Variables Atómicas

Se utilizan variables atómicas para garantizar consistencia de memoria entre núcleos y evitar condiciones de carrera en estados simples compartidos.

Variables protegidas mediante std::atomic:

* running
* gameOver
* lastKey
* hammerActive
* hammerCollected
* hammerTimer
* levelGame
* barrelSpeed

---

# Entidades del Juego

## Mario

* Movimiento horizontal.
* Salto.
* Escaleras.
* Sistema de vidas.
* Sistema de puntuación.

---

## Donkey Kong

* Generación de barriles.
* Gestión de eventos.

---

## Barriles

* Movimiento independiente del jugador.
* Colisiones.
* Destrucción mediante martillo.
* Registro de barriles saltados.

---

## Martillo

Permite destruir barriles durante un tiempo limitado.

Características:

* Aparición en el mapa.
* Recolección por el jugador.
* Temporizador de duración.
* Bonificación por destrucción de barriles.

---

# Sistema de Eventos

El juego registra eventos importantes:

* Barril destruido.
* Barril saltado.
* Martillo obtenido.
* Pérdida de vida.
* Alcance de la meta.

Estos eventos son sincronizados mediante eventMutex para evitar inconsistencias entre hilos.

---

# Detección de Colisiones

El sistema verifica:

* Jugador contra barriles.
* Jugador contra plataformas.
* Jugador contra escaleras.
* Jugador contra martillo.
* Jugador contra meta.

Las colisiones son procesadas concurrentemente manteniendo consistencia mediante mutexes.

---

# Finalización del Juego

El juego finaliza cuando:

* El jugador pierde todas sus vidas.
* El jugador alcanza la parte superior del escenario.

Al finalizar:

1. Se muestra el resultado.
2. Se registra la puntuación.
3. Se solicita el nombre del jugador.
4. Se almacena el resultado en el sistema de puntajes.
5. Se retorna al menú principal.

---

# Prevención de Problemas Concurrentes

Durante el desarrollo se implementaron mecanismos para prevenir:

## Condiciones de carrera

Mediante:

* Mutexes.
* Variables atómicas.

---

## Inconsistencias de memoria

Mediante:

* std::atomic.
* Sincronización explícita.

---

## Espera activa excesiva

Mediante:

* Variables de condición.
* Tick global sincronizado.

---

## Accesos concurrentes no controlados

Mediante:

* Protección específica para cada recurso compartido.

---

# Pruebas Realizadas

Se realizaron pruebas de validación para verificar:

* Movimiento concurrente de barriles.
* Captura de entrada simultánea.
* Correcto funcionamiento de sincronización.
* Consistencia de puntuación.
* Recolección de martillo.
* Detección de colisiones.
* Finalización correcta de partidas.
* Funcionamiento en ejecuciones repetidas.

---

# Tecnologías Utilizadas

## Lenguaje

* C++17

## Bibliotecas

* pthread.h
* semaphore.h
* ncurses.h
* atomic
* vector
* algorithm
* fstream
* ctime
* unistd.h

---

# Compilación (con AlmaLinux-10)

```bash
g++ main.cpp menu.cpp game.cpp render.cpp input.cpp -o donkeykong -lncurses -lpthread
```

---

# Ejecución

```bash
./donkeykong
```

---

# Conclusión

La implementación final utiliza múltiples mecanismos de programación concurrente y sincronización para coordinar la ejecución paralela de las entidades principales del juego. La arquitectura separa claramente las responsabilidades de entrada, simulación, generación de eventos, renderizado y control temporal, permitiendo demostrar de manera práctica el uso de mutexes, variables de condición, semáforos, barreras y variables atómicas en un entorno concurrente basado en POSIX Threads.