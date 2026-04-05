/**
 * hashfile.h
 * Interface do Hashfile Dinâmico Extensível em disco (Hashing Extensível).
 *
 * Layout do arquivo binário .hf:
 *   [HFHeader]
 *   [Diretório: dir_size × sizeof(long)]
 *   [Bucket_0][Bucket_1]...[Bucket_N]
 *
 * Diretório: array de offsets; dir[i] = offset do bucket responsável
 * pelas chaves cujos global_depth bits baixos do hash == i.
 *
 * Split: local_depth < global_depth → só cria novo bucket.
 *        local_depth == global_depth → duplica o diretório primeiro.
 *
 * Overflow encadeado como fallback para colisões irredutíveis.
 * Ao final, gera arquivo .hfd com dump legível da estrutura.
 */
#ifndef HASHFILE_H
#define HASHFILE_H

#include <stdio.h>
#include <stdint.h>

/* ── Dimensões ──────────────────────────────────────────── */
#define HF_KEY_SIZE       36   /* bytes da chave (com '\0')     */
#define HF_DATA_SIZE     480   /* bytes do valor serializado    */
#define HF_BUCKET_CAPACITY 4   /* registros por bucket          */
#define HF_MAGIC       0x48465831u /* "HFX1"                   */
#define HF_OFFSET_NULL ((long)-1)  /* offset inválido/nulo      */

/* ── Registro ───────────────────────────────────────────── */
typedef struct {
    int  active;               /* 1=ocupado, 0=livre            */
    char key [HF_KEY_SIZE];
    char data[HF_DATA_SIZE];
} HFRecord;

/* ── Bucket persistido no .hf ───────────────────────────── */
typedef struct {
    int      local_depth;
    int      count;
    HFRecord records[HF_BUCKET_CAPACITY];
    long     overflow_offset;  /* HF_OFFSET_NULL se sem overflow */
} HFBucket;

/* ── Cabeçalho persistido no início do .hf ─────────────── */
typedef struct {
    uint32_t magic;
    int      global_depth;
    int      dir_size;         /* 2^global_depth                */
    long     dir_offset;       /* offset do diretório           */
    int      bucket_capacity;  /* = HF_BUCKET_CAPACITY          */
    int      total_records;    /* registros ativos              */
    int      total_buckets;    /* buckets alocados              */
    int      split_count;      /* histórico de splits           */
    int      expand_count;     /* histórico de expansões        */
} HFHeader;

/* ── Handle opaco ───────────────────────────────────────── */
typedef struct HashFile *HashFile;

/* ── API pública ────────────────────────────────────────── */

/** Função de hash (djb2). Exposta para testes unitários. */
uint32_t hf_hash(const char *key);

/** Índice no diretório: hash & (2^depth - 1). Exposta para testes. */
int hf_dir_index(uint32_t hash, int depth);

/** Cria novo hashfile (sobrescreve se existir). NULL em erro. */
HashFile hf_create(const char *path);

/** Abre hashfile existente. NULL se não existir/inválido. */
HashFile hf_open(const char *path);

/** Persiste e fecha o hashfile, libera memória. */
void hf_close(HashFile hf);

/**
 * Insere (key, data).
 * @return  0=OK  -1=duplicata  -2=erro I/O
 */
int hf_insert(HashFile hf, const char *key, const char *data);

/**
 * Busca por key; copia registro em *out (se out!=NULL).
 * @return  0=encontrado  -1=não encontrado
 */
int hf_search(HashFile hf, const char *key, HFRecord *out);

/**
 * Remove registro com key.
 * @return  0=OK  -1=não encontrado
 */
int hf_remove(HashFile hf, const char *key);

/**
 * Atualiza data do registro com key.
 * @return  0=OK  -1=não encontrado
 */
int hf_update(HashFile hf, const char *key, const char *data);

/**
 * Itera sobre todos os registros ativos chamando cb(rec, userdata).
 * @return  número de registros visitados
 */
int hf_iterate(HashFile hf,
               void (*cb)(const HFRecord *rec, void *userdata),
               void *userdata);

/** Retorna número de registros ativos. */
int hf_count(HashFile hf);

/**
 * Gera arquivo texto .hfd com dump legível da estrutura interna.
 * Registra expansões de diretório e splits de bucket.
 */
void hf_dump(HashFile hf, const char *hfd_path);

#endif /* HASHFILE_H */
