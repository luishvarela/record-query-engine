#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "Item.h"
 
typedef struct {
    TIPO Tipo;
    union {
        double      Numero;
        const char *Texto;
    } Conteudo;
} CAMPO;
 
struct StructItem {
    int    nCampos;
    char  *Linha;
    CAMPO *Campos;
    double SimSomaPonderada;
    double SimSomaPesos;
};
 
static char *Copiar(const char *Texto){
    size_t Tamanho = strlen(Texto) + 1;
    char *Copia = (char *) malloc(Tamanho);
    if(Copia != NULL) memcpy(Copia, Texto, Tamanho);
    return Copia;
}

ITEM *iCriar(const char *Linha, int nCampos, const TIPO *Tipos, const double *Numeros, const char *const *Textos){
    if(Linha == NULL || nCampos <= 0 || Tipos == NULL) return NULL;

    ITEM *Item = (ITEM *) calloc(1, sizeof(ITEM));
    if(Item == NULL) return NULL;

    Item->nCampos = nCampos;
    Item->Linha   = Copiar(Linha);
    Item->Campos  = (CAMPO *) calloc(nCampos, sizeof(CAMPO));

    if(Item->Linha == NULL || Item->Campos == NULL){
        iApagar(&Item);
        return NULL;
    }

    for(int i = 0; i < nCampos; i++){
        switch(Tipos[i]){
            case NUMERO:
                if(Numeros != NULL){
                    Item->Campos[i].Tipo = NUMERO;
                    Item->Campos[i].Conteudo.Numero = Numeros[i];
                }
                break;

            case TEXTO:
                if(Textos != NULL && Textos[i] != NULL && Textos[i][0] != '\0'){
                    char *t = Copiar(Textos[i]);
                    if(t == NULL){ iApagar(&Item); return NULL; }
                    Item->Campos[i].Tipo = TEXTO;
                    Item->Campos[i].Conteudo.Texto = t;
                }
                break;

            case AUSENTE:
            default:
                break;
        }
    }
    return Item;
}

void iApagar(ITEM **Item){
    if(Item == NULL || *Item == NULL) return;

    if((*Item)->Campos != NULL){
        for(int i = 0; i < (*Item)->nCampos; i++)
            if((*Item)->Campos[i].Tipo == TEXTO)
                free((void *) (*Item)->Campos[i].Conteudo.Texto);
        free((*Item)->Campos);
    }
    free((*Item)->Linha);
    free(*Item);
    *Item = NULL;
}
 
int iGetNCampos(const ITEM *Item){
    return (Item == NULL) ? 0 : Item->nCampos;
}
 
TIPO iGetTipo(const ITEM *Item, int Campo){
    if(Item == NULL || Campo < 0 || Campo >= Item->nCampos) return AUSENTE;
    return Item->Campos[Campo].Tipo;
}
 
double iGetNumero(const ITEM *Item, int Campo){
    return (iGetTipo(Item, Campo) == NUMERO) ? Item->Campos[Campo].Conteudo.Numero : -1;
}
 
const char *iGetTexto(const ITEM *Item, int Campo){
    return (iGetTipo(Item, Campo) == TEXTO) ? Item->Campos[Campo].Conteudo.Texto : NULL;
}
 
const char *iGetLinha(const ITEM *Item){
    return (Item == NULL) ? NULL : Item->Linha;
}
 
void iResetSim(ITEM *Item){
    if(Item == NULL) return;
    Item->SimSomaPonderada = 0.0;
    Item->SimSomaPesos     = 0.0;
}
 
void iAddSim(ITEM *Item, double Similaridade, double Peso){
    if(Item == NULL || Peso <= 0.0) return;
    Item->SimSomaPonderada += Similaridade * Peso;
    Item->SimSomaPesos     += Peso;
}
 
double iGetSim(const ITEM *Item){
    if(Item == NULL) return -1.0;
    if(Item->SimSomaPesos <= 0.0) return 1.0;
    return (Item->SimSomaPonderada / Item->SimSomaPesos);
}
 
double iSimilaridade(double Campo, double Alvo, double Tolerancia){
    if(Tolerancia <= 0.0) return 0.0;
    double Similaridade = 1.0 - (fabs(Campo - Alvo) / Tolerancia);
    return (Similaridade > 0.0) ? Similaridade : 0.0;
}