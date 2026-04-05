/**
 * svg.c - Geração de SVG
 */
#include "svg.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

struct SvgFileStruct {
    FILE *fp;
};

SvgFile svg_open(const char *path, float width, float height) {
    assert(path);
    struct SvgFileStruct *s = malloc(sizeof(struct SvgFileStruct));
    assert(s);
    s->fp = fopen(path, "w");
    if (!s->fp) { free(s); return NULL; }

    fprintf(s->fp,
            "<svg xmlns=\"http://www.w3.org/2000/svg\" "
            "xmlns:svg=\"http://www.w3.org/2000/svg\" version=\"1.1\" "
            "width=\"%.2f\" height=\"%.2f\">\n",
            width, height);
    return s;
}

void svg_close(SvgFile f) {
    if (!f) return;
    fprintf(f->fp, "</svg>\n");
    fclose(f->fp);
    free(f);
}

void svg_rect(SvgFile f, float x, float y, float w, float h,
              const char *fill, const char *stroke, float sw) {
    assert(f);
    fprintf(f->fp,
            "  <rect x=\"%.4f\" y=\"%.4f\" width=\"%.4f\" height=\"%.4f\" "
            "style=\"fill:%s;fill-opacity:0.5;stroke:%s;stroke-width:%.2f\"/>\n",
            x, y, w, h, fill, stroke, sw);
}

void svg_text(SvgFile f, float x, float y, const char *text,
              const char *fill, float font_size) {
    assert(f);
    fprintf(f->fp,
            "  <text x=\"%.4f\" y=\"%.4f\" "
            "style=\"font-size:%.1fpx;fill:%s\">%s</text>\n",
            x, y, font_size, fill, text);
}

void svg_cross_red(SvgFile f, float x, float y, float size) {
    assert(f);
    float h = size / 2.0f;
    /* Linha \ */
    fprintf(f->fp,
            "  <line x1=\"%.2f\" y1=\"%.2f\" x2=\"%.2f\" y2=\"%.2f\" "
            "style=\"stroke:red;stroke-width:2\"/>\n",
            x - h, y - h, x + h, y + h);
    /* Linha / */
    fprintf(f->fp,
            "  <line x1=\"%.2f\" y1=\"%.2f\" x2=\"%.2f\" y2=\"%.2f\" "
            "style=\"stroke:red;stroke-width:2\"/>\n",
            x + h, y - h, x - h, y + h);
}

void svg_dagger(SvgFile f, float x, float y, float size) {
    assert(f);
    float h = size / 2.0f;
    float q = size / 4.0f;
    /* Haste vertical */
    fprintf(f->fp,
            "  <line x1=\"%.2f\" y1=\"%.2f\" x2=\"%.2f\" y2=\"%.2f\" "
            "style=\"stroke:red;stroke-width:2\"/>\n",
            x, y - h, x, y + h);
    /* Braço horizontal */
    fprintf(f->fp,
            "  <line x1=\"%.2f\" y1=\"%.2f\" x2=\"%.2f\" y2=\"%.2f\" "
            "style=\"stroke:red;stroke-width:2\"/>\n",
            x - q, y - q, x + q, y - q);
}

void svg_move_marker(SvgFile f, float x, float y,
                     const char *label, float size) {
    assert(f);
    float h = size / 2.0f;
    fprintf(f->fp,
            "  <rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" "
            "style=\"fill:none;stroke:red;stroke-width:1.5\"/>\n",
            x - h, y - h, size, size);
    fprintf(f->fp,
            "  <text x=\"%.2f\" y=\"%.2f\" "
            "style=\"font-size:3px;fill:red\">%s</text>\n",
            x - h + 0.5f, y + 1.5f, label);
}

void svg_circle_black(SvgFile f, float x, float y, float r) {
    assert(f);
    fprintf(f->fp,
            "  <circle cx=\"%.2f\" cy=\"%.2f\" r=\"%.2f\" "
            "style=\"fill:black;stroke:black\"/>\n",
            x, y, r);
}

void svg_face_count(SvgFile f, float x, float y, int count,
                    const char *fill) {
    assert(f);
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", count);
    svg_text(f, x, y, buf, fill ? fill : "black", 6.0f);
}
