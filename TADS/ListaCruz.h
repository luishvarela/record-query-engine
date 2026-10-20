#ifndef LISTA_CRUZ_H
#define LISTA_CRUZ_H

#include "Item.h"

/* Lista cruzada: cada ITEM fica em UMA unica celula, que participa de duas organizacoes ao mesmo tempo (dois "eixos"). 
   Cada eixo e uma lista de grupos(ordenados pela chave do campo do eixo) e cada grupo encadeia as celulas com aquela chave. 
   A celula tem um ponteiro Proximo por eixo, entao nada e duplicado.
   lxPercorrer usa o eixo 0. Campo < 0 faz todos cairem em um unico grupo. */
typedef struct StructListaCruz LISTACRUZ;

LISTACRUZ *lxCriar(int CampoA, int CampoB);
void       lxApagar(LISTACRUZ **Lista, bool ApagarItens);

bool       lxInserir(LISTACRUZ *Lista, ITEM *Item);        // entra nos dois eixos
ITEM      *lxRemover(LISTACRUZ *Lista, const ITEM *Item);  // sai dos dois eixos
int        lxTamanho(const LISTACRUZ *Lista);
bool       lxVazia(const LISTACRUZ *Lista);
void       lxPercorrer(const LISTACRUZ *Lista, VISITA Visitar, void *Contexto);                 // eixo 0
void       lxPercorrerEixo(const LISTACRUZ *Lista, int Eixo, VISITA Visitar, void *Contexto);   // eixo 0 ou 1

#endif
