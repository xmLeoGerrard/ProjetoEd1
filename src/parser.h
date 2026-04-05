/**
 * parser.h
 * Interface para parsing dos arquivos .geo, .pm e .qry.
 */
#ifndef PARSER_H
#define PARSER_H

#include "hashfile.h"
#include "quadra.h"
#include "pessoa.h"
#include "svg.h"
#include "cidade.h"

/* Contexto de cor corrente (alterado por comando cq) */
typedef struct {
    float sw;
    char  cfill[32];
    char  cstrk[32];
} ColorCtx;

/**
 * Processa arquivo .geo: insere quadras no hashfile e desenha SVG.
 * (Se svg==NULL não desenha; se hf_quadras já tiver a chave, ignora.)
 */
void parse_geo(const char *filepath, HashFile hf_quadras,
               SvgFile svg, ColorCtx *ctx);

/**
 * Processa arquivo .pm: insere pessoas e registra moradores.
 */
void parse_pm(const char *filepath, HashFile hf_pessoas,
              HashFile hf_quadras);

/**
 * Processa arquivo .qry: executa consultas/atualizações.
 */
void parse_qry(const char *filepath, HashFile hf_quadras,
               HashFile hf_pessoas, SvgFile svg_qry,
               FILE *txt_out, CidadeCtx *cidade);

#endif /* PARSER_H */
