#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "consulta.h"

#define MAX_PARTES 32

static char *aparar(char *s) {
    while (isspace((unsigned char)*s)) 
        s++;
    size_t n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n - 1])) 
        s[--n] = '\0';
    return s;
}

static int dividir_pipe(char *linha, char **partes) {
    int n = 0;
    char *p = linha;
    while (n < MAX_PARTES) {
        char *barra = strchr(p, '|');
        if (barra) *barra = '\0';
        partes[n++] = aparar(p);
        if (!barra) break;
        p = barra + 1;
    }
    return n;
}

static bool numero_estrito(const char *s, double *v) {
    return classificar_valor(s, false, v) == NUMERO;
}

static bool parse_operador(const char *s, OPERADOR *op) {
    if      (strcmp(s, "==") == 0) *op = IGUAL;
    else if (strcmp(s, "!=") == 0) *op = DIFERENTE;
    else if (strcmp(s, "<")  == 0) *op = MENOR;
    else if (strcmp(s, "<=") == 0) *op = MENOR_IGUAL;
    else if (strcmp(s, ">")  == 0) *op = MAIOR;
    else if (strcmp(s, ">=") == 0) *op = MAIOR_IGUAL;
    else return false;
    return true;
}

static bool garantir(void **v, int n, int *cap, size_t tam) {
    if (n < *cap) return true;
    int nova = *cap ? *cap * 2 : 4;
    void *p = realloc(*v, (size_t)nova * tam);
    if (p == NULL) return false;
    *v = p;
    *cap = nova;
    return true;
}

void consulta_liberar(PLANO_CONSULTA *p) {
    if (p == NULL) return;
    free(p->consulta.filtros);
    free(p->consulta.sims);
    free(p->campos_grupo);
    memset(p, 0, sizeof *p);
}

