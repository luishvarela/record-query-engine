#ifndef FILA_H
#define FILA_H
 
#include "Item.h"

typedef struct StructFila FILA;
 
FILA  *fCriar(void);
void   fApagar(FILA **Fila, bool ApagarItens);
 
bool   fEnfileirar(FILA *Fila, ITEM *Item);
ITEM  *fDesenfileirar(FILA *Fila);
ITEM  *fInicio(const FILA *Fila);
int    fTamanho(const FILA *Fila);
bool   fVazia(const FILA *Fila);
void   fPercorrer(const FILA *Fila, VISITA Visitar, void *Contexto);
 
#endif