#include <stdlib.h>
#include <string.h>
#include "ListaGen.h"
#include "Chave.h"

typedef struct StructNoGen NOGEN;

typedef struct {
    NOGEN *Inicio;
    NOGEN *Fim;
} SUBLISTA;

typedef enum EnumTipoNo {NO_ATOMO, NO_GRUPO} TIPONO;

struct StructNoGen {
    TIPONO Tipo;
    NOGEN *Proximo;
    union {
        ITEM *Item;                     // Atomo
        struct {                        // Sublista
            CHAVE    Chave;
            SUBLISTA Filhos;
        } Grupo;
    } Conteudo;
};

struct StructListaGen {
    SUBLISTA  Raiz;
    int      *Campos;      // Campo em cada Nivel
    int       nNiveis;
    int       Tamanho;
};

// Desconecta um no da sublista e corrige inicio/fim.
static void lgDesligar(SUBLISTA *Sub, NOGEN *Anterior, NOGEN *Atual){
    if(Anterior == NULL) Sub->Inicio = Atual->Proximo;
    else                 Anterior->Proximo = Atual->Proximo;

    if(Sub->Fim == Atual) Sub->Fim = Anterior;
}

// Libera recursivamente uma sublista, com opcao de tambem destruir seus itens.
static void lgApagarSub(SUBLISTA *Sub, bool ApagarItens){
    NOGEN *Atual = Sub->Inicio;
    while(Atual != NULL){
        NOGEN *Proximo = Atual->Proximo;

        if(Atual->Tipo == NO_ATOMO){
            if(ApagarItens) iApagar(&Atual->Conteudo.Item);
        }
        else{
            lgApagarSub(&Atual->Conteudo.Grupo.Filhos, ApagarItens);
            cApagar(&Atual->Conteudo.Grupo.Chave);
        }

        free(Atual);
        Atual = Proximo;
    }
    Sub->Inicio = NULL;
    Sub->Fim    = NULL;
}

// Acha em qual grupo o item entra
static NOGEN *lgLocalizarGrupo(const SUBLISTA *Sub, const CHAVE *Chave, NOGEN **Anterior){
    NOGEN *Ant = NULL;
    NOGEN *Atual = Sub->Inicio;

    while(Atual != NULL && cComparar(&Atual->Conteudo.Grupo.Chave, Chave) < 0){
        Ant = Atual;
        Atual = Atual->Proximo;
    }

    *Anterior = Ant;
    return Atual;
}

// Insere o item no nivel atual ou desce pelos grupos ate chegar ao atomo.
static bool lgInserirNivel(LISTAGEN *Lista, SUBLISTA *Sub, int Nivel, ITEM *Item){
    // Ao chegar ao ultimo nivel, armazena o item como atomo; antes disso, cria/usa grupos.
    if(Nivel == Lista->nNiveis){
        NOGEN *Atomo = (NOGEN *) calloc(1, sizeof(NOGEN));
        if(Atomo == NULL) return false;

        Atomo->Tipo = NO_ATOMO;
        Atomo->Conteudo.Item = Item;

        if(Sub->Fim == NULL) Sub->Inicio = Atomo;
        else                 Sub->Fim->Proximo = Atomo;
        Sub->Fim = Atomo;

        return true;
    }

    CHAVE Chave;
    if(!cCriar(Item, Lista->Campos[Nivel], &Chave)) return false;

    NOGEN *Anterior;
    NOGEN *Grupo = lgLocalizarGrupo(Sub, &Chave, &Anterior);

    if(Grupo != NULL && cComparar(&Grupo->Conteudo.Grupo.Chave, &Chave) == 0){
        cApagar(&Chave);                             // grupo ja existe
    }
    else{
        NOGEN *Novo = (NOGEN *) calloc(1, sizeof(NOGEN));
        if(Novo == NULL){
            cApagar(&Chave);
            return false;
        }

        Novo->Tipo = NO_GRUPO;
        Novo->Conteudo.Grupo.Chave = Chave;          // Grupo dono da chave

        Novo->Proximo = Grupo;
        if(Anterior == NULL) Sub->Inicio = Novo;
        else                 Anterior->Proximo = Novo;
        if(Grupo == NULL) Sub->Fim = Novo;

        Grupo = Novo;
    }

    return lgInserirNivel(Lista, &Grupo->Conteudo.Grupo.Filhos, Nivel + 1, Item);
}

