/**
 * hashfile.c - Hashfile Dinâmico (Extendible Hashing) em disco.
 *
 * Layout .hf:
 *   [HFHeader][Bucket 0][Bucket 1]...[Bucket N][Diretório]
 *
 * O diretório fica SEMPRE no final do arquivo e seu offset é
 * gravado no cabeçalho. Isso evita sobreposição entre diretório
 * e buckets quando o diretório cresce (duplicação).
 */

#include "hashfile.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

struct HashFile {
    FILE    *fp;
    char     path[512];
    HFHeader hdr;
    long    *dir;
};

/* ── I/O ─────────────────────────────────────────────── */
static void write_header(struct HashFile *hf) {
    rewind(hf->fp);
    fwrite(&hf->hdr, sizeof(HFHeader), 1, hf->fp);
    fflush(hf->fp);
}

static void read_header(struct HashFile *hf) {
    rewind(hf->fp);
    fread(&hf->hdr, sizeof(HFHeader), 1, hf->fp);
}

static void write_dir(struct HashFile *hf) {
    /* Diretório sempre no final do arquivo */
    fseek(hf->fp, 0, SEEK_END);
    long pos = ftell(hf->fp);
    hf->hdr.dir_offset = pos;
    fwrite(hf->dir, sizeof(long), (size_t)hf->hdr.dir_size, hf->fp);
    fflush(hf->fp);
    /* Salva novo offset no cabeçalho */
    write_header(hf);
}

static void read_dir(struct HashFile *hf) {
    hf->dir = realloc(hf->dir, (size_t)hf->hdr.dir_size * sizeof(long));
    assert(hf->dir);
    fseek(hf->fp, hf->hdr.dir_offset, SEEK_SET);
    fread(hf->dir, sizeof(long), (size_t)hf->hdr.dir_size, hf->fp);
}

static void write_bucket(struct HashFile *hf, long off, const HFBucket *b) {
    fseek(hf->fp, off, SEEK_SET);
    fwrite(b, sizeof(HFBucket), 1, hf->fp);
    fflush(hf->fp);
}

static void read_bucket(struct HashFile *hf, long off, HFBucket *b) {
    fseek(hf->fp, off, SEEK_SET);
    fread(b, sizeof(HFBucket), 1, hf->fp);
}

/* Aloca bucket logo após o cabeçalho (antes do diretório).
   Buckets são alocados sequencialmente.
   IMPORTANTE: Chama write_dir DEPOIS de todos os alloc_bucket. */
static long alloc_bucket_raw(struct HashFile *hf, int local_depth) {
    HFBucket b;
    memset(&b, 0, sizeof(b));
    b.local_depth     = local_depth;
    b.count           = 0;
    b.overflow_offset = HF_OFFSET_NULL;
    /* Insere antes do diretório: reescreve dir depois */
    /* Pega posição logo após o último bucket */
    long off = hf->hdr.dir_offset; /* será substituído após write_dir */
    /* Na verdade, inserimos no offset indicado pelo dir_offset atual
       porque o diretório ainda não foi reescrito. Mas queremos inserir
       ANTES do diretório atual. Usaremos dir_offset como ponto de inserção
       e depois moveremos o diretório para além deste bucket. */
    fseek(hf->fp, off, SEEK_SET);
    fwrite(&b, sizeof(HFBucket), 1, hf->fp);
    hf->hdr.total_buckets++;
    /* dir_offset agora aponta para este bucket; o diretório será
       reescrito no final após todos os allocs desta operação. */
    hf->hdr.dir_offset += sizeof(HFBucket);
    return off;
}

/* Versão pública: aloca e imediatamente reescreve diretório */
static long alloc_bucket(struct HashFile *hf, int local_depth) {
    long off = alloc_bucket_raw(hf, local_depth);
    /* Reescreve diretório no novo final */
    write_dir(hf);
    return off;
}

/* ── Hash ────────────────────────────────────────────── */
uint32_t hf_hash(const char *key) {
    uint32_t h = 5381u;
    for (const unsigned char *p = (const unsigned char *)key; *p; p++)
        h = ((h << 5) + h) ^ (uint32_t)(*p);
    return h;
}

int hf_dir_index(uint32_t hash, int depth) {
    if (depth == 0) return 0;
    return (int)(hash & (uint32_t)((1 << depth) - 1));
}

