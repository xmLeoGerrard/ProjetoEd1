/**
 * cidade.h
 * Interface para operações de negócio sobre a cidade (consultas .qry).
 */
#ifndef CIDADE_H
#define CIDADE_H

#include <stdio.h>
#include "hashfile.h"
#include "quadra.h"
#include "pessoa.h"
#include "svg.h"

/* Contexto da cidade */
typedef struct {
    HashFile hf_quadras;
    HashFile hf_pessoas;
} CidadeCtx;

/* Contexto interno para rq (iteração sobre moradores da quadra removida) */
typedef struct {
    const char *cep;
    FILE       *txt;
    HashFile    hfp;
} RqCtx;

/* Contexto interno para pq (contagem de moradores por face) */
typedef struct {
    const char *cep;
    int         counts[4]; /* N=0 S=1 L=2 O=3 */
} PqCtx;

/* Contexto interno para censo */
typedef struct {
    int total_hab, total_mor, total_sem;
    int hom, mul, hom_mor, mul_mor, hom_sem, mul_sem;
} CensoCtx;

void cidade_init(CidadeCtx *ctx, HashFile hfq, HashFile hfp);

/* rq: remove quadra; moradores viram sem-teto */
void cidade_rq(CidadeCtx *ctx, const char *cep, SvgFile svg, FILE *txt);

/* pq: conta moradores por face e total */
void cidade_pq(CidadeCtx *ctx, const char *cep, SvgFile svg, FILE *txt);

/* censo: estatísticas completas */
void cidade_censo(CidadeCtx *ctx, FILE *txt);

/* h?: dados de uma pessoa */
void cidade_h_query(CidadeCtx *ctx, const char *cpf, FILE *txt);

/* nasc: inserir nova pessoa */
void cidade_nasc(CidadeCtx *ctx, const char *cpf, const char *nome,
                 const char *sobrenome, char sexo, const char *nasc);

/* rip: remover pessoa (falecimento) */
void cidade_rip(CidadeCtx *ctx, const char *cpf, SvgFile svg, FILE *txt);

/* mud: mudança de endereço */
void cidade_mud(CidadeCtx *ctx, const char *cpf,
                const char *cep, char face, int num,
                const char *compl, SvgFile svg);

/* dspj: despejo */
void cidade_dspj(CidadeCtx *ctx, const char *cpf, SvgFile svg, FILE *txt);

#endif /* CIDADE_H */