bool consulta_carregar(const char *path, const ESQUEMA *esquema,
                       PLANO_CONSULTA *plano, char *erro, size_t tam_erro) {
    memset(plano, 0, sizeof *plano);
    FILE *arq = fopen(path, "r");
    if (arq == NULL) {
        snprintf(erro, tam_erro, "nao foi possivel abrir a consulta '%s'", path);
        return false;
    }

    int cap_f = 0, cap_s = 0, num_linha = 0;
    char buf[4096];
    bool sucesso = true;

    while (fgets(buf, sizeof buf, arq)) {
        num_linha++;
        if (strchr(buf, '\n') == NULL && !feof(arq)) {
            snprintf(erro, tam_erro, "linha %d: linha longa demais", num_linha);
            sucesso = false;
            break;
        }

        char *linha = buf;
        if (num_linha == 1 && (unsigned char)linha[0] == 0xEF &&
            (unsigned char)linha[1] == 0xBB && (unsigned char)linha[2] == 0xBF)
            linha += 3;
        linha = aparar(linha);
        if (*linha == '\0' || *linha == '#') continue;

        char *p[MAX_PARTES];
        int n = dividir_pipe(linha, p);
        const char *cmd = p[0];

        if (texto_igual_ci(cmd, "FILTER")) {
            if (n != 4) {
                snprintf(erro, tam_erro, "linha %d: FILTER exige campo|operador|valor", num_linha);
                sucesso = false; break;
            }
            int idx = esquema_indice(esquema, p[1]);
            if (idx < 0) {
                snprintf(erro, tam_erro, "linha %d: campo '%s' nao existe no cabecalho", num_linha, p[1]);
                sucesso = false; break;
            }
            if (!garantir((void **)&plano->consulta.filtros, plano->consulta.n_filtros, &cap_f, sizeof(CRITERIO_FILTER))) {
                snprintf(erro, tam_erro, "linha %d: memoria insuficiente", num_linha);
                sucesso = false; break;
            }

            CRITERIO_FILTER f;
            memset(&f, 0, sizeof f);
            f.campo_idx = idx;
            if (!parse_operador(p[2], &f.operador)) {
                snprintf(erro, tam_erro, "linha %d: operador '%s' invalido", num_linha, p[2]);
                sucesso = false; break;
            }

            TIPO t = classificar_valor(p[3], false, &f.valor_num);
            if (t == AUSENTE) {
                snprintf(erro, tam_erro, "linha %d: FILTER sem valor", num_linha);
                sucesso = false; break;
            }
            f.tipo_filtro = t;
            if (t == NUMERO && strlen(p[3]) < sizeof f.valor_texto)
                strcpy(f.valor_texto, p[3]);   // fallback p/ campos textuais (ex.: codigo "10")
            if (t == TEXTO) {
                char *v = p[3];
                size_t len = strlen(v);
                if (len >= 2 && v[0] == '"' && v[len - 1] == '"') { v[len - 1] = '\0'; v++; }
                if (strlen(v) >= sizeof f.valor_texto) {
                    snprintf(erro, tam_erro, "linha %d: valor de texto muito longo", num_linha);
                    sucesso = false; break;
                }
                strcpy(f.valor_texto, v);
                if (f.operador != IGUAL && f.operador != DIFERENTE) {
                    snprintf(erro, tam_erro, "linha %d: campos de texto so aceitam == e !=", num_linha);
                    sucesso = false; break;
                }
            }
            plano->consulta.filtros[plano->consulta.n_filtros++] = f;
        }
        else if (texto_igual_ci(cmd, "SIM")) {
            if (n != 5) {
                snprintf(erro, tam_erro, "linha %d: SIM exige campo|alvo|tolerancia|peso", num_linha);
                sucesso = false; break;
            }
            int idx = esquema_indice(esquema, p[1]);
            if (idx < 0) {
                snprintf(erro, tam_erro, "linha %d: campo '%s' nao existe no cabecalho", num_linha, p[1]);
                sucesso = false; break;
            }
            CRITERIO_SIM s;
            s.campo_idx = idx;
            if (!numero_estrito(p[2], &s.alvo) || !numero_estrito(p[3], &s.tolerancia) ||
                !numero_estrito(p[4], &s.peso)) {
                snprintf(erro, tam_erro, "linha %d: SIM com valor nao numerico", num_linha);
                sucesso = false; break;
            }
            if (s.tolerancia <= 0.0) {
                snprintf(erro, tam_erro, "linha %d: tolerancia do SIM deve ser > 0", num_linha);
                sucesso = false; break;
            }
            if (s.peso < 0.0) {
                snprintf(erro, tam_erro, "linha %d: peso do SIM deve ser >= 0", num_linha);
                sucesso = false; break;
            }
            if (!garantir((void **)&plano->consulta.sims, plano->consulta.n_sims, &cap_s, sizeof(CRITERIO_SIM))) {
                snprintf(erro, tam_erro, "linha %d: memoria insuficiente", num_linha);
                sucesso = false; break;
            }
            plano->consulta.sims[plano->consulta.n_sims++] = s;
        }
        else if (texto_igual_ci(cmd, "ACCEPT")) {
            if (n != 2 || !numero_estrito(p[1], &plano->consulta.limiar_accept)) {
                snprintf(erro, tam_erro, "linha %d: ACCEPT exige um limiar numerico", num_linha);
                sucesso = false; break;
            }
        }
        else if (texto_igual_ci(cmd, "ORDER")) {
            if (n < 2 || n > 3) {
                snprintf(erro, tam_erro, "linha %d: ORDER exige campo|ASC ou DESC", num_linha);
                sucesso = false; break;
            }
            if (texto_igual_ci(p[1], "similarity") || texto_igual_ci(p[1], "similaridade"))
                plano->campo_ordem = CAMPO_SIMILARIDADE;
            else {
                plano->campo_ordem = esquema_indice(esquema, p[1]);
                if (plano->campo_ordem < 0) {
                    snprintf(erro, tam_erro, "linha %d: campo '%s' nao existe no cabecalho", num_linha, p[1]);
                    sucesso = false; break;
                }
            }
            plano->ordem_desc = false;
            if (n == 3) {
                if (texto_igual_ci(p[2], "DESC")) plano->ordem_desc = true;
                else if (!texto_igual_ci(p[2], "ASC")) {
                    snprintf(erro, tam_erro, "linha %d: direcao deve ser ASC ou DESC", num_linha);
                    sucesso = false; break;
                }
            }
            plano->tem_order = true;
        }
        else if (texto_igual_ci(cmd, "GROUP")) {
            if (n < 2) {
                snprintf(erro, tam_erro, "linha %d: GROUP exige ao menos um campo", num_linha);
                sucesso = false; break;
            }
            free(plano->campos_grupo);
            plano->campos_grupo = malloc((size_t)(n - 1) * sizeof(int));
            plano->n_grupo = 0;
            if (plano->campos_grupo == NULL) {
                snprintf(erro, tam_erro, "linha %d: memoria insuficiente", num_linha);
                sucesso = false; break;
            }
            for (int i = 1; i < n; i++) {
                int idx = esquema_indice(esquema, p[i]);
                if (idx < 0) {
                    snprintf(erro, tam_erro, "linha %d: campo '%s' nao existe no cabecalho", num_linha, p[i]);
                    sucesso = false; break;
                }
                plano->campos_grupo[plano->n_grupo++] = idx;
            }
        }
        else if (texto_igual_ci(cmd, "CROSS")) {
            if (n != 3) {
                snprintf(erro, tam_erro, "linha %d: CROSS exige exatamente dois campos", num_linha);
                sucesso = false; break;
            }
            plano->cross_a = esquema_indice(esquema, p[1]);
            plano->cross_b = esquema_indice(esquema, p[2]);
            if (plano->cross_a < 0) {
                snprintf(erro, tam_erro, "linha %d: campo '%s' nao existe no cabecalho", num_linha, p[1]);
                sucesso = false; break;
            }
            if (plano->cross_b < 0) {
                snprintf(erro, tam_erro, "linha %d: campo '%s' nao existe no cabecalho", num_linha, p[2]);
                sucesso = false; break;
            }
            plano->tem_cross = true;
        }
        else {
            snprintf(erro, tam_erro, "linha %d: comando desconhecido '%s'", num_linha, cmd);
            sucesso = false; break;
        }
    }

    fclose(arq);
    if (!sucesso) {
        consulta_liberar(plano);
    }
    return sucesso;
}