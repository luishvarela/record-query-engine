#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <time.h>

#include "Item.h"
#include "Colecao.h"
#include "busca.h"
#include "leitura.h"
#include "consulta.h"

typedef struct {
    const char *reader, *input, *base, *result, *query, *output;
    long top_k;
    long limit;
} ARGS;

static double agora_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1e6;
}

static bool ler_inteiro(const char *s, long *out) {
    char *fim;
    errno = 0;
    long v = strtol(s, &fim, 10);
    if (errno || fim == s || *fim != '\0' || v < 0 || v > INT_MAX) return false;
    *out = v;
    return true;
}

static bool ler_args(int argc, char **argv, ARGS *a) {
    memset(a, 0, sizeof *a);
    for (int i = 1; i < argc; i++) {
        char nome[32];
        const char *valor = NULL;
        const char *eq = strchr(argv[i], '=');
        if (eq) {
            size_t n = (size_t)(eq - argv[i]);
            if (n >= sizeof nome) n = sizeof nome - 1;
            memcpy(nome, argv[i], n);
            nome[n] = '\0';
            valor = eq + 1;
        } else {
            snprintf(nome, sizeof nome, "%s", argv[i]);
        }

        if (valor == NULL) {
            if (i + 1 >= argc) return false;
            valor = argv[++i];
        }

        if      (strcmp(nome, "--reader") == 0) a->reader = valor;
        else if (strcmp(nome, "--input")  == 0) a->input  = valor;
        else if (strcmp(nome, "--base")   == 0) a->base   = valor;
        else if (strcmp(nome, "--result") == 0) a->result = valor;
        else if (strcmp(nome, "--query")  == 0) a->query  = valor;
        else if (strcmp(nome, "--output") == 0) a->output = valor;
        else if (strcmp(nome, "--top-k") == 0 || strcmp(nome, "--top_k") == 0) {
            if (!ler_inteiro(valor, &a->top_k)) return false;
        }
        else if (strcmp(nome, "--limit") == 0) {
            if (!ler_inteiro(valor, &a->limit)) return false;
        }
        else return false;
    }
    return a->reader && a->input && a->base && a->result && a->query && a->output;
}

static int comparar(const ITEM *a, const ITEM *b, int campo, bool desc) {
    int r;
    if (campo == CAMPO_SIMILARIDADE) {
        double x = iGetSim(a), y = iGetSim(b);
        r = (x > y) - (x < y);
        return desc ? -r : r;
    }
    TIPO ta = iGetTipo(a, campo), tb = iGetTipo(b, campo);
    if (ta == AUSENTE || tb == AUSENTE) return (ta == AUSENTE) - (tb == AUSENTE);
    if (ta != tb) r = (ta == TEXTO) - (tb == TEXTO);
    else if (ta == NUMERO) {
        double x = iGetNumero(a, campo), y = iGetNumero(b, campo);
        r = (x > y) - (x < y);
    } else {
        r = strcmp(iGetTexto(a, campo), iGetTexto(b, campo));
        r = (r > 0) - (r < 0);
    }
    return desc ? -r : r;
}
static int cmp_asc (const ITEM *a, const ITEM *b, int c) { return comparar(a, b, c, false); }
static int cmp_desc(const ITEM *a, const ITEM *b, int c) { return comparar(a, b, c, true); }

static const int GRUPO_PADRAO[1] = { 0 };

static bool montar_config(const char *est, const PLANO_CONSULTA *p, int n_campos,
                          bool resultado, CONFIG *cfg) {
    memset(cfg, 0, sizeof *cfg);

    if (strcmp(est, "lista_ord") == 0) {
        if (p->tem_order && (resultado || p->campo_ordem != CAMPO_SIMILARIDADE)) {
            cfg->Comparar   = p->ordem_desc ? cmp_desc : cmp_asc;
            cfg->CampoOrdem = p->campo_ordem;
        }
    }
    else if (strcmp(est, "lista_gen") == 0) {
        if (p->n_grupo > 0) {
            cfg->CamposGrupo  = p->campos_grupo;
            cfg->nCamposGrupo = p->n_grupo;
        } else if (resultado) {
            return false;
        } else {
            cfg->CamposGrupo  = GRUPO_PADRAO;
            cfg->nCamposGrupo = 1;
        }
    }
    else if (strcmp(est, "lista_cruz") == 0) {
        if (p->tem_cross) {
            cfg->CampoCruzA = p->cross_a;
            cfg->CampoCruzB = p->cross_b;
        } else if (resultado) {
            return false;
        } else {
            cfg->CampoCruzA = 0;
            cfg->CampoCruzB = n_campos > 1 ? 1 : 0;
        }
    }
    return true;
}

typedef struct {
    const CONSULTA *consulta;
    COLECAO        *resultado;
    long            aceitos;
    double          ms_insercao;
    bool            erro;
} CTX_BUSCA;

