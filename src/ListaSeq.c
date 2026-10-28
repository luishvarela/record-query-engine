#include <stdlib.h>
#include <string.h>
#include "ListaSeq.h"
 
#define CAPACIDADE_INICIAL 16
 
struct StructListaSeq {
    ITEM **Itens;
    int    Tamanho;
    int    Capacidade;
};
 
// Cria uma lista sequencial com sua capacidade inicial.
LISTASEQ *lsCriar(void){
    LISTASEQ *Lista = (LISTASEQ *) calloc(1, sizeof(LISTASEQ));
    if(Lista == NULL) return NULL;
 
    Lista->Itens = (ITEM **) malloc(CAPACIDADE_INICIAL * sizeof(ITEM *));
    if(Lista->Itens == NULL){
        free(Lista);
        return NULL;
    }
    Lista->Capacidade = CAPACIDADE_INICIAL;

    return Lista;
}
 
// Libera o vetor e, quando solicitado, os itens que ele referencia.
void lsApagar(LISTASEQ **Lista, bool ApagarItens){
    if(Lista == NULL || *Lista == NULL) return;
 
    if(ApagarItens)
        for(int i = 0; i < (*Lista)->Tamanho; i++) iApagar(&(*Lista)->Itens[i]);
 
    free((*Lista)->Itens);
    free(*Lista);
    *Lista = NULL;
}
 
// Acrescenta um item ao fim, dobrando a capacidade se necessario.
bool lsInserir(LISTASEQ *Lista, ITEM *Item){
    if(Lista == NULL || Item == NULL) return false;
 
    if(Lista->Tamanho == Lista->Capacidade){
        int NovaCapacidade = Lista->Capacidade * 2;

        ITEM **Novo = (ITEM **) realloc(Lista->Itens, NovaCapacidade * sizeof(ITEM *));
        if(Novo == NULL) return false;

        Lista->Itens = Novo;
        Lista->Capacidade = NovaCapacidade;
    }

    Lista->Itens[Lista->Tamanho] = Item;
    Lista->Tamanho++;

    return true;
}
 
// Remove pelo endereco do item e desloca os elementos seguintes para fechar o espaco.
ITEM *lsRemover(LISTASEQ *Lista, const ITEM *Item){
    if(Lista == NULL || Item == NULL) return NULL;
 
    for(int i = 0; i < Lista->Tamanho; i++){
        if(Lista->Itens[i] == Item){
            ITEM *Removido = Lista->Itens[i];
            memmove(&Lista->Itens[i], &Lista->Itens[i + 1], (Lista->Tamanho - i - 1) * sizeof(ITEM *));

            Lista->Tamanho--;

            return Removido;
        }
    }

    return NULL;
}
 
// Retorna o item na posicao indicada ou NULL se ela estiver fora da lista.
ITEM *lsGet(const LISTASEQ *Lista, int Posicao){
    if(Lista == NULL || Posicao < 0 || Posicao >= Lista->Tamanho) return NULL;
    return Lista->Itens[Posicao];
}
 
// Retorna a quantidade de itens da lista.
int lsTamanho(const LISTASEQ *Lista){
    return (Lista == NULL) ? 0 : Lista->Tamanho;
}

// Informa se a lista nao contem itens.
bool lsVazia(const LISTASEQ *Lista){
    if(lsTamanho(Lista) == 0) return true;
    else                      return false;
}

// Visita os itens na ordem dos indices
void lsPercorrer(const LISTASEQ *Lista, VISITA Visitar, void *Contexto){
    if(Lista == NULL || Visitar == NULL) return;
    for(int i = 0; i < Lista->Tamanho; i++)
        if(Visitar(Lista->Itens[i], Contexto)) return;
}