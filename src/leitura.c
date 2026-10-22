#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Item.h"
#include "Colecao.h"

COLECAO* ler_dataset(const char *pathArquivo, const char *tipo_base, int limite) {

    FILE *arq = fopen(pathArquivo, "r");

    if (arq == NULL) 
        return NULL;
    
    CONFIG config_padrao = {0}; 
    COLECAO *base = colCriar(tipo_base, &config_padrao);
    
    if (base == NULL) {
        fclose(arq);
        return NULL;
    }

    char linha[2048];
    
    if (!fgets(linha, sizeof(linha), arq)) {
        fclose(arq);
        colApagar(&base, true);
        return NULL;
    }

    int dadosLidos = 0;


    while (fgets(linha, sizeof(linha), arq)) {

        if (limite > 0 && dadosLidos >= limite) 
            break;
        

        linha[strcspn(linha, "\r\n")] = '\0';

        int numCampos = 0;
        TIPO *tipos = NULL;
        double *numeros = NULL;
        char **textos = NULL;

        ITEM *item = iCriar(linha, numCampos, tipos, numeros, (const char *const *)textos);

        if (item != NULL) {
            colInserir(base, item);
            dadosLidos++;
        }
    }

    fclose(arq);
    return base;
}