/* ── Criação ─────────────────────────────────────────── */
HashFile hf_create(const char *path) {
    assert(path);
    struct HashFile *hf = calloc(1, sizeof(struct HashFile));
    assert(hf);
    strncpy(hf->path, path, sizeof(hf->path) - 1);

    hf->fp = fopen(path, "w+b");
    if (!hf->fp) { free(hf); return NULL; }

    hf->hdr.magic           = HF_MAGIC;
    hf->hdr.global_depth    = 1;
    hf->hdr.dir_size        = 2;
    hf->hdr.bucket_capacity = HF_BUCKET_CAPACITY;
    hf->hdr.total_records   = 0;
    hf->hdr.total_buckets   = 0;
    hf->hdr.split_count     = 0;
    hf->hdr.expand_count    = 0;

    /* Diretório inicia logo após o cabeçalho */
    hf->hdr.dir_offset = (long)sizeof(HFHeader);

    write_header(hf);

    /* Diretório temporário (placeholder) */
    hf->dir = calloc(2, sizeof(long));
    assert(hf->dir);
    hf->dir[0] = 0;
    hf->dir[1] = 0;
    write_dir(hf); /* escreve dir no final; atualiza dir_offset */

    /* Aloca 2 buckets iniciais com profundidade local = 1 */
    long off0 = alloc_bucket_raw(hf, 1);
    long off1 = alloc_bucket_raw(hf, 1);

    hf->dir[0] = off0;
    hf->dir[1] = off1;

    /* Reescreve diretório com os offsets corretos */
    write_dir(hf);
    return hf;
}

/* ── Abertura ────────────────────────────────────────── */
HashFile hf_open(const char *path) {
    assert(path);
    struct HashFile *hf = calloc(1, sizeof(struct HashFile));
    assert(hf);
    strncpy(hf->path, path, sizeof(hf->path) - 1);

    hf->fp = fopen(path, "r+b");
    if (!hf->fp) { free(hf); return NULL; }

    read_header(hf);
    if (hf->hdr.magic != HF_MAGIC) {
        fclose(hf->fp); free(hf); return NULL;
    }
    hf->dir = NULL;
    read_dir(hf);
    return hf;
}

/* ── Fechamento ──────────────────────────────────────── */
void hf_close(HashFile hf) {
    if (!hf) return;
    write_dir(hf);
    write_header(hf);
    fflush(hf->fp);
    fclose(hf->fp);
    free(hf->dir);
    free(hf);
}

/* ── Expansão do diretório ───────────────────────────── */
static void expand_directory(struct HashFile *hf) {
    int old_sz = hf->hdr.dir_size;
    int new_sz = old_sz * 2;
    long *nd = malloc((size_t)new_sz * sizeof(long));
    assert(nd);
    for (int i = 0; i < old_sz; i++) {
        nd[2*i]   = hf->dir[i];
        nd[2*i+1] = hf->dir[i];
    }
    free(hf->dir);
    hf->dir = nd;
    hf->hdr.dir_size = new_sz;
    hf->hdr.global_depth++;
    hf->hdr.expand_count++;
    /* dir será reescrito pelo chamador (split_bucket → write_dir) */
}

/* ── Split ───────────────────────────────────────────── */
static void split_bucket(struct HashFile *hf, long split_off) {
    HFBucket old_b;
    read_bucket(hf, split_off, &old_b);

    if (old_b.local_depth == hf->hdr.global_depth)
        expand_directory(hf);

    int  new_depth = old_b.local_depth + 1;

    /* Aloca novo bucket (NÃO chama write_dir ainda) */
    long new_off = alloc_bucket_raw(hf, new_depth);

    old_b.local_depth     = new_depth;
    old_b.overflow_offset = HF_OFFSET_NULL;

    HFBucket new_b;
    memset(&new_b, 0, sizeof(new_b));
    new_b.local_depth     = new_depth;
    new_b.count           = 0;
    new_b.overflow_offset = HF_OFFSET_NULL;

    /* Redireciona entradas do diretório */
    int bit_mask = 1 << (new_depth - 1);
    for (int i = 0; i < hf->hdr.dir_size; i++) {
        if (hf->dir[i] == split_off && (i & bit_mask))
            hf->dir[i] = new_off;
    }

    /* Redistribui registros */
    HFRecord tmp[HF_BUCKET_CAPACITY];
    int tmp_cnt = old_b.count;
    memcpy(tmp, old_b.records, (size_t)tmp_cnt * sizeof(HFRecord));

    old_b.count = 0;
    new_b.count = 0;
    memset(old_b.records, 0, sizeof(old_b.records));

    for (int i = 0; i < tmp_cnt; i++) {
        if (!tmp[i].active) continue;
        uint32_t h   = hf_hash(tmp[i].key);
        int      idx = hf_dir_index(h, hf->hdr.global_depth);
        /* Usa o diretório atualizado para decidir o bucket de destino */
        if (hf->dir[idx] == split_off)
            old_b.records[old_b.count++] = tmp[i];
        else
            new_b.records[new_b.count++] = tmp[i];
    }

    write_bucket(hf, split_off, &old_b);
    write_bucket(hf, new_off,   &new_b);
    write_dir(hf);  /* move diretório para o novo final */
    hf->hdr.split_count++;
    write_header(hf);
}

