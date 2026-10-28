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
 
// Cria uma copia alocada de uma string.
static char *Copiar(const char *Texto){
    size_t Tamanho = strlen(Texto) + 1;
    char *Copia = (char *) malloc(Tamanho);
    if(Copia != NULL) memcpy(Copia, Texto, Tamanho);
    return Copia;
}

// Cria um item com a linha original e uma copia tipada de cada campo.
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

// Libera a linha, os campos e o proprio item, deixando o ponteiro nulo.
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
 
// Retorna o numero de campos, ou zero para um item nulo.
int iGetNCampos(const ITEM *Item){
    return (Item == NULL) ? 0 : Item->nCampos;
}
 
// Retorna o tipo do campo ou AUSENTE se o indice for invalido.
TIPO iGetTipo(const ITEM *Item, int Campo){
    if(Item == NULL || Campo < 0 || Campo >= Item->nCampos) return AUSENTE;
    return Item->Campos[Campo].Tipo;
}
 
// Retorna o valor numerico do campo ou -1 se ele nao for numerico.
double iGetNumero(const ITEM *Item, int Campo){
    return (iGetTipo(Item, Campo) == NUMERO) ? Item->Campos[Campo].Conteudo.Numero : -1;
}
 
// Retorna o texto do campo ou NULL se ele nao for textual.
const char *iGetTexto(const ITEM *Item, int Campo){
    return (iGetTipo(Item, Campo) == TEXTO) ? Item->Campos[Campo].Conteudo.Texto : NULL;
}
 
// Retorna a linha original preservada para exportacao.
const char *iGetLinha(const ITEM *Item){
    return (Item == NULL) ? NULL : Item->Linha;
}
 
// Zera os acumuladores usados para calcular a similaridade ponderada.
void iResetSim(ITEM *Item){
    if(Item == NULL) return;
    Item->SimSomaPonderada = 0.0;
    Item->SimSomaPesos     = 0.0;
}
 
// Soma uma similaridade ponderada, ignorando pesos nulos ou negativos.
void iAddSim(ITEM *Item, double Similaridade, double Peso){
    if(Item == NULL || Peso <= 0.0) return;
    Item->SimSomaPonderada += Similaridade * Peso;
    Item->SimSomaPesos     += Peso;
}
 
// Retorna a media ponderada acumulada, ou o valor neutro se nao ha pesos.
double iGetSim(const ITEM *Item){
    if(Item == NULL) return -1.0;
    // Sem criterios ponderados, a similaridade neutra e 1.
    if(Item->SimSomaPesos <= 0.0) return 1.0;
    return (Item->SimSomaPonderada / Item->SimSomaPesos);
}
 
// Calcula similaridade linear entre campo e alvo dentro da tolerancia.
double iSimilaridade(double Campo, double Alvo, double Tolerancia){
    if(Tolerancia <= 0.0) return 0.0;
    // A similaridade cai linearmente com a distancia e nunca fica abaixo de zero.
    double Similaridade = 1.0 - (fabs(Campo - Alvo) / Tolerancia);
    return (Similaridade > 0.0) ? Similaridade : 0.0;
}