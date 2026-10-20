#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "busca.h"

bool avaliar_item(ITEM *item, const CONSULTA *consulta) {
    if (item == NULL || consulta == NULL) return false;
    if (consulta->n_filtros < 0 || consulta->n_sims < 0) return false;
    if ((consulta->n_filtros > 0 && consulta->filtros == NULL) ||
        (consulta->n_sims > 0 && consulta->sims == NULL)) return false;

    // testa todos os filtros 
    for (int i = 0; i < consulta->n_filtros; i++) {
        CRITERIO_FILTER f = consulta->filtros[i];
        TIPO tipo_item = iGetTipo(item, f.campo_idx);

        if (tipo_item == AUSENTE || tipo_item != f.tipo_filtro) return false;

        if (tipo_item == NUMERO) {
            double val = iGetNumero(item, f.campo_idx);     
            bool passou = false;

            switch (f.operador) {
                case IGUAL:       passou = (fabs(val - f.valor_num) < 1e-6); break;
                case DIFERENTE:   passou = (fabs(val - f.valor_num) >= 1e-6); break;
                case MENOR:       passou = (val < f.valor_num); break;
                case MENOR_IGUAL: passou = (val <= f.valor_num); break;
                case MAIOR:       passou = (val > f.valor_num); break;
                case MAIOR_IGUAL: passou = (val >= f.valor_num); break;
            }
            if (!passou) return false;
        } 
        else if (tipo_item == TEXTO) {
            const char *val = iGetTexto(item, f.campo_idx);
            if (val == NULL) return false;
            
            int cmp = strcmp(val, f.valor_texto);
            bool passou = false;

            switch (f.operador) {
                case IGUAL:     passou = (cmp == 0); break;
                case DIFERENTE: passou = (cmp != 0); break;
                default:        passou = false; break;
            }
            if (!passou) return false;
        }
    }

    // calcula similaridades se houver comandos sim
    if (consulta->n_sims > 0) {
        iResetSim(item);
        bool tem_sim_ponderada = false;

        for (int i = 0; i < consulta->n_sims; i++) {
            CRITERIO_SIM s = consulta->sims[i];
            if (iGetTipo(item, s.campo_idx) != NUMERO) return false;

            double val = iGetNumero(item, s.campo_idx);
            double sim_indiv = iSimilaridade(val, s.alvo, s.tolerancia);
            if (s.peso > 0.0) tem_sim_ponderada = true;
            iAddSim(item, sim_indiv, s.peso);
        }

        if (!tem_sim_ponderada) return false;

        double sim_final = iGetSim(item);
        if (sim_final < consulta->limiar_accept) {
            return false; // rejeitado pelo limiar accept
        }
    }

    return true; // aprovado em tudo!!
}