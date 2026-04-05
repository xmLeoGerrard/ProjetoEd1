/**
 * cidade.c - Lógica de negócio / consultas sobre a cidade
 */
#include "cidade.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

void cidade_init(CidadeCtx *ctx, HashFile hfq, HashFile hfp) {
    assert(ctx);
    ctx->hf_quadras = hfq;
    ctx->hf_pessoas = hfp;
}

static int face_idx(char f) {
    switch(f) {
    case 'N': return 0;
    case 'S': return 1;
    case 'L': return 2;
    case 'O': return 3;
    default:  return -1;
    }
}

/* ── rq ──────────────────────────────────────────────── */

static void rq_cb(const HFRecord *rec, void *ud) {
    RqCtx *c = (RqCtx *)ud;
    Pessoa p;
    pessoa_deserialize(&p, rec->data);
    if (!p.end.has_addr) return;
    if (strncmp(p.end.cep, c->cep, CEP_SIZE) != 0) return;

    if (c->txt)
        fprintf(c->txt, "  CPF: %s  Nome: %s %s\n",
                p.cpf, p.nome, p.sobrenome);

    p.end.has_addr = 0;
    memset(p.end.cep,   0, sizeof(p.end.cep));
    p.end.face = 0;
    p.end.num  = 0;
    memset(p.end.compl, 0, sizeof(p.end.compl));

    char ser[HF_DATA_SIZE];
    pessoa_serialize(&p, ser, sizeof(ser));
    hf_update(c->hfp, p.cpf, ser);
}

void cidade_rq(CidadeCtx *ctx, const char *cep, SvgFile svg, FILE *txt) {
    assert(ctx && cep);
    HFRecord rec;
    if (hf_search(ctx->hf_quadras, cep, &rec) != 0) {
        if (txt) fprintf(txt, "[rq %s] Quadra nao encontrada.\n", cep);
        return;
    }
    Quadra q;
    quadra_deserialize(&q, rec.data);

    if (txt) fprintf(txt, "[rq %s] Moradores removidos:\n", cep);

    RqCtx rqc = { cep, txt, ctx->hf_pessoas };
    hf_iterate(ctx->hf_pessoas, rq_cb, &rqc);

    hf_remove(ctx->hf_quadras, cep);

    if (svg) {
        float ax, ay;
        quadra_anchor(&q, &ax, &ay);
        svg_cross_red(svg, ax, ay, 8.0f);
    }
}

/* ── pq ──────────────────────────────────────────────── */

static void pq_cb(const HFRecord *rec, void *ud) {
    PqCtx *c = (PqCtx *)ud;
    Pessoa p;
    pessoa_deserialize(&p, rec->data);
    if (!p.end.has_addr) return;
    if (strncmp(p.end.cep, c->cep, CEP_SIZE) != 0) return;
    int idx = face_idx(p.end.face);
    if (idx >= 0) c->counts[idx]++;
}

void cidade_pq(CidadeCtx *ctx, const char *cep, SvgFile svg, FILE *txt) {
    assert(ctx && cep);
    HFRecord rec;
    if (hf_search(ctx->hf_quadras, cep, &rec) != 0) {
        if (txt) fprintf(txt, "[pq %s] Quadra nao encontrada.\n", cep);
        return;
    }
    Quadra q;
    quadra_deserialize(&q, rec.data);

    PqCtx pqc;
    pqc.cep = cep;
    memset(pqc.counts, 0, sizeof(pqc.counts));
    hf_iterate(ctx->hf_pessoas, pq_cb, &pqc);

    int total = pqc.counts[0] + pqc.counts[1] +
                pqc.counts[2] + pqc.counts[3];

    if (txt) {
        fprintf(txt, "[pq %s] N=%d S=%d L=%d O=%d Total=%d\n",
                cep, pqc.counts[0], pqc.counts[1],
                pqc.counts[2], pqc.counts[3], total);
    }

    if (svg) {
        static const char faces[] = "NSLO";
        for (int i = 0; i < 4; i++) {
            float ox, oy;
            quadra_face_pos(&q, faces[i], &ox, &oy);
            svg_face_count(svg, ox, oy, pqc.counts[i], "blue");
        }
        float cx = q.x + q.w / 2.0f;
        float cy = q.y + q.h / 2.0f;
        char buf[16];
        snprintf(buf, sizeof(buf), "%d", total);
        svg_text(svg, cx, cy, buf, "black", 8.0f);
    }
}

