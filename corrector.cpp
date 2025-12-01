#include "stdafx.h"
#include "corrector.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// ---------------------------------------------------------------------------
// CONSTANTES Y UTILIDADES INTERNAS
// ---------------------------------------------------------------------------

// (ISO - 8859 - 1 / Latin1)
// a-z + ñ + á, é, í, ó, ú
const char CARACTERES_VALIDOS[] = {
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    (char)0xF1, // ñ
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    (char)0xE1, // á
    (char)0xE9, // é
    (char)0xED, // í
    (char)0xF3, // ó
    (char)0xFA  // ú
};
const int TOTAL_CHARS = 32;


bool esSeparador(unsigned char c) {
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r' ||
        c == ',' || c == '.' || c == ';' || c == '(' || c == ')');
}

void limpiarPalabra(char* str) {
    int len = strlen(str);
    while (len > 0) {
        char c = str[len - 1];
        if (c == ',' || c == '.' || c == ';' || c == '(' || c == ')') {
            str[len - 1] = '\0';
            len--;
        }
        else {
            break;
        }
    }
}

// Convierte a minúsculas para estandarizar
void hacerMinuscula(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower((unsigned char)str[i]);
    }
}

// Búsqueda binaria clásica (O(log n)) requerida por rendimiento
int buscarPosicion(char lista[][TAMTOKEN], int total, const char* buscado) {
    int inicio = 0;
    int fin = total - 1;

    while (inicio <= fin) {
        int mitad = inicio + (fin - inicio) / 2;
        int cmp = strcmp(lista[mitad], buscado);

        if (cmp == 0) return mitad; // Encontrado
        if (cmp < 0) inicio = mitad + 1;
        else fin = mitad - 1;
    }
    return -1;
}

// Inserta en el array manteniendo orden alfabético (Insertion Sort logic)
void insertarEnOrden(char lista[][TAMTOKEN], int freqs[], int& total, const char* nuevo) {
    int i = total - 1;
    // Desplazamos mientras el nuevo sea "menor" alfabéticamente
    while (i >= 0 && strcmp(lista[i], nuevo) > 0) {
        strcpy(lista[i + 1], lista[i]);
        if (freqs != NULL) freqs[i + 1] = freqs[i]; // Solo si manejamos frecuencias
        i--;
    }
    // Insertamos
    strcpy(lista[i + 1], nuevo);
    if (freqs != NULL) freqs[i + 1] = 1;
    total++;
}


void Diccionario(char* szNombre, char szPalabras[][TAMTOKEN], int iEstadisticas[], int& iNumElementos) {
    iNumElementos = 0;
    FILE* fp = fopen(szNombre, "r");

    if (!fp) return;

    char buffer[TAMTOKEN];
    int idx = 0;
    int ch; // Usamos int para detectar EOF correctamente

    while ((ch = fgetc(fp)) != EOF) {
        char c = (char)ch;

        if (esSeparador((unsigned char)c)) {
            // Fin de token detectado
            if (idx > 0) {
                buffer[idx] = '\0';
                limpiarPalabra(buffer);
                hacerMinuscula(buffer);

                if (strlen(buffer) > 0) {
                    // Verificar si ya existe
                    int pos = buscarPosicion(szPalabras, iNumElementos, buffer);

                    if (pos != -1) {
                        // Ya existe: sumar 1 a su estadística
                        iEstadisticas[pos]++;
                    }
                    else {
                        insertarEnOrden(szPalabras, iEstadisticas, iNumElementos, buffer);
                    }
                }
                idx = 0; // Reiniciar buffer
            }
        }
        else {
            // Acumular caracteres
            if (idx < TAMTOKEN - 1) {
                buffer[idx++] = c;
            }
        }
    }

    // Procesar la última palabra si el archivo no terminó en separador
    if (idx > 0) {
        buffer[idx] = '\0';
        limpiarPalabra(buffer);
        hacerMinuscula(buffer);
        if (strlen(buffer) > 0) {
            int pos = buscarPosicion(szPalabras, iNumElementos, buffer);
            if (pos != -1) iEstadisticas[pos]++;
            else insertarEnOrden(szPalabras, iEstadisticas, iNumElementos, buffer);
        }
    }

    fclose(fp);
}

