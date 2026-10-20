#include <stdlib.h>
#include <string.h>
#include "Colecao.h"
#include "Pilha.h"
#include "Fila.h"
#include "ListaSeq.h"
#include "ListaEnc.h"
#include "ListaCab.h"
#include "ListaOrd.h"
#include "ListaGen.h"
#include "ListaCruz.h"

typedef enum EnumEstrutura {
    E_PILHA, E_FILA, E_SEQ, E_ENC, E_CABECA, E_ORD, E_GEN, E_CRUZ, E_INVALIDA
} ESTRUTURA;

static const char *const Nomes[] = {
    "pilha", "fila", "lista_seq", "lista_enc",
    "lista_cabeca", "lista_ord", "lista_gen", "lista_cruz"
};

struct StructColecao {
    ESTRUTURA Estrutura;
    void     *Dados;
};

static ESTRUTURA colIdentificar(const char *Nome){
    if(Nome == NULL) return E_INVALIDA;

    for(int i = 0; i < E_INVALIDA; i++)
        if(strcmp(Nome, Nomes[i]) == 0) return (ESTRUTURA) i;
    return E_INVALIDA;
}

static void colApagarDados(ESTRUTURA Estrutura, void *Dados, bool ApagarItens){
    switch(Estrutura){
        case E_PILHA:  pApagar((PILHA **) &Dados, ApagarItens); break;
        case E_FILA:   fApagar((FILA **) &Dados, ApagarItens); break;
        case E_SEQ:    lsApagar((LISTASEQ **) &Dados, ApagarItens); break;
        case E_ENC:    leApagar((LISTAENC **) &Dados, ApagarItens); break;
        case E_CABECA: lcApagar((LISTACAB **) &Dados, ApagarItens); break;
        case E_ORD:    loApagar((LISTAORD **) &Dados, ApagarItens); break;
        case E_GEN:    lgApagar((LISTAGEN **) &Dados, ApagarItens); break;
        case E_CRUZ:   lxApagar((LISTACRUZ **) &Dados, ApagarItens); break;
        default: break;
    }
}

bool colNomeValido(const char *Nome){
    return colIdentificar(Nome) != E_INVALIDA;
}

COLECAO *colCriar(const char *Nome, const CONFIG *Config){
    ESTRUTURA Estrutura = colIdentificar(Nome);
    if(Estrutura == E_INVALIDA) return NULL;

    CONFIG Padrao = {NULL, 0, NULL, 0, -1, -1};
    if(Config == NULL) Config = &Padrao;

    void *Dados = NULL;
    switch(Estrutura){
        case E_PILHA:  Dados = pCriar(); break;
        case E_FILA:   Dados = fCriar(); break;
        case E_SEQ:    Dados = lsCriar(); break;
        case E_ENC:    Dados = leCriar(); break;
        case E_CABECA: Dados = lcCriar(); break;
        case E_ORD:    Dados = loCriar(Config->Comparar, Config->CampoOrdem); break;
        case E_GEN:    Dados = lgCriar(Config->CamposGrupo, Config->nCamposGrupo); break;
        case E_CRUZ:   Dados = lxCriar(Config->CampoCruzA, Config->CampoCruzB); break;
        default: break;
    }
    
    if(Dados == NULL) return NULL;

    COLECAO *Colecao = (COLECAO *) malloc(sizeof(COLECAO));
    if(Colecao == NULL){
        colApagarDados(Estrutura, Dados, false);
        return NULL;
    }

    Colecao->Estrutura = Estrutura;
    Colecao->Dados = Dados;
    return Colecao;
}

void colApagar(COLECAO **Colecao, bool ApagarItens){
    if(Colecao == NULL || *Colecao == NULL) return;

    colApagarDados((*Colecao)->Estrutura, (*Colecao)->Dados, ApagarItens);
    free(*Colecao);
    *Colecao = NULL;
}

bool colInserir(COLECAO *Colecao, ITEM *Item){
    if(Colecao == NULL) return false;
    switch(Colecao->Estrutura){
        case E_PILHA:  return pEmpilhar((PILHA *) Colecao->Dados, Item);
        case E_FILA:   return fEnfileirar((FILA *) Colecao->Dados, Item);
        case E_SEQ:    return lsInserir((LISTASEQ *) Colecao->Dados, Item);
        case E_ENC:    return leInserir((LISTAENC *) Colecao->Dados, Item);
        case E_CABECA: return lcInserir((LISTACAB *) Colecao->Dados, Item);
        case E_ORD:    return loInserir((LISTAORD *) Colecao->Dados, Item);
        case E_GEN:    return lgInserir((LISTAGEN *) Colecao->Dados, Item);
        case E_CRUZ:   return lxInserir((LISTACRUZ *) Colecao->Dados, Item);
        default:       return false;
    }
}

int colTamanho(const COLECAO *Colecao){
    if(Colecao == NULL) return 0;
    switch(Colecao->Estrutura){
        case E_PILHA:  return pTamanho((const PILHA *) Colecao->Dados);
        case E_FILA:   return fTamanho((const FILA *) Colecao->Dados);
        case E_SEQ:    return lsTamanho((const LISTASEQ *) Colecao->Dados);
        case E_ENC:    return leTamanho((const LISTAENC *) Colecao->Dados);
        case E_CABECA: return lcTamanho((const LISTACAB *) Colecao->Dados);
        case E_ORD:    return loTamanho((const LISTAORD *) Colecao->Dados);
        case E_GEN:    return lgTamanho((const LISTAGEN *) Colecao->Dados);
        case E_CRUZ:   return lxTamanho((const LISTACRUZ *) Colecao->Dados);
        default:       return 0;
    }
}

void colPercorrer(const COLECAO *Colecao, VISITA Visitar, void *Contexto){
    if(Colecao == NULL) return;
    switch(Colecao->Estrutura){
        case E_PILHA:  pPercorrer((const PILHA *) Colecao->Dados, Visitar, Contexto); break;
        case E_FILA:   fPercorrer((const FILA *) Colecao->Dados, Visitar, Contexto); break;
        case E_SEQ:    lsPercorrer((const LISTASEQ *) Colecao->Dados, Visitar, Contexto); break;
        case E_ENC:    lePercorrer((const LISTAENC *) Colecao->Dados, Visitar, Contexto); break;
        case E_CABECA: lcPercorrer((const LISTACAB *) Colecao->Dados, Visitar, Contexto); break;
        case E_ORD:    loPercorrer((const LISTAORD *) Colecao->Dados, Visitar, Contexto); break;
        case E_GEN:    lgPercorrer((const LISTAGEN *) Colecao->Dados, Visitar, Contexto); break;
        case E_CRUZ:   lxPercorrer((const LISTACRUZ *) Colecao->Dados, Visitar, Contexto); break;
        default: break;
    }
}
