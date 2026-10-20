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
- **Gestão Eficiente de Memória:** Alocação dinâmica sem vazamentos (*zero memory leaks*), tratada para falhas e testada via Valgrind.

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
├── include/           # Arquivos de cabeçalho (.h)
│   ├── Item.h         # Definição do tipo genérico ITEM e TAD de similaridade
│   ├── busca.h        # Motor de busca e estruturas de critérios/consultas
│   └── ...            # Headers dos TADs de listas, pilhas e filas
├── src/               # Implementações em C (.c)
│   ├── Item.c         # Gerenciamento dinâmico e comparações de itens
│   ├── busca.c        # Lógica de filtragem, sim e corte por accept
│   └── main.c         # Leitura de argumentos da CLI e orquestração
├── dados/             # Arquivos CSV de entrada (ex: asteroids.csv)
├── consultas/         # Arquivos de consulta .txt
├── Makefile           # Script de compilação automatizada
└── README.md          # Documentação do repositório
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
  --input dados/asteroids.csv \
  --base lista_enc \
  --result lista_ord \
  --query consultas/q1.txt \
  --output resultado.csv \
  --top-k 100 \
  --limit 10000
```

### **Descrição dos Argumentos**
- `--reader`: Especifica o leitor do dataset (`nasa`, etc.).
- `--input`: Caminho para o CSV de entrada.
- `--base`: TAD utilizada para armazenar a base completa.
- `--result`: TAD utilizada para armazenar os resultados da busca.
- `--query`: Caminho do arquivo com as regras de busca.
- `--output`: Caminho para salvar os primeiros `--top-k` resultados em CSV.
- `--top-k`: Quantidade máxima de registros a exportar (`0` para todos).
- `--limit`: Limite de registros a carregar da base (`0` para a base inteira).

---

## Sintaxe das Consultas (`--query`)

O arquivo de consulta texto suporta os seguintes comandos por linha:

```text
FILTER is_future_event|==|true
FILTER distance_au|<=|10.08
SIM velocity_km_s|15.0|10.0|1.0
SIM risk_score|0.70|0.30|2.0
ACCEPT 0.60
ORDER similarity|DESC
GROUP threat_category|year
CROSS threat_category|year
```

---

## Métricas e Saída

Durante a execução, o programa imprime relatórios de status e benchmarks na saída padrão:

```text
STATUS-OK
INPUT RECORDS=10000
MATCHED RECORDS=487
OUTPUT RECORDS=100
LOAD_MS=12.43
SEARCH_MS=3.288
RESULT_INSERT_MS=0.812
WRITE_MS=1.050
```

---

## Integrantes do Grupo

- **[Guilherme Souza de Oliveira Santos]**
- **[Maria Rita Fagundes Vargas]**
- **[Luís Henrique Varela Medeiros Bezerra]**

---
*SCC0202 — Algoritmos e Estruturas de Dados I (2026)*  
*Instituto de Ciências Matemáticas e de Computação — ICMC / USP*