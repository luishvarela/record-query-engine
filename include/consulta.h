#ifndef CONSULTA_H
#define CONSULTA_H

#include <stdbool.h>
#include <stddef.h>
#include "busca.h"
#include "leitura.h"

#define CAMPO_SIMILARIDADE (-1)

// Consulta parseada: filtros/similaridades e configuracoes das estruturas de resultado.
typedef struct {
    CONSULTA consulta;

    bool tem_order;
    int  campo_ordem;
    bool ordem_desc;

    int *campos_grupo;
    int  n_grupo;

    bool tem_cross;
    int  cross_a, cross_b;
} PLANO_CONSULTA;

bool consulta_carregar(const char *path, const ESQUEMA *esquema, PLANO_CONSULTA *plano, char *erro, size_t tam_erro);
void consulta_liberar(PLANO_CONSULTA *plano);

#endif