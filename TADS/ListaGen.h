#ifndef LISTA_GEN_H
#define LISTA_GEN_H

#include "Item.h"

//Lista generalizada: cada elemento e um ATOMO (um ITEM) ou um GRUPO (chave + sublista). Os niveis vem dos campos do comando GROUP.

typedef struct StructListaGen LISTAGEN;

LISTAGEN *lgCriar(const int *Campos, int nNiveis);
void      lgApagar(LISTAGEN **Lista, bool ApagarItens);

bool      lgInserir(LISTAGEN *Lista, ITEM *Item);
ITEM     *lgRemover(LISTAGEN *Lista, const ITEM *Item);
int       lgTamanho(const LISTAGEN *Lista);
bool      lgVazia(const LISTAGEN *Lista);
void      lgPercorrer(const LISTAGEN *Lista, VISITA Visitar, void *Contexto);

#endif
