#ifndef GAME_H
#define GAME_H

/**
 * Inicia el bucle principal del juego y devuelve la puntuación final.
 * No recibe parámetros.
 */
int startGame();

/**
 * Muestra la pantalla de instrucciones y espera a que el jugador pulse una tecla.
 */
void instructions();

/**
 * Muestra las puntuaciones almacenadas en el archivo de resultados.
 */
void showScores();

#endif