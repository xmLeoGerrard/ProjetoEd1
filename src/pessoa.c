/**
 * pessoa.c - Implementação de Pessoa/Morador
 */
#include "pessoa.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

void pessoa_serialize(const Pessoa *p, char *buf, int buflen) {
    assert(p && buf && buflen > 0);
    snprintf(buf, (size_t)buflen,
             "%s|%s|%s|%c|%s|%d|%s|%c|%d|%s",
             p->cpf, p->nome, p->sobrenome,
             p->sexo, p->nasc,
             p->end.has_addr,
             p->end.cep,
             p->end.face ? p->end.face : '-',
             p->end.num,
             p->end.compl);
}

void pessoa_deserialize(Pessoa *p, const char *buf) {
    assert(p && buf);
    memset(p, 0, sizeof(Pessoa));
    char face_ch = '-';
    sscanf(buf,
           "%19[^|]|%49[^|]|%49[^|]|%c|%11[^|]|%d|%31[^|]|%c|%d|%31s",
           p->cpf, p->nome, p->sobrenome,
           &p->sexo, p->nasc,
           &p->end.has_addr,
           p->end.cep,
           &face_ch,
           &p->end.num,
           p->end.compl);
    p->end.face = (face_ch == '-') ? 0 : face_ch;
}

void pessoa_key(const Pessoa *p, char *buf, int buflen) {
    assert(p && buf && buflen > 0);
    strncpy(buf, p->cpf, (size_t)(buflen - 1));
    buf[buflen - 1] = '\0';
}

int pessoa_is_morador(const Pessoa *p) {
    assert(p);
    return p->end.has_addr;
}

void pessoa_fmt_endereco(const Pessoa *p, char *buf, int buflen) {
    assert(p && buf && buflen > 0);
    if (!p->end.has_addr) {
        snprintf(buf, (size_t)buflen, "(sem endereco)");
    } else {
        snprintf(buf, (size_t)buflen, "%s/%c/%d %s",
                 p->end.cep, p->end.face, p->end.num, p->end.compl);
    }
}
