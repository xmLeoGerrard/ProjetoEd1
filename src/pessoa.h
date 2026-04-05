/**
 * pessoa.h
 * Interface para entidades Pessoa e Morador do sistema TED.
 * Chave: cpf (string)
 */

#ifndef PESSOA_H
#define PESSOA_H

#define CPF_SIZE    20
#define NOME_SIZE   50
#define DATE_SIZE   12
#define FACE_CHARS  "NSLO"

typedef enum { SEXO_M = 'M', SEXO_F = 'F' } Sexo;

/* ── Endereço ────────────────────────────────────────── */
typedef struct {
    char  cep[32];
    char  face;       /* N, S, L, O                  */
    int   num;
    char  compl[32];
    int   has_addr;   /* 1=morador, 0=sem-teto        */
} Endereco;

/* ── Registro de Pessoa ──────────────────────────────── */
typedef struct {
    char     cpf[CPF_SIZE];
    char     nome[NOME_SIZE];
    char     sobrenome[NOME_SIZE];
    char     sexo;           /* 'M' ou 'F'            */
    char     nasc[DATE_SIZE]; /* dd/mm/aaaa            */
    Endereco end;
} Pessoa;

/* ── Serialização ────────────────────────────────────── */
void pessoa_serialize(const Pessoa *p, char *buf, int buflen);
void pessoa_deserialize(Pessoa *p, const char *buf);
void pessoa_key(const Pessoa *p, char *buf, int buflen);

/* Verifica se pessoa é morador */
int pessoa_is_morador(const Pessoa *p);

/* Formata endereço como "cep/face/num compl" */
void pessoa_fmt_endereco(const Pessoa *p, char *buf, int buflen);

#endif /* PESSOA_H */
