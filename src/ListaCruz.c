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
    GRUPO *Ant = NULL;
    GRUPO *Atual = Lista->Grupos[Eixo];

    while(Atual != NULL && cComparar(&Atual->Chave, Chave) < 0){
        Ant = Atual;
        Atual = Atual->Proximo;
    }

    *Anterior = Ant;
    return Atual;
}

// Liga a celula ao fim do grupo do eixo (cria o grupo se for preciso).
// Insere a celula no grupo da chave do eixo, criando o grupo quando necessario.
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
    GRUPO *AnteriorGrupo = NULL;
    for(GRUPO *Grupo = Lista->Grupos[Eixo]; Grupo != NULL; ){
        CELULA *Anterior = NULL;
        for(CELULA *Atual = Grupo->Inicio; Atual != NULL; Atual = Atual->Proximo[Eixo]){
            if(Atual == Celula){
                if(Anterior == NULL) Grupo->Inicio = Atual->Proximo[Eixo];
                else                 Anterior->Proximo[Eixo] = Atual->Proximo[Eixo];
                if(Grupo->Fim == Atual) Grupo->Fim = Anterior;

                if(Grupo->Inicio == NULL){
                    if(AnteriorGrupo == NULL) Lista->Grupos[Eixo] = Grupo->Proximo;
                    else                      AnteriorGrupo->Proximo = Grupo->Proximo;
                    cApagar(&Grupo->Chave);
                    free(Grupo);
                }
                return true;
            }
            Anterior = Atual;
        }
        AnteriorGrupo = Grupo;
        Grupo = Grupo->Proximo;
    }

    return false;
}

// Procura uma celula pelo endereco do item usando o eixo principal.
static CELULA *lxEncontrar(const LISTACRUZ *Lista, const ITEM *Item){
    for(GRUPO *Grupo = Lista->Grupos[0]; Grupo != NULL; Grupo = Grupo->Proximo)
        for(CELULA *Atual = Grupo->Inicio; Atual != NULL; Atual = Atual->Proximo[0])
            if(Atual->Item == Item) return Atual;

    return NULL;
}

// Confirma que a mesma celula tambem esta conectada ao eixo indicado.
static bool lxContemCelula(const LISTACRUZ *Lista, int Eixo, const CELULA *Celula){
    for(GRUPO *Grupo = Lista->Grupos[Eixo]; Grupo != NULL; Grupo = Grupo->Proximo)
        for(CELULA *Atual = Grupo->Inicio; Atual != NULL; Atual = Atual->Proximo[Eixo])
            if(Atual == Celula) return true;

    return false;
}

// Cria uma lista cruzada configurada com um campo de agrupamento por eixo.
LISTACRUZ *lxCriar(int CampoA, int CampoB){
    LISTACRUZ *Lista = (LISTACRUZ *) calloc(1, sizeof(LISTACRUZ));
    if(Lista == NULL) return NULL;

    Lista->Campos[0] = CampoA;
    Lista->Campos[1] = CampoB;

    return Lista;
}

// Libera as celulas uma vez pelo eixo 0 e depois os grupos do eixo 1.
void lxApagar(LISTACRUZ **Lista, bool ApagarItens){
    if(Lista == NULL || *Lista == NULL) return;

    // Cada celula pertence aos dois eixos, mas so deve ser liberada uma vez pelo eixo 0.
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

    // As celulas ja foram liberadas; no eixo 1 resta liberar somente os grupos e suas chaves.
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

// Cria uma celula compartilhada e conecta-a aos dois eixos atomicamente.
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
        // A insercao so e valida nos dois eixos; desfaz o primeiro anexo se o segundo falhar.
        lxDesligar(Lista, 0, Nova);
        free(Nova);
        return false;
    }

    Lista->Tamanho++;
    return true;
}

// Desconecta dos dois eixos e devolve o item sem libera-lo.
ITEM *lxRemover(LISTACRUZ *Lista, const ITEM *Item){
    if(Lista == NULL || Item == NULL) return NULL;

    CELULA *Atual = lxEncontrar(Lista, Item);
    if(Atual == NULL || !lxContemCelula(Lista, 1, Atual)) return NULL;

    if(!lxDesligar(Lista, 0, Atual)) return NULL;
    if(!lxDesligar(Lista, 1, Atual)) return NULL;

    ITEM *Removido = Atual->Item;
    free(Atual);
    Lista->Tamanho--;

    return Removido;
}

// Retorna o total de itens unicos da lista cruzada.
int lxTamanho(const LISTACRUZ *Lista){
    return (Lista == NULL) ? 0 : Lista->Tamanho;
}

// Informa se a lista cruzada nao contem itens.
bool lxVazia(const LISTACRUZ *Lista){
    if(lxTamanho(Lista) == 0) return true;
    else                      return false;
}

// Percorre os grupos do eixo escolhido e para se o callback solicitar.
void lxPercorrerEixo(const LISTACRUZ *Lista, int Eixo, VISITA Visitar, void *Contexto){
    if(Lista == NULL || Visitar == NULL || Eixo < 0 || Eixo >= N_EIXOS) return;

    for(GRUPO *Grupo = Lista->Grupos[Eixo]; Grupo != NULL; Grupo = Grupo->Proximo)
        for(CELULA *Atual = Grupo->Inicio; Atual != NULL; Atual = Atual->Proximo[Eixo])
            if(Visitar(Atual->Item, Contexto)) return;
}

// Percorre pelo eixo 0, que e a ordem padrao da lista cruzada.
void lxPercorrer(const LISTACRUZ *Lista, VISITA Visitar, void *Contexto){
    lxPercorrerEixo(Lista, 0, Visitar, Contexto);
}
