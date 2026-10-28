#include <stdlib.h>
#include "Fila.h"
 
typedef struct StructNo NO;
struct StructNo {
    ITEM *Item;
    NO   *Proximo;
};
 
struct StructFila {
    NO  *Inicio;
    NO  *Fim;
    int  Tamanho;
};
 
// Cria uma fila vazia.
FILA *fCriar(void){
    return (FILA *) calloc(1, sizeof(FILA));
}
 
// Libera todos os nos e, opcionalmente, os itens armazenados.
void fApagar(FILA **Fila, bool ApagarItens){
    if(Fila == NULL || *Fila == NULL) return;
 
    NO *Atual = (*Fila)->Inicio;
    while(Atual != NULL){
        NO *Proximo = Atual->Proximo;
        if(ApagarItens) iApagar(&Atual->Item);
        free(Atual);
        Atual = Proximo;
    }
    free(*Fila);
    *Fila = NULL;
}
 
// Adiciona um item ao fim da fila.
bool fEnfileirar(FILA *Fila, ITEM *Item){
    if(Fila == NULL || Item == NULL) return false;
 
    NO *Novo = (NO *) malloc(sizeof(NO));
    if(Novo == NULL) return false;
 
    Novo->Item = Item;
    Novo->Proximo = NULL;
 
    if(Fila->Fim == NULL) Fila->Inicio = Novo;
    else                  Fila->Fim->Proximo = Novo;
    Fila->Fim = Novo;
    Fila->Tamanho++;

    return true;
}
 
// Remove e retorna o item do inicio da fila.
ITEM *fDesenfileirar(FILA *Fila){
    if(Fila == NULL || Fila->Inicio == NULL) return NULL;
 
    NO *Removido = Fila->Inicio;
    ITEM *Item = Removido->Item;
    Fila->Inicio = Removido->Proximo;
    if(Fila->Inicio == NULL) Fila->Fim = NULL;
    Fila->Tamanho--;
    free(Removido);

    return Item;
}
 
// Consulta o primeiro item sem remove-lo.
ITEM *fInicio(const FILA *Fila){
    return (Fila == NULL || Fila->Inicio == NULL) ? NULL : Fila->Inicio->Item;
}
 
// Retorna a quantidade de itens da fila.
int fTamanho(const FILA *Fila){
    return (Fila == NULL) ? 0 : Fila->Tamanho;
}
 
// Informa se a fila nao contem itens.
bool fVazia(const FILA *Fila){
    if(fTamanho(Fila) == 0) return true;
    else                    return false;
}
 
// Visita os itens do inicio ao fim
void fPercorrer(const FILA *Fila, VISITA Visitar, void *Contexto){
    if(Fila == NULL || Visitar == NULL) return;
    for(NO *Atual = Fila->Inicio; Atual != NULL; Atual = Atual->Proximo)
        if(Visitar(Atual->Item, Contexto)) return;
}