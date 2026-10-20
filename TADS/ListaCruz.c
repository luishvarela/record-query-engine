#include <stdlib.h>
#include "ListaCruz.h"
#include "Chave.h"

#define N_EIXOS 2

typedef struct StructCelula CELULA;
typedef struct StructGrupo  GRUPO;

struct StructCelula {
    ITEM   *Item;
    CELULA *Proximo[N_EIXOS];    /* proxima celula no grupo de cada eixo */
};

struct StructGrupo {
    CHAVE   Chave;
    CELULA *Inicio;
    CELULA *Fim;
    GRUPO  *Proximo;
};

struct StructListaCruz {
    GRUPO *Grupos[N_EIXOS];      /* lista de grupos de cada eixo */
    int    Campos[N_EIXOS];
    int    Tamanho;
};

// Acha em qual grupo o item entra
static GRUPO *lxLocalizarGrupo(const LISTACRUZ *Lista, int Eixo, const CHAVE *Chave, GRUPO **Anterior){
    GRUPO *Anterior = NULL;
    GRUPO *Atual = Lista->Grupos[Eixo];

    while(Atual != NULL && cComparar(&Atual->Chave, Chave) < 0){
        Anterior = Atual;
        Atual = Atual->Proximo;
    }

    *Anterior = Anterior;
    return Atual;
}

// Liga a celula ao fim do grupo do eixo (cria o grupo se for preciso).
static bool lxAnexar(LISTACRUZ *Lista, int Eixo, CELULA *Celula){
    CHAVE Chave;
    if(!cCriar(Celula->Item, Lista->Campos[Eixo], &Chave)) return false;

    GRUPO *Anterior;
    GRUPO *Grupo = lxLocalizarGrupo(Lista, Eixo, &Chave, &Anterior);

    if(Grupo != NULL && cComparar(&Grupo->Chave, &Chave) == 0){
        cApagar(&Chave);                             // Grupo ja existe
    }
    else{
        GRUPO *Novo = (GRUPO *) calloc(1, sizeof(GRUPO));
        if(Novo == NULL){
            cApagar(&Chave);
            return false;
        }

        Novo->Chave   = Chave;                       // Grupo dono da chave
        Novo->Proximo = Grupo;
        if(Anterior == NULL) Lista->Grupos[Eixo] = Novo;
        else                 Anterior->Proximo = Novo;

        Grupo = Novo;
    }

    Celula->Proximo[Eixo] = NULL;
    if(Grupo->Fim == NULL) Grupo->Inicio = Celula;
    else                   Grupo->Fim->Proximo[Eixo] = Celula;
    Grupo->Fim = Celula;

    return true;
}

// tira celula do eixo sem apagar o item, remove se ficar vazio
static bool lxDesligar(LISTACRUZ *Lista, int Eixo, CELULA *Celula){
    CHAVE Chave;
    if(!cCriar(Celula->Item, Lista->Campos[Eixo], &Chave)) return false;

    GRUPO *AnteriorGrupo;
    GRUPO *Grupo = lxLocalizarGrupo(Lista, Eixo, &Chave, &AnteriorGrupo);
    bool Achou = (Grupo != NULL && cComparar(&Grupo->Chave, &Chave) == 0);
    cApagar(&Chave);
    if(!Achou) return false;

    CELULA *Anterior = NULL;
    CELULA *Atual = Grupo->Inicio;
    while(Atual != NULL && Atual != Celula){
        Anterior = Atual;
        Atual = Atual->Proximo[Eixo];
    }
    if(Atual == NULL) return false;

    if(Anterior == NULL) Grupo->Inicio = Celula->Proximo[Eixo];
    else                 Anterior->Proximo[Eixo] = Celula->Proximo[Eixo];
    if(Grupo->Fim == Celula) Grupo->Fim = Anterior;

    if(Grupo->Inicio == NULL){
        if(AnteriorGrupo == NULL) Lista->Grupos[Eixo] = Grupo->Proximo;
        else                      AnteriorGrupo->Proximo = Grupo->Proximo;
        cApagar(&Grupo->Chave);
        free(Grupo);
    }

    return true;
}

