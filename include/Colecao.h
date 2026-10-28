#ifndef COLECAO_H
#define COLECAO_H

#include "Item.h"

// Interface unica sobre as 8 estruturas, apenas uma determinada pelo cliente e usada
typedef struct StructColecao COLECAO;

//Colecao de parametros que podem ser utilizados
typedef struct StructConfig {
    COMPARA    Comparar;        // lista_ord (ORDER); NULL = ordem de insercao
    int        CampoOrdem;      // campo/criterio repassado ao Comparar
    const int *CamposGrupo;     // lista_gen (GROUP): campo de cada nivel
    int        nCamposGrupo;
    int        CampoCruzA;      // lista_cruz (CROSS): campos que vao ser usados
    int        CampoCruzB;
} CONFIG;

// Nomes: pilha, fila, lista_seq, lista_enc, lista_cabeca, lista_ord, lista_gen, lista_cruz. Config pode ser NULL. Devolve NULL se o nome for invalido.
bool     colNomeValido(const char *Nome);
COLECAO *colCriar(const char *Nome, const CONFIG *Config);

// se ApagarItens = true apaga os itens, se nao apaga apenas os nos/celulas
void     colApagar(COLECAO **Colecao, bool ApagarItens);

bool     colInserir(COLECAO *Colecao, ITEM *Item);
int      colTamanho(const COLECAO *Colecao);

// Percorre na ordem natural da estrutura (topo->base na pilha, grupo a grupo na generalizada, eixo 0 na cruzada...) ate acabar ou Visitar devolver true.
void     colPercorrer(const COLECAO *Colecao, VISITA Visitar, void *Contexto);
void     colPercorrerEixo(const COLECAO *Colecao, int Eixo, VISITA Visitar, void *Contexto);

#endif
