/**
 * quadra.h
 * Interface para entidade Quadra (bloco urbano) do sistema TED.
 * Chave: cep (string alfanumérica)
 */

#ifndef QUADRA_H
#define QUADRA_H

#define CEP_SIZE    32
#define COLOR_SIZE  32

/* ── Registro de Quadra ──────────────────────────────── */
typedef struct {
    char  cep[CEP_SIZE];   /* identificador único      */
    float x, y;            /* coordenada âncora (SE)   */
    float w, h;            /* largura e altura         */
    float sw;              /* stroke-width             */
    char  cfill[COLOR_SIZE];
    char  cstrk[COLOR_SIZE];
} Quadra;

/* ── Serialização para hashfile ──────────────────────── */

/* Serializa Quadra em string (para HFRecord.data) */
void quadra_serialize(const Quadra *q, char *buf, int buflen);

/* Desserializa string em Quadra */
void quadra_deserialize(Quadra *q, const char *buf);

/* Copia cep para buf (para HFRecord.key) */
void quadra_key(const Quadra *q, char *buf, int buflen);

/* Calcula posição de uma face para SVG:
   face: 'N','S','L','O'
   Retorna (ox,oy) relativo ao SVG */
void quadra_face_pos(const Quadra *q, char face, float *ox, float *oy);

/* Ponto âncora (canto sudeste) */
void quadra_anchor(const Quadra *q, float *ax, float *ay);

#endif /* QUADRA_H */
