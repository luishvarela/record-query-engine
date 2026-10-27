#include <stdlib.h>
#include "ListaCab.h"
 
typedef struct StructNo NO;
struct StructNo {
    ITEM *Item;
    NO   *Proximo;
};
 
struct StructListaCab {
    NO  *Cabeca;
    NO  *Fim;
    int  Tamanho;
};
 
LISTACAB *lcCriar(void){
    LISTACAB *Lista = (LISTACAB *) calloc(1, sizeof(LISTACAB));
    if(Lista == NULL) return NULL;
 
    Lista->Cabeca = (NO *) calloc(1, sizeof(NO));
    if(Lista->Cabeca == NULL){
        free(Lista);
        return NULL;
    }

    Lista->Fim = Lista->Cabeca;

    return Lista;
}
 
void lcApagar(LISTACAB **Lista, bool ApagarItens){
    if(Lista == NULL || *Lista == NULL) return;
 
    NO *Atual = (*Lista)->Cabeca;
    while(Atual != NULL){
        NO *Proximo = Atual->Proximo;
        if(ApagarItens) iApagar(&Atual->Item);
        free(Atual);
        Atual = Proximo;
    }

    free(*Lista);

    *Lista = NULL;
}
 
bool lcInserir(LISTACAB *Lista, ITEM *Item){
    if(Lista == NULL || Item == NULL) return false;
 
    NO *Novo = (NO *) malloc(sizeof(NO));
    if(Novo == NULL) return false;
 
    Novo->Item = Item;
    Novo->Proximo = NULL;

    Lista->Fim->Proximo = Novo;
    Lista->Fim = Novo;

    Lista->Tamanho++;
    return true;
}
 
ITEM *lcRemover(LISTACAB *Lista, const ITEM *Item){
    if(Lista == NULL || Item == NULL) return NULL;
 
    for(NO *Anterior = Lista->Cabeca; Anterior->Proximo != NULL; Anterior = Anterior->Proximo){
        NO *Atual = Anterior->Proximo;

        if(Atual->Item == Item){
            Anterior->Proximo = Atual->Proximo;
            if(Lista->Fim == Atual) Lista->Fim = Anterior;
 
            ITEM *Removido = Atual->Item;
            free(Atual);
            Lista->Tamanho--;

            return Removido;
        }
    }

    return NULL;
}
 
int lcTamanho(const LISTACAB *Lista){
    return (Lista == NULL) ? 0 : Lista->Tamanho;
}

bool lcVazia(const LISTACAB *Lista){
    if(lcTamanho(Lista) == 0) return true;
    else                      return false;
}
 
void lcPercorrer(const LISTACAB *Lista, VISITA Visitar, void *Contexto){
    if(Lista == NULL || Visitar == NULL) return;
    for(NO *Atual = Lista->Cabeca->Proximo; Atual != NULL; Atual = Atual->Proximo)
        if(Visitar(Atual->Item, Contexto)) return;
}