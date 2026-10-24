#ifndef LEITURA_H
#define LEITURA_H

#include <stdbool.h>
#include <stddef.h>
#include "Colecao.h"

// Descrição das colunas do arquivo CSV
typedef struct {
    char **nomes;      // Nome de cada coluna
    int nCampos;    // Número de colunas
    char delimitador;
    char *cabecalho;  // Cabeçalho bruto do arquivo
    bool crlf;       // True se as linhas terminam com \r\n
} ESQUEMA;

bool leitor_valido(const char *nome);
const char *leitor_listar(void);

bool esquema_abrir(ESQUEMA *e, const char *reader, const char *pathArquivo);
void esquema_liberar(ESQUEMA *e);
int esquema_indice(const ESQUEMA *e, const char *nome);

COLECAO *carregar_dataset(const char *pathArquivo, const char *tipo_base, const CONFIG *config, const ESQUEMA *e, int limite, int *lidos);

TIPO classificar_valor(const char *txt, bool virgula_decimal, double *num);
bool texto_igual_ci(const char *a, const char *b);

#endif