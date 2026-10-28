#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "Item.h"
#include "Colecao.h"
#include "leitura.h"

// Perfis de leitura suportados
typedef struct { const char *nome; char delimitador; } PERFIL;

static const PERFIL PERFIS[] = {
    { "nasa",     0   },
    { "lattes",   ';' },
    { "generico", 0   },
};
#define N_PERFIS (sizeof PERFIS / sizeof PERFIS[0])

// Procura o perfil de leitura pelo nome e retorna NULL quando ele nao existe.
static const PERFIL *buscar_perfil(const char *nome) {
    if (nome == NULL) return NULL;
    for (size_t i = 0; i < N_PERFIS; i++) {
        if (strcmp(PERFIS[i].nome, nome) == 0) return &PERFIS[i];
    }
    return NULL;
}

// Informa se o nome corresponde a um perfil suportado.
bool leitor_valido(const char *nome) { return buscar_perfil(nome) != NULL; }

// Retorna a lista textual dos nomes de leitores disponiveis.
const char *leitor_listar(void) { return "nasa, lattes, generico"; }

// Compara duas strings sem diferenciar letras maiusculas de minusculas.
bool texto_igual_ci(const char *a, const char *b) {
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return false;
        a++; b++;
    }
    return *a == *b;
}

// Aloca e retorna uma copia independente da string recebida.
static char *copiar(const char *s) {
    size_t n = strlen(s) + 1;
    char *p = malloc(n);
    if (p) memcpy(p, s, n);
    return p;
}

// Remove espacos das extremidades modificando a propria string.
static char *aparar(char *s) {
    while (isspace((unsigned char)*s)) s++;
    size_t n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n - 1])) s[--n] = '\0';
    return s;
}

// Classifica um campo como ausente, numero ou texto e grava o numero quando aplicavel.
TIPO classificar_valor(const char *txt, bool virgula_decimal, double *num) {
    while (isspace((unsigned char)*txt)) txt++;
    size_t len = strlen(txt);
    while (len > 0 && isspace((unsigned char)txt[len - 1])) len--;
    if (len == 0) return AUSENTE;

    // Normaliza marcadores comuns de ausencia e valores booleanos antes da conversao numerica.
    if (len < 8) {
        char w[8];
        memcpy(w, txt, len);
        w[len] = '\0';
        if (texto_igual_ci(w, "na") || texto_igual_ci(w, "n/a") || texto_igual_ci(w, "nan") ||
            texto_igual_ci(w, "null") || texto_igual_ci(w, "none")) return AUSENTE;
        if (texto_igual_ci(w, "true"))  { *num = 1.0; return NUMERO; }
        if (texto_igual_ci(w, "false")) { *num = 0.0; return NUMERO; }
    }

    if (len >= 64) return TEXTO;
    char tmp[64];
    memcpy(tmp, txt, len);
    tmp[len] = '\0';

    int digitos = 0, virgulas = 0;
    for (size_t i = 0; i < len; i++) {
        char c = tmp[i];
        if (isdigit((unsigned char)c)) digitos++;
        else if (c == ',' && virgula_decimal) { tmp[i] = '.'; virgulas++; }
        else if (c == '\0' || strchr("+-.eE", c) == NULL) return TEXTO;
    }
    if (digitos == 0 || virgulas > 1) return TEXTO;

    char *fim;
    double v = strtod(tmp, &fim);
    if (*fim != '\0' || !isfinite(v)) return TEXTO;
    *num = v;
    return NUMERO;
}

// Le uma linha completa, aumentando o buffer quando a linha nao cabe no espaco atual.
static long ler_linha(FILE *f, char **buf, size_t *cap) {
    if (*buf == NULL) {
        *cap = 1024;
        *buf = malloc(*cap);
        if (*buf == NULL) return -1;
    }
    size_t len = 0;
    (*buf)[0] = '\0';
    while (fgets(*buf + len, (int)(*cap - len), f)) {
        len += strlen(*buf + len);
        if (len > 0 && (*buf)[len - 1] == '\n') break;
        if (len + 1 >= *cap) {
            // A linha pode ultrapassar o tamanho inicial; expande o buffer sem descartar dados.
            char *nb = realloc(*buf, *cap * 2);
            if (nb == NULL) return -1;
            *buf = nb;
            *cap *= 2;
        }
    }
    return len > 0 ? (long)len : -1;
}

