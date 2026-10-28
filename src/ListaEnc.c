#include <stdlib.h>
#include "ListaEnc.h"
 
typedef struct StructNo NO;
struct StructNo {
    ITEM *Item;
    NO   *Proximo;
};
 
struct StructListaEnc {
    NO  *Inicio;
    NO  *Fim;
    int  Tamanho;
};
 
// Cria uma lista encadeada vazia.
LISTAENC *leCriar(void){
    return (LISTAENC *) calloc(1, sizeof(LISTAENC));
}
 
// Libera os nos e, quando solicitado, os itens armazenados.
void leApagar(LISTAENC **Lista, bool ApagarItens){
    if(Lista == NULL || *Lista == NULL) return;
 
    NO *Atual = (*Lista)->Inicio;
    while(Atual != NULL){
        NO *Proximo = Atual->Proximo;
        if(ApagarItens) iApagar(&Atual->Item);
        free(Atual);
        Atual = Proximo;
    }

    free(*Lista);
    *Lista = NULL;
}
 
// Acrescenta um item ao fim da lista encadeada.
bool leInserir(LISTAENC *Lista, ITEM *Item){
    if(Lista == NULL || Item == NULL) return false;
 
    NO *Novo = (NO *) malloc(sizeof(NO));
    if(Novo == NULL) return false;
 
    Novo->Item = Item;
    Novo->Proximo = NULL;
 
    if(Lista->Fim == NULL) Lista->Inicio = Novo;
    else                   Lista->Fim->Proximo = Novo;

    Lista->Fim = Novo;
    Lista->Tamanho++;

    return true;
}
 
// Remove o item pelo endereco e atualiza inicio e fim quando necessario.
ITEM *leRemover(LISTAENC *Lista, const ITEM *Item){
    if(Lista == NULL || Item == NULL) return NULL;
 
    NO *Anterior = NULL;
    for(NO *Atual = Lista->Inicio; Atual != NULL; Anterior = Atual, Atual = Atual->Proximo){
        if(Atual->Item == Item){
            if(Anterior == NULL) Lista->Inicio = Atual->Proximo;
            else            Anterior->Proximo  = Atual->Proximo;

            if(Lista->Fim == Atual) Lista->Fim = Anterior;
 
            ITEM *Removido = Atual->Item;
            free(Atual);

            Lista->Tamanho--;

            return Removido;
        }
    }
    
    return NULL;
}
 
// Retorna a quantidade de itens da lista.
int leTamanho(const LISTAENC *Lista){
    return (Lista == NULL) ? 0 : Lista->Tamanho;
}

// Informa se a lista nao contem itens.
bool leVazia(const LISTAENC *Lista){
    if(leTamanho(Lista) == 0) return true;
    else                      return false;
}

// Visita os itens do inicio ao fim
void lePercorrer(const LISTAENC *Lista, VISITA Visitar, void *Contexto){
    if(Lista == NULL || Visitar == NULL) return;
    for(NO *Atual = Lista->Inicio; Atual != NULL; Atual = Atual->Proximo)
        if(Visitar(Atual->Item, Contexto)) return;
} 