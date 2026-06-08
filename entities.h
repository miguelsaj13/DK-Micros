#ifndef ENTITIES_H
#define ENTITIES_H

/*
 * Tipos de datos que representan los objetos activos en el juego.
 * Las estructuras son simples y están diseñadas para compartirse entre hilos.
 */

// Estado dinámico del jugador en el juego.
struct Player {
    int x;           // Posición horizontal en el mapa.
    int y;           // Posición vertical en el mapa.
    int lives;       // Vidas disponibles.
    int score;       // Puntuación acumulada.

    bool jumping;    // Indica si el jugador está en un salto.
    int jumpFrames;  // Conteo de fotogramas de salto restantes.
    int jumpHeight;  // Altura actual alcanzada durante el salto.
};

// Estado dinámico de un barril en movimiento.
struct Barrel {
    int x;              // Posición horizontal en el mapa.
    int y;              // Posición vertical en el mapa.
    bool active;        // Estado de actividad del barril.
    int direction;      // Dirección de desplazamiento (-1 o +1).

    bool alreadyJumped; // Indica si el jugador ya saltó este barril.
};

// Estado de Donkey Kong.
struct DonkeyKong {
    int x; // Posición horizontal en el mapa.
    int y; // Posición vertical en el mapa.
};

#endif