static CELULA *lxEncontrar(const LISTACRUZ *Lista, const ITEM *Item){
    CHAVE Chave;
    if(!cCriar(Item, Lista->Campos[0], &Chave)) return NULL;

    GRUPO *Anterior;
    GRUPO *Grupo = lxLocalizarGrupo(Lista, 0, &Chave, &Anterior);
    bool Achou = (Grupo != NULL && cComparar(&Grupo->Chave, &Chave) == 0);
    cApagar(&Chave);
    if(!Achou) return NULL;

    for(CELULA *Atual = Grupo->Inicio; Atual != NULL; Atual = Atual->Proximo[0])
        if(Atual->Item == Item) return Atual;

    return NULL;
}

LISTACRUZ *lxCriar(int CampoA, int CampoB){
    LISTACRUZ *Lista = (LISTACRUZ *) calloc(1, sizeof(LISTACRUZ));
    if(Lista == NULL) return NULL;

    Lista->Campos[0] = CampoA;
    Lista->Campos[1] = CampoB;

    return Lista;
}

void lxApagar(LISTACRUZ **Lista, bool ApagarItens){
    if(Lista == NULL || *Lista == NULL) return;

    //Toda celula em um grupo do eixo 0, por onde sao liberadas
    GRUPO *Grupo = (*Lista)->Grupos[0];
    while(Grupo != NULL){
        GRUPO *ProximoGrupo = Grupo->Proximo;

        CELULA *Atual = Grupo->Inicio;
        while(Atual != NULL){
            CELULA *Proximo = Atual->Proximo[0];
            if(ApagarItens) iApagar(&Atual->Item);
            free(Atual);
            Atual = Proximo;
        }

        cApagar(&Grupo->Chave);
        free(Grupo);
        Grupo = ProximoGrupo;
    }

    // Celulas liberadas -> Libera os grupos
    Grupo = (*Lista)->Grupos[1];
    while(Grupo != NULL){
        GRUPO *ProximoGrupo = Grupo->Proximo;
        cApagar(&Grupo->Chave);
        free(Grupo);
        Grupo = ProximoGrupo;
    }

    free(*Lista);
    *Lista = NULL;
}

bool lxInserir(LISTACRUZ *Lista, ITEM *Item){
    if(Lista == NULL || Item == NULL) return false;

    CELULA *Nova = (CELULA *) calloc(1, sizeof(CELULA));
    if(Nova == NULL) return false;
    Nova->Item = Item;

    if(!lxAnexar(Lista, 0, Nova)){
        free(Nova);
        return false;
    }

    if(!lxAnexar(Lista, 1, Nova)){
        lxDesligar(Lista, 0, Nova);
        free(Nova);
        return false;
    }

    Lista->Tamanho++;
    return true;
}

ITEM *lxRemover(LISTACRUZ *Lista, const ITEM *Item){
    if(Lista == NULL || Item == NULL) return NULL;

    CELULA *Atual = lxEncontrar(Lista, Item);
    if(Atual == NULL) return NULL;

    lxDesligar(Lista, 0, Atual);
    lxDesligar(Lista, 1, Atual);

    ITEM *Removido = Atual->Item;
    free(Atual);
    Lista->Tamanho--;

    return Removido;
}

int lxTamanho(const LISTACRUZ *Lista){
    return (Lista == NULL) ? 0 : Lista->Tamanho;
}

bool lxVazia(const LISTACRUZ *Lista){
    if(lxTamanho(Lista) == 0) return true;
    else                      return false;
}

void lxPercorrerEixo(const LISTACRUZ *Lista, int Eixo, VISITA Visitar, void *Contexto){
    if(Lista == NULL || Visitar == NULL || Eixo < 0 || Eixo >= N_EIXOS) return;

    for(GRUPO *Grupo = Lista->Grupos[Eixo]; Grupo != NULL; Grupo = Grupo->Proximo)
        for(CELULA *Atual = Grupo->Inicio; Atual != NULL; Atual = Atual->Proximo[Eixo])
            if(Visitar(Atual->Item, Contexto)) return;
}

void lxPercorrer(const LISTACRUZ *Lista, VISITA Visitar, void *Contexto){
    lxPercorrerEixo(Lista, 0, Visitar, Contexto);
}