void ClonaPalabras(char* szPalabraLeida, char szPalabrasSugeridas[][TAMTOKEN], int& iNumSugeridas) {
    iNumSugeridas = 0;
    int len = strlen(szPalabraLeida);
    char temp[TAMTOKEN];

    // 0. Agregar la original primero
    strcpy(szPalabrasSugeridas[iNumSugeridas++], szPalabraLeida);

    // 1. ELIMINACIÓN (Borrar cada char)
    for (int i = 0; i < len; i++) {
        int p = 0;
        for (int j = 0; j < len; j++) {
            if (i != j) temp[p++] = szPalabraLeida[j];
        }
        temp[p] = '\0';
        // Solo agregamos si no quedó vacía
        if (p > 0) strcpy(szPalabrasSugeridas[iNumSugeridas++], temp);
    }

    // 2. TRANSPOSICIÓN (Intercambiar vecinos)
    for (int i = 0; i < len - 1; i++) {
        strcpy(temp, szPalabraLeida);
        char aux = temp[i];
        temp[i] = temp[i + 1];
        temp[i + 1] = aux;
        strcpy(szPalabrasSugeridas[iNumSugeridas++], temp);
    }

    // 3. SUSTITUCIÓN (Cambiar cada char por todo el alfabeto)
    for (int i = 0; i < len; i++) {
        for (int k = 0; k < TOTAL_CHARS; k++) {
            strcpy(temp, szPalabraLeida);
            temp[i] = CARACTERES_VALIDOS[k];
            strcpy(szPalabrasSugeridas[iNumSugeridas++], temp);
        }
    }

    // 4. INSERCIÓN (Meter alfabeto en cada hueco)
    for (int i = 0; i <= len; i++) {
        for (int k = 0; k < TOTAL_CHARS; k++) {
            int p = 0;
            // Copiar primera parte
            for (int j = 0; j < i; j++) temp[p++] = szPalabraLeida[j];
            // Insertar letra
            temp[p++] = CARACTERES_VALIDOS[k];
            // Copiar resto
            for (int j = i; j < len; j++) temp[p++] = szPalabraLeida[j];
            temp[p] = '\0';

            strcpy(szPalabrasSugeridas[iNumSugeridas++], temp);
        }
    }
    // Ordenar alfabéticamente
    for (int i = 0; i < iNumSugeridas - 1; i++) {
        for (int j = 0; j < iNumSugeridas - i - 1; j++) {
            if (strcmp(szPalabrasSugeridas[j], szPalabrasSugeridas[j + 1]) > 0) {
                char aux[TAMTOKEN];
                strcpy(aux, szPalabrasSugeridas[j]);
                strcpy(szPalabrasSugeridas[j], szPalabrasSugeridas[j + 1]);
                strcpy(szPalabrasSugeridas[j + 1], aux);
            }
        }
    }
}

void ListaCandidatas(char szPalabrasSugeridas[][TAMTOKEN], int iNumSugeridas,
    char szPalabras[][TAMTOKEN], int iEstadisticas[], int iNumElementos,
    char szListaFinal[][TAMTOKEN], int iPeso[], int& iNumLista) {
    iNumLista = 0;

    // Recorremos las sugerencias generadas
    for (int i = 0; i < iNumSugeridas; i++) {

        int idxDic = buscarPosicion(szPalabras, iNumElementos, szPalabrasSugeridas[i]);

        if (idxDic != -1) {
            // Existe. Ahora verificamos no duplicarla en la lista final
            bool yaEsta = false;
            for (int k = 0; k < iNumLista; k++) {
                if (strcmp(szListaFinal[k], szPalabras[idxDic]) == 0) {
                    yaEsta = true;
                    break;
                }
            }

            if (!yaEsta) {
                strcpy(szListaFinal[iNumLista], szPalabras[idxDic]);
                iPeso[iNumLista] = iEstadisticas[idxDic];
                iNumLista++;
            }
        }
    }

    // Ordenar por peso
    for (int i = 0; i < iNumLista - 1; i++) {
        for (int j = 0; j < iNumLista - i - 1; j++) {
            // Si el siguiente pesa más, intercambiamos (para que el mayor quede arriba)
            if (iPeso[j] < iPeso[j + 1]) {
                // Swap peso
                int auxP = iPeso[j];
                iPeso[j] = iPeso[j + 1];
                iPeso[j + 1] = auxP;

                // Swap palabra
                char auxS[TAMTOKEN];
                strcpy(auxS, szListaFinal[j]);
                strcpy(szListaFinal[j], szListaFinal[j + 1]);
                strcpy(szListaFinal[j + 1], auxS);
            }
        }
    }
}