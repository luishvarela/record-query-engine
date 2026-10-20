#ifndef LISTA_ENC_H
#define LISTA_ENC_H
 
#include "Item.h"

typedef struct StructListaEnc LISTAENC;
 
LISTAENC *leCriar(void);
void      leApagar(LISTAENC **Lista, bool ApagarItens);
 
bool      leInserir(LISTAENC *Lista, ITEM *Item);
ITEM     *leRemover(LISTAENC *Lista, const ITEM *Item);
int       leTamanho(const LISTAENC *Lista);
bool      leVazia(const LISTAENC *Lista);
void      lePercorrer(const LISTAENC *Lista, VISITA Visitar, void *Contexto);
 
#endif