#include <stdlib.h>
#include "ListaOrd.h"
 
typedef struct StructNo NO;
struct StructNo {
    ITEM *Item;
    NO   *Proximo;
};
 
struct StructListaOrd {
    NO      *Inicio;
    NO      *Fim;
    int      Tamanho;
    COMPARA  Comparar;
    int      Campo;
};
 
// Usa o comparador configurado; sem comparador, considera os itens equivalentes.
static int loComparar(const LISTAORD *Lista, const ITEM *A, const ITEM *B){
    return (Lista->Comparar == NULL) ? 0 : Lista->Comparar(A, B, Lista->Campo);
}
 
// Cria uma lista ordenada e guarda o comparador e o campo usados na insercao.
LISTAORD *loCriar(COMPARA Comparar, int Campo){
    LISTAORD *Lista = (LISTAORD *) calloc(1, sizeof(LISTAORD));
    if(Lista != NULL) {
        Lista->Comparar = Comparar;
        Lista->Campo = Campo;
    }

    return Lista;
}
 
// Libera os nos e, quando solicitado, os itens armazenados.
void loApagar(LISTAORD **Lista, bool ApagarItens){
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
 
// Insere antes do primeiro item maior, preservando a ordem da lista.
bool loInserir(LISTAORD *Lista, ITEM *Item){
    if(Lista == NULL || Item == NULL) return false;
 
    NO *Novo = (NO *) malloc(sizeof(NO));
    if(Novo == NULL) return false;
    Novo->Item = Item;

 
    if(Lista->Inicio == NULL || loComparar(Lista, Item, Lista->Inicio->Item) < 0){
        Novo->Proximo = Lista->Inicio;
        Lista->Inicio = Novo;
        if(Lista->Fim == NULL) Lista->Fim = Novo;
    }

    else if(loComparar(Lista, Item, Lista->Fim->Item) >= 0){
        Novo->Proximo = NULL;
        Lista->Fim->Proximo = Novo;
        Lista->Fim = Novo;
    }

    else{
        NO *Anterior = Lista->Inicio;
        while(Anterior->Proximo != NULL && loComparar(Lista, Item, Anterior->Proximo->Item) >= 0)
            Anterior = Anterior->Proximo;
        Novo->Proximo = Anterior->Proximo;
        Anterior->Proximo  = Novo;
    }

    Lista->Tamanho++;

    return true;
}
 
// Remove o item pelo endereco e atualiza inicio/fim quando necessario.
ITEM *loRemover(LISTAORD *Lista, const ITEM *Item){
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
int loTamanho(const LISTAORD *Lista){
    return (Lista == NULL) ? 0 : Lista->Tamanho;
}

// Informa se a lista nao contem itens.
bool loVazia(const LISTAORD *Lista){
    if(loTamanho(Lista) == 0) return true;
    else                      return false;
}

// Visita os itens na ordem mantida pela lista.
void loPercorrer(const LISTAORD *Lista, VISITA Visitar, void *Contexto){
    if(Lista == NULL || Visitar == NULL) return;
    for(NO *Atual = Lista->Inicio; Atual != NULL; Atual = Atual->Proximo)
        if(Visitar(Atual->Item, Contexto)) return;
}