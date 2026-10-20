#ifndef ITEM_H
#define ITEM_H

#include <stdbool.h>

typedef struct StructItem ITEM;

typedef enum EnumTipo {AUSENTE, NUMERO, TEXTO} TIPO;

ITEM       *iCriar(const char *Linha, int nCampos, const TIPO *Tipos, const double *Numeros, const char *const *Textos);
void        iApagar(ITEM **Item);

int         iGetNCampos(const ITEM *Item);
TIPO        iGetTipo(const ITEM *Item, int Campo);
double      iGetNumero(const ITEM *Item, int Campo);
const char *iGetTexto(const ITEM *Item, int Campo);
const char *iGetLinha(const ITEM *Item);

void        iResetSim(ITEM *Item);
void        iAddSim(ITEM *Item, double Similaridade, double Peso);
double      iGetSim(const ITEM *Item);
double      iSimilaridade(double x, double alvo, double tolerancia);

// Funcao coringa para visitar o item, devolve false pra continuar, true para parar
typedef bool (*VISITA)(ITEM *Item, void *Contexto);

/*Negativo: A antes de B; 
  zero: equivalentes; 
  positivo: A depois de B. 
  Campo = Campo comparado. */
typedef int (*COMPARA)(const ITEM *A, const ITEM *B, int Campo);

#endif