/* ── censo ───────────────────────────────────────────── */

static void censo_cb(const HFRecord *rec, void *ud) {
    CensoCtx *c = (CensoCtx *)ud;
    Pessoa p;
    pessoa_deserialize(&p, rec->data);
    c->total_hab++;
    int is_m = (p.sexo == 'M');
    if (is_m) c->hom++; else c->mul++;

    if (p.end.has_addr) {
        c->total_mor++;
        if (is_m) c->hom_mor++; else c->mul_mor++;
    } else {
        c->total_sem++;
        if (is_m) c->hom_sem++; else c->mul_sem++;
    }
}

static float pct(int num, int den) {
    return (den > 0) ? (100.0f * (float)num / (float)den) : 0.0f;
}

void cidade_censo(CidadeCtx *ctx, FILE *txt) {
    assert(ctx && txt);
    CensoCtx c;
    memset(&c, 0, sizeof(c));
    hf_iterate(ctx->hf_pessoas, censo_cb, &c);

    fprintf(txt, "[censo]\n");
    fprintf(txt, "  Total habitantes    : %d\n",  c.total_hab);
    fprintf(txt, "  Total moradores     : %d\n",  c.total_mor);
    fprintf(txt, "  Proporcao mor/hab   : %.2f%%\n", pct(c.total_mor, c.total_hab));
    fprintf(txt, "  Homens              : %d\n",  c.hom);
    fprintf(txt, "  Mulheres            : %d\n",  c.mul);
    fprintf(txt, "  %% homens            : %.2f%%\n", pct(c.hom, c.total_hab));
    fprintf(txt, "  %% mulheres          : %.2f%%\n", pct(c.mul, c.total_hab));
    fprintf(txt, "  %% moradores homens  : %.2f%%\n", pct(c.hom_mor, c.total_mor));
    fprintf(txt, "  %% moradores mulheres: %.2f%%\n", pct(c.mul_mor, c.total_mor));
    fprintf(txt, "  Total sem-teto      : %d\n",  c.total_sem);
    fprintf(txt, "  %% sem-teto homens   : %.2f%%\n", pct(c.hom_sem, c.total_sem));
    fprintf(txt, "  %% sem-teto mulheres : %.2f%%\n", pct(c.mul_sem, c.total_sem));
}

/* ── h? ──────────────────────────────────────────────── */
void cidade_h_query(CidadeCtx *ctx, const char *cpf, FILE *txt) {
    assert(ctx && cpf && txt);
    HFRecord rec;
    if (hf_search(ctx->hf_pessoas, cpf, &rec) != 0) {
        fprintf(txt, "[h? %s] Pessoa nao encontrada.\n", cpf);
        return;
    }
    Pessoa p;
    pessoa_deserialize(&p, rec.data);
    fprintf(txt, "[h? %s]\n", cpf);
    fprintf(txt, "  Nome    : %s %s\n", p.nome, p.sobrenome);
    fprintf(txt, "  Sexo    : %c\n",    p.sexo);
    fprintf(txt, "  Nasc    : %s\n",    p.nasc);
    if (p.end.has_addr) {
        char ebuf[128];
        pessoa_fmt_endereco(&p, ebuf, sizeof(ebuf));
        fprintf(txt, "  Endereco: %s\n", ebuf);
    } else {
        fprintf(txt, "  Situacao: sem-teto\n");
    }
}

/* ── nasc ────────────────────────────────────────────── */
void cidade_nasc(CidadeCtx *ctx, const char *cpf, const char *nome,
                 const char *sobrenome, char sexo, const char *nasc) {
    assert(ctx && cpf && nome && sobrenome && nasc);
    Pessoa p;
    memset(&p, 0, sizeof(p));
    strncpy(p.cpf,       cpf,       sizeof(p.cpf) - 1);
    strncpy(p.nome,      nome,      sizeof(p.nome) - 1);
    strncpy(p.sobrenome, sobrenome, sizeof(p.sobrenome) - 1);
    p.sexo         = sexo;
    strncpy(p.nasc,      nasc,      sizeof(p.nasc) - 1);
    p.end.has_addr = 0;

    char key[HF_KEY_SIZE], ser[HF_DATA_SIZE];
    pessoa_key(&p, key, sizeof(key));
    pessoa_serialize(&p, ser, sizeof(ser));
    hf_insert(ctx->hf_pessoas, key, ser);
}