// Procura o item pelo caminho de chaves e remove grupos que ficarem vazios.
static ITEM *lgRemoverNivel(LISTAGEN *Lista, SUBLISTA *Sub, int Nivel, const ITEM *Item){
    if(Nivel == Lista->nNiveis){
        NOGEN *Anterior = NULL;
        for(NOGEN *Atual = Sub->Inicio; Atual != NULL; Anterior = Atual, Atual = Atual->Proximo){
            if(Atual->Conteudo.Item == Item){
                ITEM *Removido = Atual->Conteudo.Item;
                lgDesligar(Sub, Anterior, Atual);
                free(Atual);
                return Removido;
            }
        }
        return NULL;
    }

    CHAVE Chave;
    if(!cCriar(Item, Lista->Campos[Nivel], &Chave)) return NULL;

    NOGEN *Anterior;
    NOGEN *Grupo = lgLocalizarGrupo(Sub, &Chave, &Anterior);
    bool Achou = (Grupo != NULL && cComparar(&Grupo->Conteudo.Grupo.Chave, &Chave) == 0);
    cApagar(&Chave);
    if(!Achou) return NULL;

    ITEM *Removido = lgRemoverNivel(Lista, &Grupo->Conteudo.Grupo.Filhos, Nivel + 1, Item);

    // Remove grupos vazios ao voltar da recursao para nao deixar niveis sem itens.
    if(Removido != NULL && Grupo->Conteudo.Grupo.Filhos.Inicio == NULL){   // remove grupo vazio
        lgDesligar(Sub, Anterior, Grupo);
        cApagar(&Grupo->Conteudo.Grupo.Chave);
        free(Grupo);
    }
    return Removido;
}

// Percorre recursivamente atomos e grupos
static bool lgPercorrerSub(const SUBLISTA *Sub, VISITA Visitar, void *Contexto){
    // Percorre os atomos em ordem e propaga o pedido de parada
    for(NOGEN *Atual = Sub->Inicio; Atual != NULL; Atual = Atual->Proximo){
        if(Atual->Tipo == NO_ATOMO){
            if(Visitar(Atual->Conteudo.Item, Contexto)) return true;
        }
        else if(lgPercorrerSub(&Atual->Conteudo.Grupo.Filhos, Visitar, Contexto)) return true;
    }
    return false;
}

// Cria a lista generalizada e copia os campos que definem cada nivel.
LISTAGEN *lgCriar(const int *Campos, int nNiveis){
    if(nNiveis < 0 || (nNiveis > 0 && Campos == NULL)) return NULL;

    LISTAGEN *Lista = (LISTAGEN *) calloc(1, sizeof(LISTAGEN));
    if(Lista == NULL) return NULL;

    if(nNiveis > 0){
        Lista->Campos = (int *) malloc(nNiveis * sizeof(int));
        if(Lista->Campos == NULL){
            free(Lista);
            return NULL;
        }
        memcpy(Lista->Campos, Campos, nNiveis * sizeof(int));
    }
    Lista->nNiveis = nNiveis;

    return Lista;
}

// Libera a arvore de grupos e a configuracao de campos da lista.
void lgApagar(LISTAGEN **Lista, bool ApagarItens){
    if(Lista == NULL || *Lista == NULL) return;

    lgApagarSub(&(*Lista)->Raiz, ApagarItens);

    free((*Lista)->Campos);
    free(*Lista);
    *Lista = NULL;
}

// Insere um item na hierarquia configurada e atualiza seu tamanho.
bool lgInserir(LISTAGEN *Lista, ITEM *Item){
    if(Lista == NULL || Item == NULL) return false;
    if(!lgInserirNivel(Lista, &Lista->Raiz, 0, Item)) return false;

    Lista->Tamanho++;
    return true;
}

// Remove um item da hierarquia e retorna sua propriedade ao chamador.
ITEM *lgRemover(LISTAGEN *Lista, const ITEM *Item){
    if(Lista == NULL || Item == NULL) return NULL;

    ITEM *Removido = lgRemoverNivel(Lista, &Lista->Raiz, 0, Item);
    if(Removido != NULL) Lista->Tamanho--;

    return Removido;
}

// Retorna a quantidade total de atomos armazenados.
int lgTamanho(const LISTAGEN *Lista){
    return (Lista == NULL) ? 0 : Lista->Tamanho;
}

// Informa se a hierarquia nao contem atomos.
bool lgVazia(const LISTAGEN *Lista){
    if(lgTamanho(Lista) == 0) return true;
    else                      return false;
}

// Inicia o percurso pela sublista raiz.
void lgPercorrer(const LISTAGEN *Lista, VISITA Visitar, void *Contexto){
    if(Lista == NULL || Visitar == NULL) return;
    lgPercorrerSub(&Lista->Raiz, Visitar, Contexto);
}
