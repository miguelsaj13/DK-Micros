#ifndef INPUT_H
#define INPUT_H

/*
 * Interfaz del hilo de entrada.
 * El hilo captura pulsaciones de teclado y notifica al resto del sistema.
 */
void *inputThread(void *arg);

#endif // INPUT_H