/* ── rip ─────────────────────────────────────────────── */
void cidade_rip(CidadeCtx *ctx, const char *cpf, SvgFile svg, FILE *txt) {
    assert(ctx && cpf);
    HFRecord rec;
    if (hf_search(ctx->hf_pessoas, cpf, &rec) != 0) {
        if (txt) fprintf(txt, "[rip %s] Pessoa nao encontrada.\n", cpf);
        return;
    }
    Pessoa p;
    pessoa_deserialize(&p, rec.data);

    if (txt) {
        fprintf(txt, "[rip %s] %s %s, Nasc:%s\n",
                cpf, p.nome, p.sobrenome, p.nasc);
        if (p.end.has_addr) {
            char ebuf[128];
            pessoa_fmt_endereco(&p, ebuf, sizeof(ebuf));
            fprintf(txt, "  Endereco: %s\n", ebuf);
        }
    }

    if (svg && p.end.has_addr) {
        HFRecord qrec;
        if (hf_search(ctx->hf_quadras, p.end.cep, &qrec) == 0) {
            Quadra q;
            quadra_deserialize(&q, qrec.data);
            float ox, oy;
            quadra_face_pos(&q, p.end.face, &ox, &oy);
            svg_dagger(svg, ox + (float)p.end.num * 0.1f, oy, 6.0f);
        }
    }
    hf_remove(ctx->hf_pessoas, cpf);
}

/* ── mud ─────────────────────────────────────────────── */
void cidade_mud(CidadeCtx *ctx, const char *cpf,
                const char *cep, char face, int num,
                const char *compl, SvgFile svg) {
    assert(ctx && cpf && cep);
    HFRecord rec;
    if (hf_search(ctx->hf_pessoas, cpf, &rec) != 0) return;

    Pessoa p;
    pessoa_deserialize(&p, rec.data);
    strncpy(p.end.cep,   cep,   sizeof(p.end.cep) - 1);
    p.end.face     = face;
    p.end.num      = num;
    if (compl) strncpy(p.end.compl, compl, sizeof(p.end.compl) - 1);
    p.end.has_addr = 1;

    char ser[HF_DATA_SIZE];
    pessoa_serialize(&p, ser, sizeof(ser));
    hf_update(ctx->hf_pessoas, cpf, ser);

    if (svg) {
        HFRecord qrec;
        if (hf_search(ctx->hf_quadras, cep, &qrec) == 0) {
            Quadra q;
            quadra_deserialize(&q, qrec.data);
            float ox, oy;
            quadra_face_pos(&q, face, &ox, &oy);
            svg_move_marker(svg, ox + (float)num * 0.1f, oy, cpf, 6.0f);
        }
    }
}

/* ── dspj ────────────────────────────────────────────── */
void cidade_dspj(CidadeCtx *ctx, const char *cpf, SvgFile svg, FILE *txt) {
    assert(ctx && cpf);
    HFRecord rec;
    if (hf_search(ctx->hf_pessoas, cpf, &rec) != 0) {
        if (txt) fprintf(txt, "[dspj %s] Pessoa nao encontrada.\n", cpf);
        return;
    }
    Pessoa p;
    pessoa_deserialize(&p, rec.data);

    if (txt) {
        fprintf(txt, "[dspj %s] %s %s\n", cpf, p.nome, p.sobrenome);
        if (p.end.has_addr) {
            char ebuf[128];
            pessoa_fmt_endereco(&p, ebuf, sizeof(ebuf));
            fprintf(txt, "  Despejado de: %s\n", ebuf);
        }
    }

    if (svg && p.end.has_addr) {
        HFRecord qrec;
        if (hf_search(ctx->hf_quadras, p.end.cep, &qrec) == 0) {
            Quadra q;
            quadra_deserialize(&q, qrec.data);
            float ox, oy;
            quadra_face_pos(&q, p.end.face, &ox, &oy);
            svg_circle_black(svg, ox + (float)p.end.num * 0.1f, oy, 4.0f);
        }
    }

    /* Vira sem-teto */
    p.end.has_addr = 0;
    memset(p.end.cep,   0, sizeof(p.end.cep));
    p.end.face = 0;
    p.end.num  = 0;
    memset(p.end.compl, 0, sizeof(p.end.compl));

    char ser[HF_DATA_SIZE];
    pessoa_serialize(&p, ser, sizeof(ser));
    hf_update(ctx->hf_pessoas, cpf, ser);
}
