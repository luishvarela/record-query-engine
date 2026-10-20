#ifndef LISTA_SEQ_H
#define LISTA_SEQ_H
 
#include "Item.h"
 
typedef struct StructListaSeq LISTASEQ;
 
LISTASEQ *lsCriar(void);
void      lsApagar(LISTASEQ **Lista, bool ApagarItens);
 
bool      lsInserir(LISTASEQ *Lista, ITEM *Item);
ITEM     *lsRemover(LISTASEQ *Lista, const ITEM *Item);
ITEM     *lsGet(const LISTASEQ *Lista, int Posicao);
int       lsTamanho(const LISTASEQ *Lista);
bool      lsVazia(const LISTASEQ *Lista);
void      lsPercorrer(const LISTASEQ *Lista, VISITA Visitar, void *Contexto);
 
#endif