/* ── Inserção ────────────────────────────────────────── */
int hf_insert(HashFile hf, const char *key, const char *data) {
    assert(hf && key && data);

    uint32_t h   = hf_hash(key);
    int      idx = hf_dir_index(h, hf->hdr.global_depth);
    long     cur  = hf->dir[idx];
    long     prev = HF_OFFSET_NULL;

    while (cur != HF_OFFSET_NULL) {
        HFBucket b;
        read_bucket(hf, cur, &b);

        /* Duplicata? */
        for (int i = 0; i < b.count; i++) {
            if (b.records[i].active &&
                strncmp(b.records[i].key, key, HF_KEY_SIZE) == 0)
                return -1;
        }

        if (b.count < HF_BUCKET_CAPACITY) {
            /* Espaço disponível */
            strncpy(b.records[b.count].key,  key,  HF_KEY_SIZE  - 1);
            strncpy(b.records[b.count].data, data, HF_DATA_SIZE - 1);
            b.records[b.count].key[HF_KEY_SIZE-1]   = '\0';
            b.records[b.count].data[HF_DATA_SIZE-1] = '\0';
            b.records[b.count].active = 1;
            b.count++;
            write_bucket(hf, cur, &b);
            hf->hdr.total_records++;
            write_header(hf);
            return 0;
        }
        prev = cur;
        cur  = b.overflow_offset;
    }

    /* Bucket primário cheio → split */
    if (prev == HF_OFFSET_NULL) {
        split_bucket(hf, hf->dir[idx]);
        return hf_insert(hf, key, data);
    }

    /* Todos os overflows cheios → novo bucket de overflow */
    long ov = alloc_bucket(hf, 0);
    HFBucket prev_b;
    read_bucket(hf, prev, &prev_b);
    prev_b.overflow_offset = ov;
    write_bucket(hf, prev, &prev_b);

    HFBucket ov_b;
    read_bucket(hf, ov, &ov_b);
    strncpy(ov_b.records[0].key,  key,  HF_KEY_SIZE  - 1);
    strncpy(ov_b.records[0].data, data, HF_DATA_SIZE - 1);
    ov_b.records[0].key[HF_KEY_SIZE-1]   = '\0';
    ov_b.records[0].data[HF_DATA_SIZE-1] = '\0';
    ov_b.records[0].active = 1;
    ov_b.count = 1;
    write_bucket(hf, ov, &ov_b);
    hf->hdr.total_records++;
    write_header(hf);
    return 0;
}

/* ── Busca ───────────────────────────────────────────── */
int hf_search(HashFile hf, const char *key, HFRecord *out) {
    assert(hf && key);
    uint32_t h   = hf_hash(key);
    int      idx = hf_dir_index(h, hf->hdr.global_depth);
    long     cur = hf->dir[idx];

    while (cur != HF_OFFSET_NULL) {
        HFBucket b;
        read_bucket(hf, cur, &b);
        for (int i = 0; i < b.count; i++) {
            if (b.records[i].active &&
                strncmp(b.records[i].key, key, HF_KEY_SIZE) == 0) {
                if (out) *out = b.records[i];
                return 0;
            }
        }
        cur = b.overflow_offset;
    }
    return -1;
}

/* ── Remoção ─────────────────────────────────────────── */
int hf_remove(HashFile hf, const char *key) {
    assert(hf && key);
    uint32_t h   = hf_hash(key);
    int      idx = hf_dir_index(h, hf->hdr.global_depth);
    long     cur = hf->dir[idx];

    while (cur != HF_OFFSET_NULL) {
        HFBucket b;
        read_bucket(hf, cur, &b);
        for (int i = 0; i < b.count; i++) {
            if (b.records[i].active &&
                strncmp(b.records[i].key, key, HF_KEY_SIZE) == 0) {
                b.records[i].active = 0;
                write_bucket(hf, cur, &b);
                hf->hdr.total_records--;
                write_header(hf);
                return 0;
            }
        }
        cur = b.overflow_offset;
    }
    return -1;
}

