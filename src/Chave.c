#include <stdlib.h>
#include <string.h>
#include "Chave.h"

// Extrai do item uma chave independente para ordenacao e agrupamento.
bool cCriar(const ITEM *Item, int Campo, CHAVE *Chave){
    if(Chave == NULL) return false;

    Chave->Tipo   = AUSENTE;
    Chave->Numero = 0.0;
    Chave->Texto  = NULL;

    switch(iGetTipo(Item, Campo)){
        case NUMERO:
            Chave->Tipo   = NUMERO;
            Chave->Numero = iGetNumero(Item, Campo);
            break;

        case TEXTO: {
            const char *Texto = iGetTexto(Item, Campo);
            size_t Tamanho = strlen(Texto) + 1;

            // A chave armazena uma copia propria, independente da vida do ITEM.
            Chave->Texto = (char *) malloc(Tamanho);
            if(Chave->Texto == NULL) return false;

            memcpy(Chave->Texto, Texto, Tamanho);
            Chave->Tipo = TEXTO;
            break;
        }

        case AUSENTE:
        default:
            break;
    }
    return true;
}

// Libera o texto proprio da chave e a redefine como ausente.
void cApagar(CHAVE *Chave){
    if(Chave == NULL) return;
    free(Chave->Texto);
    Chave->Texto = NULL;
    Chave->Tipo  = AUSENTE;
}

// Compara chaves por tipo e, quando iguais, pelo valor armazenado.
int cComparar(const CHAVE *A, const CHAVE *B){
    if(A->Tipo != B->Tipo) return (A->Tipo < B->Tipo) ? -1 : 1;

    switch(A->Tipo){
        case NUMERO:
            if(A->Numero < B->Numero) return -1;
            if(A->Numero > B->Numero) return  1;
            return 0;

        case TEXTO:
            return strcmp(A->Texto, B->Texto);

        case AUSENTE:
        default:
            return 0;
    }
}