// Separa os campos no lugar, preservando delimitadores e aspas que pertencem a valores citados.
static int dividir(char *s, char delim, char **campos, int max) {
    int n = 0;
    char *p = s;
    while (1) {
        char *ini = p;
        if (*p == '"') {
            // Aspas duplas protegem delimitadores; duas aspas seguidas representam uma aspa literal.
            char *w = p, *r = p + 1;
            while (1) {
                if (*r == '"') {
                    if (r[1] == '"') { 
                        *w++ = '"'; 
                        r += 2; 
                    }
                    else { 
                        r++; 
                        break; 
                    }
                } 
                else if (*r == '\0') 
                    break;
                else 
                    *w++ = *r++;
            }
            *w = '\0';
            while (*r && *r != delim) 
                r++;
            if (campos && n < max) 
                campos[n] = ini;
            n++;
            if (*r == delim) { 
                p = r + 1; 
                continue; 
            }
            break;
        }
        char *q = p;
        while (*q && *q != delim) 
            q++;
        if (campos && n < max) 
            campos[n] = ini;
        n++;
        if (*q == delim) { 
            *q = '\0'; 
            p = q + 1; 
            continue; 
        }

        break;
    }
    return n;
}

// Escolhe o delimitador mais frequente no cabecalho, ignorando caracteres entre aspas.
static char detectar_delim(const char *cab) {
    const char cand[] = { ';', ',', '\t', '|' };
    int cont[4] = { 0, 0, 0, 0 };
    bool aspas = false;
    // Ignora delimitadores que aparecem dentro de valores entre aspas no cabecalho.
    for (const char *c = cab; *c; c++) {
        if (*c == '"') 
            aspas = !aspas;
        else if (!aspas) {
            for (int i = 0; i < 4; i++) {
                if (*c == cand[i]) 
                    cont[i]++;
            }
        }
    }
    int melhor = 1;
    for (int i = 0; i < 4; i++) {
        if (cont[i] > cont[melhor]) 
            melhor = i;
    }
    return cand[melhor];
}

// Libera os nomes das colunas e o cabecalho armazenados no esquema.
void esquema_liberar(ESQUEMA *e) {
    if (e == NULL) 
        return;
    for (int i = 0; i < e->nCampos && e->nomes; i++) 
        free(e->nomes[i]);
    free(e->nomes);
    free(e->cabecalho);
    memset(e, 0, sizeof *e);
}

// Abre o arquivo e inicializa os metadados do cabecalho e das colunas.
bool esquema_abrir(ESQUEMA *e, const char *reader, const char *pathArquivo) {
    memset(e, 0, sizeof *e);
    const PERFIL *perfil = buscar_perfil(reader);
    if (perfil == NULL) 
        return false;

    FILE *arq = fopen(pathArquivo, "r");
    if (arq == NULL) 
        return false;
    
    char *linha = NULL;
    size_t cap = 0;
    long len = ler_linha(arq, &linha, &cap);
    fclose(arq);
    if (len <= 0) { 
        free(linha); 
        return false; 
    }

    e->crlf = (len >= 2 && linha[len - 2] == '\r' && linha[len - 1] == '\n');
    linha[strcspn(linha, "\r\n")] = '\0';
    e->cabecalho = copiar(linha);

    // Mantem o BOM no cabecalho bruto, mas remove-o da primeira coluna usada internamente.
    const char *conteudo = linha;
    if ((unsigned char)linha[0] == 0xEF && (unsigned char)linha[1] == 0xBB &&
        (unsigned char)linha[2] == 0xBF)
        conteudo += 3;

    e->delimitador = perfil->delimitador ? perfil->delimitador : detectar_delim(conteudo);

    char *t1 = copiar(conteudo);
    char *t2 = copiar(conteudo);
    bool ok = (e->cabecalho && t1 && t2);
    if (ok) {
        int n = dividir(t1, e->delimitador, NULL, 0);
        char **campos = malloc((size_t)n * sizeof *campos);
        e->nomes = calloc((size_t)n, sizeof *e->nomes);
        if (campos && e->nomes) {
            dividir(t2, e->delimitador, campos, n);
            e->nCampos = n;
            for (int i = 0; i < n; i++) {
                e->nomes[i] = copiar(aparar(campos[i]));
                if (e->nomes[i] == NULL) 
                    ok = false;
            }
        } else {
            ok = false;
        }
        free(campos);
    }
    free(t1); 
    free(t2); 
    free(linha);

    if (!ok) 
        esquema_liberar(e);
    return ok;
}

