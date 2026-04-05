/**
 * svg.h
 * Interface para geração de arquivos SVG.
 */

#ifndef SVG_H
#define SVG_H

#include <stdio.h>

typedef struct SvgFileStruct * SvgFile;

/* Abre arquivo SVG para escrita */
SvgFile svg_open(const char *path, float width, float height);

/* Fecha e finaliza o SVG */
void svg_close(SvgFile f);

/* Retângulo */
void svg_rect(SvgFile f, float x, float y, float w, float h,
              const char *fill, const char *stroke, float sw);

/* Texto */
void svg_text(SvgFile f, float x, float y, const char *text,
              const char *fill, float font_size);

/* Pequeno X vermelho (remoção de quadra) */
void svg_cross_red(SvgFile f, float x, float y, float size);

/* Pequena cruz vermelha (óbito) */
void svg_dagger(SvgFile f, float x, float y, float size);

/* Pequeno quadrado vermelho com texto (mudança) */
void svg_move_marker(SvgFile f, float x, float y,
                     const char *label, float size);

/* Pequeno círculo preto (despejo) */
void svg_circle_black(SvgFile f, float x, float y, float r);

/* Número de moradores por face */
void svg_face_count(SvgFile f, float x, float y, int count,
                    const char *fill);

#endif /* SVG_H */
