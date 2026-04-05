/**
 * args.c - Tratamento de argumentos de linha de comando
 */
#include "args.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

int args_parse(int argc, char *argv[], TedArgs *out) {
    assert(out);
    memset(out, 0, sizeof(TedArgs));
    /* Diretório de entrada padrão = corrente */
    strcpy(out->base_entrada, ".");

    int i = 1;
    while (i < argc) {
        if (strcmp(argv[i], "-e") == 0 && i + 1 < argc) {
            strncpy(out->base_entrada, argv[++i],
                    sizeof(out->base_entrada) - 1);
            /* Remove barra final */
            int len = (int)strlen(out->base_entrada);
            if (len > 1 && out->base_entrada[len - 1] == '/')
                out->base_entrada[len - 1] = '\0';
        } else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
            strncpy(out->arq_geo, argv[++i], sizeof(out->arq_geo) - 1);
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            strncpy(out->base_saida, argv[++i],
                    sizeof(out->base_saida) - 1);
            int len = (int)strlen(out->base_saida);
            if (len > 1 && out->base_saida[len - 1] == '/')
                out->base_saida[len - 1] = '\0';
        } else if (strcmp(argv[i], "-q") == 0 && i + 1 < argc) {
            strncpy(out->arq_qry, argv[++i], sizeof(out->arq_qry) - 1);
            out->has_qry = 1;
        } else if (strcmp(argv[i], "-pm") == 0 && i + 1 < argc) {
            strncpy(out->arq_pm, argv[++i], sizeof(out->arq_pm) - 1);
            out->has_pm = 1;
        }
        i++;
    }

    /* Parâmetros obrigatórios */
    if (out->arq_geo[0] == '\0' || out->base_saida[0] == '\0')
        return -1;
    return 0;
}

void args_build_path(const char *base_dir, const char *filename,
                     char *dest, int dest_len) {
    assert(base_dir && filename && dest && dest_len > 0);
    if (filename[0] == '/' || filename[0] == '.') {
        /* Caminho já absoluto ou relativo explícito */
        snprintf(dest, (size_t)dest_len, "%s", filename);
    } else {
        snprintf(dest, (size_t)dest_len, "%s/%s", base_dir, filename);
    }
}

void args_basename(const char *filepath, char *dest, int dest_len) {
    assert(filepath && dest && dest_len > 0);
    /* Pula diretórios */
    const char *p = filepath;
    const char *last_slash = NULL;
    for (const char *c = filepath; *c; c++)
        if (*c == '/') last_slash = c;
    if (last_slash) p = last_slash + 1;

    strncpy(dest, p, (size_t)(dest_len - 1));
    dest[dest_len - 1] = '\0';

    /* Remove extensão */
    char *dot = strrchr(dest, '.');
    if (dot) *dot = '\0';
}
