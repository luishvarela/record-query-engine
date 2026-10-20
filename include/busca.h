#ifndef BUSCA_H
#define BUSCA_H

#include <stdbool.h>
#include "TADS/Item.h"

// operadores do comando FILTER
typedef enum { 
    IGUAL,         // == 
    DIFERENTE,     // !=
    MENOR,         // < 
    MENOR_IGUAL,   // <=
    MAIOR,         // > 
    MAIOR_IGUAL    // >=
} OPERADOR;

// representa 1 regra FILTER
typedef struct {
    int campo_idx;        // índice da coluna que será testada
    OPERADOR operador;    // qual comparação fazer (IGUAL, MENOR_IGUAL, etc.)
    double valor_num;     // valor se o campo for NUMERO
    char valor_texto[128];// texto se o campo for TEXTO
    TIPO tipo_filtro;     // NUMERO ou TEXTO
} CRITERIO_FILTER;

// representa uma regra SIM
typedef struct {
    int campo_idx;     // índice da coluna testada
    double alvo;       // valor alvo (q)
    double tolerancia; // tolerância (t)
    double peso;       // peso na média ponderada
} CRITERIO_SIM;

// conjunto de todos os critérios lidos do arquivo .txt
typedef struct {
    CRITERIO_FILTER *filtros;
    int n_filtros; 
    
    CRITERIO_SIM *sims;
    int n_sims;
    
    double limiar_accept;     // limiar do comando ACCEPT
} CONSULTA;

// avalia se 1 ITEM atende a todos os critérios da CONSULTA
bool avaliar_item(ITEM *item, const CONSULTA *consulta);

#endif