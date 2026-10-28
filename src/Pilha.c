#include <stdlib.h>
#include "Pilha.h"
 
typedef struct StructNo NO;
struct StructNo {
    ITEM *Item;
    NO   *Proximo;
};
 
struct StructPilha {
    NO  *Topo;
    int  Tamanho;
};
 
// Cria uma pilha vazia.
PILHA *pCriar(void){
    return (PILHA *) calloc(1, sizeof(PILHA));
}
 
// Libera todos os nos e, opcionalmente, os itens armazenados.
void pApagar(PILHA **Pilha, bool ApagarItens){
    if(Pilha == NULL || *Pilha == NULL) return;
 
    NO *Atual = (*Pilha)->Topo;
    while(Atual != NULL){
        NO *Proximo = Atual->Proximo;
        if(ApagarItens) iApagar(&Atual->Item);
        free(Atual);
        Atual = Proximo;
    }
    free(*Pilha);
    *Pilha = NULL;
}
 
// Coloca um item no topo da pilha.
bool pEmpilhar(PILHA *Pilha, ITEM *Item){
    if(Pilha == NULL || Item == NULL) return false;
 
    NO *Novo = (NO *) malloc(sizeof(NO));
    if(Novo == NULL) return false;
 
    Novo->Item = Item;
    Novo->Proximo = Pilha->Topo;
    Pilha->Topo = Novo;
    Pilha->Tamanho++;
    return true;
}
 
// Remove e retorna o item do topo da pilha.
ITEM *pDesempilhar(PILHA *Pilha){
    if(Pilha == NULL || Pilha->Topo == NULL) return NULL;
 
    NO *No = Pilha->Topo;
    ITEM *Item = No->Item;
    Pilha->Topo = No->Proximo;
    Pilha->Tamanho--;
    free(No);
    return Item;
}
 
// Consulta o item do topo sem remove-lo.
ITEM *pTopo(const PILHA *Pilha){
    return (Pilha == NULL || Pilha->Topo == NULL) ? NULL : Pilha->Topo->Item;
}
 
// Retorna a quantidade de itens da pilha.
int pTamanho(const PILHA *Pilha){
    return (Pilha == NULL) ? 0 : Pilha->Tamanho;
}
 
// Informa se a pilha nao contem itens.
bool pVazia(const PILHA *Pilha){
    if(pTamanho(Pilha) == 0) return true;
    else                     return false;
}
 
// Visita os itens do topo para a base
void pPercorrer(const PILHA *Pilha, VISITA Visitar, void *Contexto){
    if(Pilha == NULL || Visitar == NULL) return;
    for(NO *Atual = Pilha->Topo; Atual != NULL; Atual = Atual->Proximo)
        if(Visitar(Atual->Item, Contexto)) return;
}