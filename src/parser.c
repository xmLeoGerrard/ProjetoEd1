/**
 * parser.c - Leitura dos arquivos .geo, .pm e .qry
 */
#include "parser.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

/* ── .geo ─────────────────────────────────────────────── */
void parse_geo(const char *filepath, HashFile hf_quadras,
               SvgFile svg, ColorCtx *ctx) {
    assert(filepath && hf_quadras && ctx);

    FILE *f = fopen(filepath, "r");
    if (!f) {
        fprintf(stderr, "parse_geo: nao abriu '%s'\n", filepath);
        return;
    }

    char cmd[16];
    while (fscanf(f, "%15s", cmd) == 1) {
        if (strcmp(cmd, "q") == 0) {
            Quadra q;
            memset(&q, 0, sizeof(q));
            if (fscanf(f, "%31s %f %f %f %f",
                       q.cep, &q.x, &q.y, &q.w, &q.h) != 5)
                continue;
            q.sw = ctx->sw;
            strncpy(q.cfill, ctx->cfill, sizeof(q.cfill) - 1);
            strncpy(q.cstrk, ctx->cstrk, sizeof(q.cstrk) - 1);

            char key[HF_KEY_SIZE], ser[HF_DATA_SIZE];
            quadra_key(&q, key, sizeof(key));
            quadra_serialize(&q, ser, sizeof(ser));
            hf_insert(hf_quadras, key, ser); /* -1 se duplicata: ok */

            if (svg) {
                svg_rect(svg, q.x, q.y, q.w, q.h,
                         q.cfill, q.cstrk, q.sw);
                /* CEP centralizado */
                svg_text(svg,
                         q.x + q.w / 2.0f - 5.0f,
                         q.y + q.h / 2.0f,
                         q.cep, q.cstrk, 6.0f);
            }

        } else if (strcmp(cmd, "cq") == 0) {
            /* Suporta "1.5" ou "1.5px" como stroke-width */
            char sw_tok[32];
            fscanf(f, "%31s %31s %31s",
                   sw_tok, ctx->cfill, ctx->cstrk);
            sscanf(sw_tok, "%f", &ctx->sw);
        } else {
            /* ignora linha desconhecida */
            char buf[512];
            if (!fgets(buf, sizeof(buf), f)) break;
        }
    }
    fclose(f);
}

/* ── .pm ──────────────────────────────────────────────── */
void parse_pm(const char *filepath, HashFile hf_pessoas,
              HashFile hf_quadras) {
    assert(filepath && hf_pessoas);
    (void)hf_quadras;

    FILE *f = fopen(filepath, "r");
    if (!f) {
        fprintf(stderr, "parse_pm: nao abriu '%s'\n", filepath);
        return;
    }

    char cmd[16];
    while (fscanf(f, "%15s", cmd) == 1) {
        if (strcmp(cmd, "p") == 0) {
            Pessoa p;
            memset(&p, 0, sizeof(p));
            char sexo_s[4];
            if (fscanf(f, "%19s %49s %49s %3s %11s",
                       p.cpf, p.nome, p.sobrenome, sexo_s, p.nasc) != 5)
                continue;
            p.sexo         = sexo_s[0];
            p.end.has_addr = 0;

            char key[HF_KEY_SIZE], ser[HF_DATA_SIZE];
            pessoa_key(&p, key, sizeof(key));
            pessoa_serialize(&p, ser, sizeof(ser));
            hf_insert(hf_pessoas, key, ser);

        } else if (strcmp(cmd, "m") == 0) {
            char cpf[CPF_SIZE], cep[32], face_s[4], compl[32];
            int  num;
            if (fscanf(f, "%19s %31s %3s %d %31s",
                       cpf, cep, face_s, &num, compl) != 5)
                continue;

            HFRecord rec;
            if (hf_search(hf_pessoas, cpf, &rec) == 0) {
                Pessoa p;
                pessoa_deserialize(&p, rec.data);
                strncpy(p.end.cep,   cep,   sizeof(p.end.cep) - 1);
                /* Suporta face simples 'N' e formato "Face.N" */
                { char *dot = strrchr(face_s, '.'); p.end.face = dot ? dot[1] : face_s[0]; }
                p.end.num      = num;
                strncpy(p.end.compl, compl, sizeof(p.end.compl) - 1);
                p.end.has_addr = 1;

                char ser[HF_DATA_SIZE];
                pessoa_serialize(&p, ser, sizeof(ser));
                hf_update(hf_pessoas, cpf, ser);
            }
        } else {
            char buf[512];
            if (!fgets(buf, sizeof(buf), f)) break;
        }
    }
    fclose(f);
}

