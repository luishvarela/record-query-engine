#ifndef PILHA_H
#define PILHA_H

#include "Item.h"

typedef struct StructPilha PILHA;

PILHA *pCriar(void);
void   pApagar(PILHA **Pilha, bool ApagarItens);
 
bool   pEmpilhar(PILHA *Pilha, ITEM *Item);
ITEM  *pDesempilhar(PILHA *Pilha);
ITEM  *pTopo(const PILHA *Pilha);
int    pTamanho(const PILHA *Pilha);
bool   pVazia(const PILHA *Pilha);
void   pPercorrer(const PILHA *Pilha, VISITA Visitar, void *Contexto);
 
#endif