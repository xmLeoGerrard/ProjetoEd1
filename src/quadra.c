/**
 * quadra.c - Implementação de Quadra
 */
#include "quadra.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

void quadra_serialize(const Quadra *q, char *buf, int buflen) {
    assert(q && buf && buflen > 0);
    snprintf(buf, (size_t)buflen,
             "%s|%.4f|%.4f|%.4f|%.4f|%.4f|%s|%s",
             q->cep, q->x, q->y, q->w, q->h, q->sw,
             q->cfill, q->cstrk);
}

void quadra_deserialize(Quadra *q, const char *buf) {
    assert(q && buf);
    memset(q, 0, sizeof(Quadra));
    sscanf(buf,
           "%31[^|]|%f|%f|%f|%f|%f|%31[^|]|%31s",
           q->cep, &q->x, &q->y, &q->w, &q->h, &q->sw,
           q->cfill, q->cstrk);
}

void quadra_key(const Quadra *q, char *buf, int buflen) {
    assert(q && buf && buflen > 0);
    strncpy(buf, q->cep, (size_t)(buflen - 1));
    buf[buflen - 1] = '\0';
}

void quadra_anchor(const Quadra *q, float *ax, float *ay) {
    assert(q && ax && ay);
    /* Ponto de ancoragem = canto sudeste = (x+w, y+h) */
    *ax = q->x + q->w;
    *ay = q->y + q->h;
}

void quadra_face_pos(const Quadra *q, char face, float *ox, float *oy) {
    assert(q && ox && oy);
    float cx = q->x + q->w / 2.0f;
    float cy = q->y + q->h / 2.0f;
    float mg = 5.0f;
    switch (face) {
    case 'N': *ox = cx;           *oy = q->y + mg;        break;
    case 'S': *ox = cx;           *oy = q->y + q->h - mg; break;
    case 'L': *ox = q->x + mg;    *oy = cy;               break;
    case 'O': *ox = q->x + q->w - mg; *oy = cy;           break;
    default:  *ox = cx;           *oy = cy;                break;
    }
}