static bool visita_busca(ITEM *item, void *ctx_) {
    CTX_BUSCA *ctx = ctx_;
    if (!avaliar_item(item, ctx->consulta)) return false;

    double t0 = agora_ms();
    bool ok = colInserir(ctx->resultado, item);
    ctx->ms_insercao += agora_ms() - t0;

    if (!ok) { ctx->erro = true; return true; }
    ctx->aceitos++;
    return false;
}

typedef struct {
    FILE       *arq;
    long        k;
    long        escritos;
    const char *eol;
    bool        erro;
} CTX_SAIDA;

static bool visita_saida(ITEM *item, void *ctx_) {
    CTX_SAIDA *ctx = ctx_;
    if (fputs(iGetLinha(item), ctx->arq) == EOF || fputs(ctx->eol, ctx->arq) == EOF) {
        ctx->erro = true;
        return true;
    }
    ctx->escritos++;
    return ctx->k > 0 && ctx->escritos >= ctx->k;
}

int main(int argc, char **argv) {
    ARGS args;
    ESQUEMA esquema;
    PLANO_CONSULTA plano;
    CONFIG cfg_base, cfg_res;
    COLECAO *base = NULL, *resultado = NULL;
    CTX_BUSCA busca;
    CTX_SAIDA saida;
    char erro_consulta[256];
    bool esquema_ok = false, plano_ok = false;
    int status = 1, lidos = 0;
    double t_ini = agora_ms(), t0;
    double load_ms = 0, search_ms = 0, insert_ms = 0, write_ms = 0;

    if (!ler_args(argc, argv, &args)) return 1;
    if (!leitor_valido(args.reader)) return 1;
    if (!colNomeValido(args.base) || !colNomeValido(args.result)) return 1;

    // 1) Abre o esquema do arquivo
    if (!esquema_abrir(&esquema, args.reader, args.input)) return 1;
    esquema_ok = true;

    // 2) Carrega o plano de consulta
    if (!consulta_carregar(args.query, &esquema, &plano, erro_consulta, sizeof erro_consulta)) {
        esquema_liberar(&esquema);
        return 1;
    }
    plano_ok = true;

    // 3) Configura as estruturas base e resultado
    if (!montar_config(args.base,   &plano, esquema.nCampos, false, &cfg_base) ||
        !montar_config(args.result, &plano, esquema.nCampos, true,  &cfg_res)) {
        consulta_liberar(&plano);
        esquema_liberar(&esquema);
        return 1;
    }

    // 4) Carrega o dataset
    t0 = agora_ms();
    base = carregar_dataset(args.input, args.base, &cfg_base, &esquema, (int)args.limit, &lidos);
    load_ms = agora_ms() - t0;
    if (base == NULL) {
        consulta_liberar(&plano);
        esquema_liberar(&esquema);
        return 1;
    }

    // 5) Cria o container de resultado e executa a busca
    resultado = colCriar(args.result, &cfg_res);
    if (resultado == NULL) {
        colApagar(&base, true);
        consulta_liberar(&plano);
        esquema_liberar(&esquema);
        return 1;
    }

    busca.consulta    = &plano.consulta;
    busca.resultado   = resultado;
    busca.aceitos     = 0;
    busca.ms_insercao = 0.0;
    busca.erro        = false;

    t0 = agora_ms();
    colPercorrer(base, visita_busca, &busca);
    search_ms = agora_ms() - t0;

    if (!busca.erro) {
        insert_ms = busca.ms_insercao;
        search_ms -= insert_ms;

        // 6) Escreve o arquivo de saída
        t0 = agora_ms();
        saida.arq      = fopen(args.output, "w");
        saida.k        = args.top_k;
        saida.escritos = 0;
        saida.eol      = esquema.crlf ? "\r\n" : "\n";
        saida.erro     = (saida.arq == NULL);

        if (!saida.erro) {
            if (fputs(esquema.cabecalho, saida.arq) == EOF || fputs(saida.eol, saida.arq) == EOF)
                saida.erro = true;
            if (!saida.erro) colPercorrer(resultado, visita_saida, &saida);
            if (fclose(saida.arq) != 0) saida.erro = true;
        }
        write_ms = agora_ms() - t0;

        if (!saida.erro) {
            printf("STATUS=OK\n");
            printf("BASE=%s\nRESULT=%s\n", args.base, args.result);
            printf("INPUT_RECORDS=%d\n", lidos);
            printf("MATCHED_RECORDS=%ld\n", busca.aceitos);
            printf("OUTPUT_RECORDS=%ld\n", saida.escritos);
            printf("LOAD_MS=%.3f\n", load_ms);
            printf("SEARCH_MS=%.3f\n", search_ms);
            printf("RESULT_INSERT_MS=%.3f\n", insert_ms);
            printf("WRITE_MS=%.3f\n", write_ms);
            printf("TOTAL_MS=%.3f\n", agora_ms() - t_ini);
            status = 0;
        }
    }

    // Liberação de memória ao finalizar
    colApagar(&resultado, false);
    colApagar(&base, true);
    consulta_liberar(&plano);
    esquema_liberar(&esquema);
    return status;
}