# Algoritmos e Estruturas de Dados — Projeto Prático

![C](https://img.shields.io/badge/Language-C-blue.svg)
![Build](https://img.shields.io/badge/Build-Makefile-success.svg)
![Course](https://img.shields.io/badge/Course-SCC0202--ICMC--USP-red.svg)

Sistema modular e reutilizável em C para **carregamento, busca por similaridade e organização de dados** em larga escala utilizando diversos Tipos Abstratos de Dados (TADs).

Trabalho prático desenvolvido para a disciplina **SCC0202 — Algoritmos e Estruturas de Dados I** do ICMC/USP.

---

## 📑 Sumário

- [Visão Geral](#-visão-geral)
- [Funcionalidades Principais](#-funcionalidades-principais)
- [Estruturas de Dados Suportadas](#-estruturas-de-dados-suportadas)
- [Arquitetura do Projeto](#-arquitetura-do-projeto)
- [Compilação e Execução](#-compilação-e-execução)
- [Sintaxe das Consultas (-query)](#-sintaxe-das-consultas--query)
- [Métricas e Saída](#-métricas-e-saída)
- [Integrantes do Grupo](#-integrantes-do-grupo)

---

## Visão Geral

O projeto consiste em um motor de busca flexível capaz de processar datasets heterogêneos sem duplicar a lógica das estruturas subjacentes. 

O sistema é dividido em duas partes operacionais:
1. **Parte 1 — Asteroides (NASA):** Busca e classificação de aproximadores celestes com base em risco, velocidade, magnitude e distância.
2. **Parte 2 — Dados Abertos:** Reutilização total dos TADs para processamento de datasets governamentais públicos via customização de *readers*.

---

## Funcionalidades Principais

- **Desacoplamento Total:** As estruturas de dados não conhecem a estrutura interna do `ITEM`, garantindo reaproveitamento de código em 100%.
- **Motor de Busca e Similaridade Ponderada:**
  - Filtragem lógica relacional (`==`, `!=`, `<`, `<=`, `>`, `>=`).
  - Cálculo de similaridade individual por tolerância:
    $$s(x, q, t) = \max\left(0, 1 - \frac{|x - q|}{t}\right)$$
  - Agregação por média ponderada e limiar de aceitação (`ACCEPT`).
- **Gestão Eficiente de Memória:** Alocação dinâmica sem vazamentos (*zero memory leaks*), tratada para falhas e verificada com AddressSanitizer/LeakSanitizer.

---

## Estruturas de Dados Suportadas

O executável aceita a especificação das estruturas de entrada e saída via linha de comando:

| Identificador | Descrição |
| :--- | :--- |
| `pilha` | Estrutura LIFO (Last-In, First-Out) |
| `fila` | Estrutura FIFO (First-In, First-Out) |
| `lista_seq` | Lista contígua estática/dinâmica sequencial |
| `lista_enc` | Lista simplesmente encadeada por nós dinâmicos |
| `lista_cabeca` | Lista encadeada com nó sentinela/cabeça real |
| `lista_ord` | Lista mantida estritamente ordenada durante a inserção |
| `lista_gen` | Lista generalizada para agrupamento em níveis (níveis dinâmicos/subgrupos) |
| `lista_cruz` | Lista cruzada com ponteiros multidimensionais sem duplicação de `ITEM` |

---

## Arquitetura do Projeto

```text
├── main.c             # Leitura de argumentos da CLI e orquestração
├── include/           # Arquivos de cabeçalho (.h)
│   ├── Item.h         # TAD ITEM genérico (linha + campos tipados)
│   ├── Colecao.h      # Fachada que seleciona a estrutura em tempo de execução
│   ├── busca.h        # Motor de filtros e similaridade
│   ├── consulta.h     # Parser do arquivo de consulta
│   ├── leitura.h      # Leitores (nasa, lattes, generico)
│   └── ...            # Headers de Pilha, Fila e das listas
├── src/               # Implementações em C (.c), um módulo por TAD
├── dados/             # CSVs de entrada
├── consultas/         # Consultas de exemplo (.txt)
├── Makefile           # `make` gera o executável `trabalho`
└── README.md
```

---

## Compilação e Execução

### **Pré-requisitos**
- Compilador GCC com suporte às flags `-Wall -Wextra -pedantic -std=c99`.
- Utilitário `make`.

### **Compilando o Projeto**
```bash
make
```

### **Padrão de Execução**
O executável gerado se chama `trabalho` e aceita os seguintes argumentos formais:

```bash
./trabalho \
  --reader nasa \
  --input dados/asteroid_dataset_20251019.csv \
  --base lista_enc \
  --result lista_ord \
  --query consultas/q1.txt \
  --output resultado.csv \
  --top-k 100 \
  --limit 10000
```

### **Descrição dos Argumentos**
- `--reader`: Leitor do dataset (`nasa`, `lattes` ou `generico`).
- `--input`: Caminho para o CSV de entrada.
- `--base`: TAD utilizada para armazenar a base completa.
- `--result`: TAD utilizada para armazenar os resultados da busca.
- `--query`: Caminho do arquivo com as regras de busca.
- `--output`: Caminho para salvar os primeiros `--top-k` resultados em CSV.
- `--top-k`: Quantidade máxima de registros a exportar (`0` para todos).
- `--limit`: Limite de registros a carregar da base (`0` para a base inteira).
- `--axis` *(opcional, padrão `0`)*: com `--result lista_cruz`, escolhe o eixo de percurso na saída (`0` = primeiro campo do `CROSS`, `1` = segundo). Ignorado nas demais estruturas.

Se a consulta não tiver `GROUP`/`CROSS`, `lista_gen` agrupa pelo primeiro campo e `lista_cruz` usa as duas primeiras colunas como eixos (tanto na base quanto no resultado).

---

## Sintaxe das Consultas (`--query`)

O arquivo de consulta texto suporta os seguintes comandos por linha:

```text
FILTER|is_future_event|==|true
FILTER|distance_au|<=|0.08
SIM|velocity_km_s|15.0|10.0|1.0
SIM|risk_score|0.70|0.30|2.0
ACCEPT|0.60
ORDER|similarity|DESC
GROUP|threat_category|year
CROSS|threat_category|year
```

Campos separados por `|`; linhas vazias e iniciadas por `#` são ignoradas. Exemplos prontos em `consultas/`
(`q1`–`q3` para asteroides e `lattes1`–`lattes2` para o dataset da Parte 2).

---

## Métricas e Saída

Durante a execução, o programa imprime relatórios de status e benchmarks na saída padrão:

```text
STATUS=OK
BASE=lista_enc
RESULT=lista_ord
INPUT_RECORDS=10000
MATCHED_RECORDS=487
OUTPUT_RECORDS=100
LOAD_MS=12.431
SEARCH_MS=3.288
RESULT_INSERT_MS=0.812
WRITE_MS=1.050
TOTAL_MS=17.900
```

Em caso de falha o programa imprime `STATUS=ERROR` e `MESSAGE=<motivo>` (o motivo também vai para o `stderr`) e retorna código 1.

---

## Integrantes do Grupo

- **[Guilherme Souza de Oliveira Santos]**
- **[Maria Rita Fagundes Vargas]**
- **[Luís Henrique Varela Medeiros Bezerra]**

---
*SCC0202 — Algoritmos e Estruturas de Dados I (2026)*  
*Instituto de Ciências Matemáticas e de Computação — ICMC / USP*