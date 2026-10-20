#ifndef LISTA_ORD_H
#define LISTA_ORD_H
 
#include "Item.h"

typedef struct StructListaOrd LISTAORD;
 
LISTAORD *loCriar(COMPARA Comparar, int Campo);
void      loApagar(LISTAORD **Lista, bool ApagarItens);
 
bool      loInserir(LISTAORD *Lista, ITEM *Item);
ITEM     *loRemover(LISTAORD *Lista, const ITEM *Item);
int       loTamanho(const LISTAORD *Lista);
bool      loVazia(const LISTAORD *Lista);
void      loPercorrer(const LISTAORD *Lista, VISITA Visitar, void *Contexto);
 
#endif