#ifndef CHAVE_H
#define CHAVE_H

#include "Item.h"

// Chave de agrupamento extraida de um campo de um ITEM (usada por ListaGen e ListaCruz).
typedef struct StructChave {
    TIPO   Tipo;
    double Numero;
    char  *Texto;   //Copia propria
} CHAVE;

bool cCriar(const ITEM *Item, int Campo, CHAVE *Chave);
void cApagar(CHAVE *Chave);
int  cComparar(const CHAVE *A, const CHAVE *B);

#endif