/* ── .qry ─────────────────────────────────────────────── */
void parse_qry(const char *filepath, HashFile hf_quadras,
               HashFile hf_pessoas, SvgFile svg_qry,
               FILE *txt_out, CidadeCtx *cidade) {
    assert(filepath && hf_quadras && hf_pessoas && cidade);

    FILE *f = fopen(filepath, "r");
    if (!f) {
        fprintf(stderr, "parse_qry: nao abriu '%s'\n", filepath);
        return;
    }

    char line[512];
    while (fgets(line, sizeof(line), f)) {
        /* Remove newline */
        char *nl = strchr(line, '\n');
        if (nl) *nl = '\0';
        if (line[0] == '\0' || line[0] == '#') continue;

        if (txt_out) fprintf(txt_out, "[*] %s\n", line);

        char cmd[32];
        if (sscanf(line, "%31s", cmd) != 1) continue;

        if (strcmp(cmd, "rq") == 0) {
            char cep[CEP_SIZE];
            if (sscanf(line, "%*s %31s", cep) == 1)
                cidade_rq(cidade, cep, svg_qry, txt_out);

        } else if (strcmp(cmd, "pq") == 0) {
            char cep[CEP_SIZE];
            if (sscanf(line, "%*s %31s", cep) == 1)
                cidade_pq(cidade, cep, svg_qry, txt_out);

        } else if (strcmp(cmd, "censo") == 0) {
            cidade_censo(cidade, txt_out);

        } else if (strcmp(cmd, "h?") == 0) {
            char cpf[CPF_SIZE];
            if (sscanf(line, "%*s %19s", cpf) == 1)
                cidade_h_query(cidade, cpf, txt_out);

        } else if (strcmp(cmd, "nasc") == 0) {
            char cpf[CPF_SIZE], nome[50], sob[50], sexo_s[4], nasc[12];
            if (sscanf(line, "%*s %19s %49s %49s %3s %11s",
                       cpf, nome, sob, sexo_s, nasc) == 5)
                cidade_nasc(cidade, cpf, nome, sob, sexo_s[0], nasc);

        } else if (strcmp(cmd, "rip") == 0) {
            char cpf[CPF_SIZE];
            if (sscanf(line, "%*s %19s", cpf) == 1)
                cidade_rip(cidade, cpf, svg_qry, txt_out);

        } else if (strcmp(cmd, "mud") == 0) {
            char cpf[CPF_SIZE], cep[32], face_s[32], compl[32];
            int num;
            if (sscanf(line, "%*s %19s %31s %31s %d %31s",
                       cpf, cep, face_s, &num, compl) == 5)
                { char *dot = strrchr(face_s, '.'); char fc = dot ? dot[1] : face_s[0];
                  cidade_mud(cidade, cpf, cep, fc, num, compl, svg_qry); }

        } else if (strcmp(cmd, "dspj") == 0) {
            char cpf[CPF_SIZE];
            if (sscanf(line, "%*s %19s", cpf) == 1)
                cidade_dspj(cidade, cpf, svg_qry, txt_out);
        }
    }
    fclose(f);
}
