/**
 * ted.c - Programa principal do sistema TED
 *
 * Uso: ted [-e path] -f arq.geo [-q cons.qry] -o dir [-pm arq.pm]
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "args.h"
#include "hashfile.h"
#include "quadra.h"
#include "pessoa.h"
#include "parser.h"
#include "svg.h"
#include "cidade.h"

int main(int argc, char *argv[]) {
    TedArgs args;
    if (args_parse(argc, argv, &args) != 0) {
        fprintf(stderr,
                "Uso: ted [-e path] -f arq.geo [-q qry] -o dir [-pm arq.pm]\n");
        return 1;
    }

    /* ── Monta caminhos ──────────────────────────────── */
    char geo_path[PATH_MAX_LEN + FILE_MAX_LEN + 4];
    args_build_path(args.base_entrada, args.arq_geo,
                    geo_path, sizeof(geo_path));

    char base_name[FILE_MAX_LEN];
    args_basename(args.arq_geo, base_name, sizeof(base_name));

    /* Hashfiles no diretório de saída */
    char hfq_path[1024];
    char hfp_path[1024];
    snprintf(hfq_path, sizeof(hfq_path), "%s/%s_quadras.hf",
             args.base_saida, base_name);
    snprintf(hfp_path, sizeof(hfp_path), "%s/%s_pessoas.hf",
             args.base_saida, base_name);

    /* SVG base (somente .geo) */
    char svg_base_path[1024];
    snprintf(svg_base_path, sizeof(svg_base_path), "%s/%s.svg",
             args.base_saida, base_name);

    /* ── Cria hashfiles ──────────────────────────────── */
    HashFile hfq = hf_create(hfq_path);
    HashFile hfp = hf_create(hfp_path);
    if (!hfq || !hfp) {
        fprintf(stderr, "Erro ao criar hashfiles em: %s\n", args.base_saida);
        return 1;
    }

    /* ── SVG base ────────────────────────────────────── */
    SvgFile svg_base = svg_open(svg_base_path, 800.0f, 600.0f);

    ColorCtx cctx;
    cctx.sw = 1.0f;
    strcpy(cctx.cfill, "orange");
    strcpy(cctx.cstrk, "black");

    /* ── Processa .geo ───────────────────────────────── */
    parse_geo(geo_path, hfq, svg_base, &cctx);
    svg_close(svg_base);

    /* ── Processa .pm ────────────────────────────────── */
    if (args.has_pm) {
        char pm_path[PATH_MAX_LEN + FILE_MAX_LEN + 4];
        args_build_path(args.base_entrada, args.arq_pm,
                        pm_path, sizeof(pm_path));
        parse_pm(pm_path, hfp, hfq);
    }

    /* ── Processa .qry ───────────────────────────────── */
    if (args.has_qry) {
        char qry_path[PATH_MAX_LEN + FILE_MAX_LEN + 4];
        args_build_path(args.base_entrada, args.arq_qry,
                        qry_path, sizeof(qry_path));

        char qry_name[FILE_MAX_LEN];
        args_basename(args.arq_qry, qry_name, sizeof(qry_name));

        char svg_qry_path[1024];
        char txt_qry_path[1024];
        snprintf(svg_qry_path, sizeof(svg_qry_path),
                 "%s/%s-%s.svg", args.base_saida, base_name, qry_name);
        snprintf(txt_qry_path, sizeof(txt_qry_path),
                 "%s/%s-%s.txt", args.base_saida, base_name, qry_name);

        SvgFile svg_qry = svg_open(svg_qry_path, 800.0f, 600.0f);
        FILE   *txt_qry = fopen(txt_qry_path, "w");

        CidadeCtx cidade;
        cidade_init(&cidade, hfq, hfp);

        /* Re-desenha quadras no SVG de consultas */
        if (svg_qry) {
            ColorCtx cctx2;
            cctx2.sw = 1.0f;
            strcpy(cctx2.cfill, "orange");
            strcpy(cctx2.cstrk, "black");
            parse_geo(geo_path, hfq, svg_qry, &cctx2);
        }

        parse_qry(qry_path, hfq, hfp, svg_qry, txt_qry, &cidade);

        if (svg_qry) svg_close(svg_qry);
        if (txt_qry) fclose(txt_qry);
    }

    /* ── Dumps .hfd ──────────────────────────────────── */
    char hfqd[1024], hfpd[1024];
    snprintf(hfqd, sizeof(hfqd), "%s/%s_quadras.hfd",
             args.base_saida, base_name);
    snprintf(hfpd, sizeof(hfpd), "%s/%s_pessoas.hfd",
             args.base_saida, base_name);
    hf_dump(hfq, hfqd);
    hf_dump(hfp, hfpd);

    hf_close(hfq);
    hf_close(hfp);

    printf("TED concluido. Saida em: %s\n", args.base_saida);
    return 0;
}
