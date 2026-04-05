/**
 * args.h
 * Interface para tratamento dos argumentos de linha de comando do TED.
 */

#ifndef ARGS_H
#define ARGS_H

#define PATH_MAX_LEN 512
#define FILE_MAX_LEN 256

typedef struct {
    char base_entrada[PATH_MAX_LEN]; /* -e */
    char arq_geo[FILE_MAX_LEN];      /* -f */
    char base_saida[PATH_MAX_LEN];   /* -o */
    char arq_qry[FILE_MAX_LEN];      /* -q */
    char arq_pm[FILE_MAX_LEN];       /* -pm */
    int  has_qry;
    int  has_pm;
} TedArgs;

/**
 * Faz parse dos argumentos argc/argv.
 * @return 0 se OK, -1 se parâmetros obrigatórios ausentes
 */
int args_parse(int argc, char *argv[], TedArgs *out);

/**
 * Constrói caminho completo: base_dir + "/" + filename → dest
 */
void args_build_path(const char *base_dir, const char *filename,
                     char *dest, int dest_len);

/**
 * Extrai nome-base de um arquivo (sem extensão e sem diretório).
 * Ex: "/foo/bar/t001.geo" → "t001"
 */
void args_basename(const char *filepath, char *dest, int dest_len);

#endif /* ARGS_H */
