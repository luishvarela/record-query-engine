#ifndef LISTA_CABECA_H
#define LISTA_CABECA_H
 
#include "Item.h"
 
typedef struct StructListaCab LISTACAB;
 
LISTACAB *lcCriar(void);
void      lcApagar(LISTACAB **Lista, bool ApagarItens);

bool      lcInserir(LISTACAB *Lista, ITEM *Item);
ITEM     *lcRemover(LISTACAB *Lista, const ITEM *Item);
int       lcTamanho(const LISTACAB *Lista);
bool      lcVazia(const LISTACAB *Lista);
void      lcPercorrer(const LISTACAB *Lista, VISITA Visitar, void *Contexto);
 
#endif