// Localiza uma coluna por nome, tentando primeiro correspondencia exata e depois sem diferenciar maiuscula e minuscula.
int esquema_indice(const ESQUEMA *e, const char *nome) {
    for (int i = 0; i < e->nCampos; i++) {
        if (strcmp(e->nomes[i], nome) == 0) 
            return i;
    }
    for (int i = 0; i < e->nCampos; i++) {
        if (texto_igual_ci(e->nomes[i], nome)) 
            return i;
    }
    return -1;
}

// Le os registros do arquivo, converte seus campos e os insere na colecao escolhida.
COLECAO *carregar_dataset(const char *pathArquivo, const char *tipo_base,
                          const CONFIG *config, const ESQUEMA *e,
                          int limite, int *lidos) {

    if (lidos) *lidos = 0;
    if (e == NULL || e->nCampos <= 0) return NULL;

    FILE *arq = fopen(pathArquivo, "r");
    if (arq == NULL) 
        return NULL;

    COLECAO *base = colCriar(tipo_base, config);
    if (base == NULL) { 
        fclose(arq); 
        return NULL; 
    }

    int n = e->nCampos;
    // trab é uma copia mutavel: dividir() substitui delimitadores por NUL, enquanto linha
    // permanece intacta para que o ITEM preserve o registro original para exportacao.
    char  *linha = NULL, *trab = NULL;
    size_t cap = 0, cap_trab = 0;
    TIPO *tipos   = malloc((size_t)n * sizeof *tipos);
    double *numeros = malloc((size_t)n * sizeof *numeros);
    const char **textos  = malloc((size_t)n * sizeof *textos);
    char **campos  = malloc((size_t)n * sizeof *campos);
    bool erro = !(tipos && numeros && textos && campos);

    if (!erro && ler_linha(arq, &linha, &cap) < 0) erro = true;

    int dados = 0;
    while (!erro && (limite <= 0 || dados < limite) && ler_linha(arq, &linha, &cap) >= 0) {
        linha[strcspn(linha, "\r\n")] = '\0';
        if (linha[0] == '\0') 
            continue;

        size_t len = strlen(linha) + 1;
        if (len > cap_trab) {
            char *nt = realloc(trab, len);
            if (nt == NULL) { erro = true; break; }
            trab = nt;
            cap_trab = len;
        }
        memcpy(trab, linha, len);

        int achados = dividir(trab, e->delimitador, campos, n);
        for (int i = 0; i < n; i++) {
            tipos[i] = AUSENTE;
            numeros[i] = 0.0;
            textos[i] = NULL;

            if (i >= achados)  
                continue;
            char *campo = aparar(campos[i]);
            // Em arquivos com delimitador diferente de virgula, virgula pode ser decimal.
            tipos[i] = classificar_valor(campo, e->delimitador != ',', &numeros[i]);

            if (tipos[i] == TEXTO) 
                textos[i] = campo;
        }

        ITEM *item = iCriar(linha, n, tipos, numeros, textos);
        if (item == NULL) 
            continue;

        if (!colInserir(base, item)) { 
            iApagar(&item); 
            erro = true; 
            break; 
        }

        dados++;
    }

    fclose(arq);

    free(linha); 
    free(trab); 
    free(tipos); 
    free(numeros); 
    free(textos); 
    free(campos);

    if (erro) { 
        colApagar(&base, true); 
        return NULL; 
    }

    if (lidos) 
        *lidos = dados;
    return base;
}