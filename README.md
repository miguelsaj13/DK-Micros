# Donkey Kong Clásico Concurrente

## Universidad del Valle de Guatemala

### Facultad de Ingeniería

### CC3086 - Programación de Microprocesadores

---

## Integrantes

* Ricardo Escobar
* Miguel Sajquín
* José Ceferino

---

## Descripción del Proyecto

Este proyecto consiste en una implementación concurrente del juego clásico **Donkey Kong** utilizando el lenguaje C++ y la biblioteca POSIX Threads (Pthreads).

El objetivo principal es controlar al personaje jugador para ascender por una estructura de plataformas y escaleras mientras evita barriles y otros obstáculos generados por Donkey Kong. El juego finaliza cuando el jugador alcanza la meta o pierde todas sus vidas.

La aplicación fue desarrollada como parte del Proyecto 2 del curso CC3086 Programación de Microprocesadores, enfocándose en el uso de concurrencia, sincronización entre hilos y representación gráfica mediante ASCII-Art en consola.

---

## Características Implementadas

### Sistema de Juego

* Movimiento horizontal del jugador.
* Subida y bajada de escaleras.
* Sistema de salto.
* Sistema de vidas.
* Sistema de puntuación.
* Menú principal interactivo.
* Pantalla de instrucciones.
* Tabla de puntajes destacados.
* Múltiples niveles de dificultad.

### Elementos del Juego

* Jugador.
* Donkey Kong.
* Barriles.
* Escaleras.
* Plataformas.
* Martillo temporal.
* Zona de meta.

### Concurrencia

El juego utiliza múltiples hilos POSIX para simular la ejecución paralela de los elementos principales:

| Hilo        | Función                             |
| ----------- | ----------------------------------- |
| Jugador     | Control de movimiento e interacción |
| Donkey Kong | Generación de barriles              |
| Barriles    | Movimiento de obstáculos            |
| Render      | Actualización visual del juego      |
| Entrada     | Captura de teclado                  |

---

## Tecnologías Utilizadas

### Lenguaje

* C++17

### Bibliotecas

| Biblioteca | Propósito                           |
| ---------- | ----------------------------------- |
| pthread.h  | Manejo de hilos POSIX               |
| ncurses.h  | Interfaz gráfica en consola         |
| unistd.h   | Temporización y control del sistema |
| vector     | Manejo dinámico de estructuras      |
| fstream    | Lectura y escritura de archivos     |
| algorithm  | Ordenamiento de puntajes            |
| ctime      | Generación de eventos aleatorios    |

---

## Controles

| Tecla   | Acción          |
| ------- | --------------- |
| A       | Mover izquierda |
| D       | Mover derecha   |
| W       | Subir escalera  |
| S       | Bajar escalera  |
| Espacio | Saltar          |
| P       | Pausar          |
| Q       | Salir           |

---

## Estructura del Proyecto

```text
donkeykong/
│
├── main.cpp
├── menu.cpp
├── menu.h
├── game.cpp
├── game.h
├── render.cpp
├── render.h
├── input.cpp
├── input.h
├── entities.h
├── globals.h
├── scores.txt
└── README.md
```

---

## Compilación

### Requisitos

Ubuntu / Debian:

```bash
sudo apt install build-essential
sudo apt install libncurses5-dev libncursesw5-dev
```

### Compilar

```bash
g++ main.cpp menu.cpp game.cpp render.cpp input.cpp -o donkeykong -lncurses -lpthread
```

---

## Ejecución

```bash
./donkeykong
```

---

## Sistema de Puntajes

Los puntajes se almacenan en el archivo:

```text
scores.txt
```

Al finalizar una partida, el jugador puede ingresar su nombre y la puntuación obtenida será registrada para futuras ejecuciones del programa.

---

## Mecanismos de Sincronización

Para evitar condiciones de carrera entre los distintos hilos se utilizaron:

* Mutex (`pthread_mutex_t`)
* Secciones críticas protegidas
* Variables compartidas sincronizadas

Estos mecanismos garantizan la consistencia de la información del juego mientras múltiples hilos acceden simultáneamente a ella.

---

## Consideraciones

* El proyecto fue desarrollado para sistemas Linux y entornos WSL.
* Se recomienda ejecutar el juego en una terminal compatible con ncurses.
* La resolución mínima recomendada es de 80x30 caracteres.

---