/* ── Atualização ─────────────────────────────────────── */
int hf_update(HashFile hf, const char *key, const char *new_data) {
    assert(hf && key && new_data);
    uint32_t h   = hf_hash(key);
    int      idx = hf_dir_index(h, hf->hdr.global_depth);
    long     cur = hf->dir[idx];

    while (cur != HF_OFFSET_NULL) {
        HFBucket b;
        read_bucket(hf, cur, &b);
        for (int i = 0; i < b.count; i++) {
            if (b.records[i].active &&
                strncmp(b.records[i].key, key, HF_KEY_SIZE) == 0) {
                strncpy(b.records[i].data, new_data, HF_DATA_SIZE - 1);
                b.records[i].data[HF_DATA_SIZE-1] = '\0';
                write_bucket(hf, cur, &b);
                return 0;
            }
        }
        cur = b.overflow_offset;
    }
    return -1;
}

/* ── Iteração ────────────────────────────────────────── */
int hf_iterate(HashFile hf,
               void (*cb)(const HFRecord *, void *),
               void *ud) {
    assert(hf);
    int   vcap = 128;
    long *vis  = malloc((size_t)vcap * sizeof(long));
    assert(vis);
    int vcnt = 0, total = 0;

    for (int i = 0; i < hf->hdr.dir_size; i++) {
        long off = hf->dir[i];
        int dup = 0;
        for (int j = 0; j < vcnt; j++)
            if (vis[j] == off) { dup = 1; break; }
        if (dup) continue;

        if (vcnt == vcap) {
            vcap *= 2;
            vis = realloc(vis, (size_t)vcap * sizeof(long));
            assert(vis);
        }
        vis[vcnt++] = off;

        long cur = off;
        while (cur != HF_OFFSET_NULL) {
            HFBucket b;
            read_bucket(hf, cur, &b);
            for (int k = 0; k < b.count; k++) {
                if (b.records[k].active) {
                    if (cb) cb(&b.records[k], ud);
                    total++;
                }
            }
            cur = b.overflow_offset;
        }
    }
    free(vis);
    return total;
}

/* ── Dump .hfd ───────────────────────────────────────── */
void hf_dump(HashFile hf, const char *hfd_path) {
    assert(hf && hfd_path);
    FILE *f = fopen(hfd_path, "w");
    if (!f) return;

    fprintf(f, "=== HASHFILE DUMP: %s ===\n", hf->path);
    fprintf(f, "global_depth  : %d\n", hf->hdr.global_depth);
    fprintf(f, "dir_size      : %d\n", hf->hdr.dir_size);
    fprintf(f, "dir_offset    : %ld\n", hf->hdr.dir_offset);
    fprintf(f, "total_records : %d\n", hf->hdr.total_records);
    fprintf(f, "total_buckets : %d\n", hf->hdr.total_buckets);
    fprintf(f, "split_count   : %d\n", hf->hdr.split_count);
    fprintf(f, "expand_count  : %d\n", hf->hdr.expand_count);
    fprintf(f, "\n--- DIRETÓRIO ---\n");

    for (int i = 0; i < hf->hdr.dir_size; i++) {
        char bits[33] = {0};
        int d = hf->hdr.global_depth;
        for (int b = d - 1; b >= 0; b--)
            bits[d - 1 - b] = ((i >> b) & 1) ? '1' : '0';
        fprintf(f, "  [%3d] %s -> @%ld\n", i, bits, hf->dir[i]);
    }

    fprintf(f, "\n--- BUCKETS ---\n");
    int  vcap = 128;
    long *vis  = malloc((size_t)vcap * sizeof(long));
    assert(vis);
    int vcnt = 0;

    for (int i = 0; i < hf->hdr.dir_size; i++) {
        long off = hf->dir[i];
        int dup = 0;
        for (int j = 0; j < vcnt; j++)
            if (vis[j] == off) { dup = 1; break; }
        if (dup) continue;
        if (vcnt == vcap) {
            vcap *= 2;
            vis = realloc(vis, (size_t)vcap * sizeof(long));
            assert(vis);
        }
        vis[vcnt++] = off;

        long cur = off;
        int  ov  = 0;
        while (cur != HF_OFFSET_NULL) {
            HFBucket b;
            read_bucket(hf, cur, &b);
            fprintf(f, "%sBUCKET @%ld (ld=%d, cnt=%d)\n",
                    ov ? "  OV " : "", cur, b.local_depth, b.count);
            for (int k = 0; k < b.count; k++) {
                fprintf(f, "    [%d] %s key='%s'\n", k,
                        b.records[k].active ? "ACTIVE" : "DELETED",
                        b.records[k].key);
            }
            cur = b.overflow_offset;
            ov  = 1;
        }
    }
    free(vis);
    fclose(f);
}

/* ── Contagem ────────────────────────────────────────── */
int hf_count(HashFile hf) {
    assert(hf);
    return hf->hdr.total_records;
}
