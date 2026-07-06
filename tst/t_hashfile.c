/**
 * t_hashfile.c - Testes unitários do módulo hashfile
 */
#include "../unity/unity.h"
#include "../src/hashfile.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define TEST_HF "/tmp/test_ted_hash.hf"

void setUp(void) {
    remove(TEST_HF);
}
void tearDown(void) {
    remove(TEST_HF);
}

/* ── Testes de hash ──────────────────────────────────── */
void test_hash_nao_zero_para_string_valida(void) {
    uint32_t h = hf_hash("cep01");
    TEST_ASSERT_TRUE(h != 0);
}

void test_hash_deterministico(void) {
    TEST_ASSERT_EQUAL_INT((int)hf_hash("abc"), (int)hf_hash("abc"));
}

void test_hash_diferente_para_chaves_distintas(void) {
    uint32_t h1 = hf_hash("cep01");
    uint32_t h2 = hf_hash("cep99");
    TEST_ASSERT_TRUE(h1 != h2);
}

void test_dir_index_profundidade_1(void) {
    /* profundidade 1 → índice é bit 0 do hash */
    uint32_t h = 0b101;
    TEST_ASSERT_EQUAL_INT(1, hf_dir_index(h, 1));
    h = 0b100;
    TEST_ASSERT_EQUAL_INT(0, hf_dir_index(h, 1));
}

void test_dir_index_profundidade_2(void) {
    uint32_t h = 0b11;
    TEST_ASSERT_EQUAL_INT(3, hf_dir_index(h, 2));
}

/* ── Criação ─────────────────────────────────────────── */
void test_create_retorna_handle_valido(void) {
    HashFile hf = hf_create(TEST_HF);
    TEST_ASSERT_NOT_NULL(hf);
    hf_close(hf);
}

void test_create_arquivo_existe_apos_criacao(void) {
    HashFile hf = hf_create(TEST_HF);
    TEST_ASSERT_NOT_NULL(hf);
    hf_close(hf);
    FILE *f = fopen(TEST_HF, "rb");
    TEST_ASSERT_NOT_NULL(f);
    if (f) fclose(f);
}

/* ── Inserção ────────────────────────────────────────── */
void test_insert_retorna_zero_em_sucesso(void) {
    HashFile hf = hf_create(TEST_HF);
    int ret = hf_insert(hf, "cep01", "dados01");
    TEST_ASSERT_EQUAL_INT(0, ret);
    hf_close(hf);
}

void test_insert_duplicata_retorna_menos_um(void) {
    HashFile hf = hf_create(TEST_HF);
    hf_insert(hf, "cep01", "dados01");
    int ret = hf_insert(hf, "cep01", "dados_dup");
    TEST_ASSERT_EQUAL_INT(-1, ret);
    hf_close(hf);
}

void test_insert_multiplos_registros(void) {
    HashFile hf = hf_create(TEST_HF);
    for (int i = 0; i < 20; i++) {
        char key[32], data[64];
        snprintf(key, sizeof(key), "cep%02d", i);
        snprintf(data, sizeof(data), "dados_%02d", i);
        int ret = hf_insert(hf, key, data);
        TEST_ASSERT_EQUAL_INT(0, ret);
    }
    TEST_ASSERT_EQUAL_INT(20, hf_count(hf));
    hf_close(hf);
}

/* ── Busca ───────────────────────────────────────────── */
void test_search_encontra_registro_inserido(void) {
    HashFile hf = hf_create(TEST_HF);
    hf_insert(hf, "cep01", "dados01");
    HFRecord rec;
    int ret = hf_search(hf, "cep01", &rec);
    TEST_ASSERT_EQUAL_INT(0, ret);
    TEST_ASSERT_EQUAL_STRING("cep01",   rec.key);
    TEST_ASSERT_EQUAL_STRING("dados01", rec.data);
    hf_close(hf);
}

void test_search_nao_encontra_chave_inexistente(void) {
    HashFile hf = hf_create(TEST_HF);
    hf_insert(hf, "cep01", "dados01");
    int ret = hf_search(hf, "cep99", NULL);
    TEST_ASSERT_EQUAL_INT(-1, ret);
    hf_close(hf);
}

void test_search_com_out_rec_null_nao_crasha(void) {
    HashFile hf = hf_create(TEST_HF);
    hf_insert(hf, "cep01", "dados01");
    int ret = hf_search(hf, "cep01", NULL);
    TEST_ASSERT_EQUAL_INT(0, ret);
    hf_close(hf);
}

/* ── Remoção ─────────────────────────────────────────── */
void test_remove_retorna_zero_para_chave_existente(void) {
    HashFile hf = hf_create(TEST_HF);
    hf_insert(hf, "cep01", "dados01");
    int ret = hf_remove(hf, "cep01");
    TEST_ASSERT_EQUAL_INT(0, ret);
    hf_close(hf);
}

void test_remove_retorna_menos_um_para_chave_inexistente(void) {
    HashFile hf = hf_create(TEST_HF);
    int ret = hf_remove(hf, "inexistente");
    TEST_ASSERT_EQUAL_INT(-1, ret);
    hf_close(hf);
}

void test_remove_torna_chave_nao_encontravel(void) {
    HashFile hf = hf_create(TEST_HF);
    hf_insert(hf, "cep01", "dados01");
    hf_remove(hf, "cep01");
    int ret = hf_search(hf, "cep01", NULL);
    TEST_ASSERT_EQUAL_INT(-1, ret);
    hf_close(hf);
}

void test_count_decresce_apos_remove(void) {
    HashFile hf = hf_create(TEST_HF);
    hf_insert(hf, "cep01", "d");
    hf_insert(hf, "cep02", "d");
    TEST_ASSERT_EQUAL_INT(2, hf_count(hf));
    hf_remove(hf, "cep01");
    TEST_ASSERT_EQUAL_INT(1, hf_count(hf));
    hf_close(hf);
}

/* ── Atualização ─────────────────────────────────────── */
void test_update_modifica_dado(void) {
    HashFile hf = hf_create(TEST_HF);
    hf_insert(hf, "cep01", "original");
    hf_update(hf, "cep01", "atualizado");
    HFRecord rec;
    hf_search(hf, "cep01", &rec);
    TEST_ASSERT_EQUAL_STRING("atualizado", rec.data);
    hf_close(hf);
}

void test_update_retorna_menos_um_para_inexistente(void) {
    HashFile hf = hf_create(TEST_HF);
    int ret = hf_update(hf, "naoexiste", "x");
    TEST_ASSERT_EQUAL_INT(-1, ret);
    hf_close(hf);
}

/* ── Persistência (close/open) ───────────────────────── */
void test_persistencia_apos_close_open(void) {
    HashFile hf = hf_create(TEST_HF);
    hf_insert(hf, "cep01", "dados01");
    hf_insert(hf, "cep02", "dados02");
    hf_close(hf);

    hf = hf_open(TEST_HF);
    TEST_ASSERT_NOT_NULL(hf);
    TEST_ASSERT_EQUAL_INT(2, hf_count(hf));

    HFRecord rec;
    TEST_ASSERT_EQUAL_INT(0, hf_search(hf, "cep01", &rec));
    TEST_ASSERT_EQUAL_STRING("dados01", rec.data);
    TEST_ASSERT_EQUAL_INT(0, hf_search(hf, "cep02", &rec));
    TEST_ASSERT_EQUAL_STRING("dados02", rec.data);
    hf_close(hf);
}

/* ── Split / expansão dinâmica ───────────────────────── */
void test_split_e_expansao_com_muitos_registros(void) {
    HashFile hf = hf_create(TEST_HF);
    /* Insere 50 registros para forçar splits */
    for (int i = 0; i < 50; i++) {
        char key[32], data[64];
        snprintf(key, sizeof(key), "key%03d", i);
        snprintf(data, sizeof(data), "val%03d", i);
        hf_insert(hf, key, data);
    }
    TEST_ASSERT_EQUAL_INT(50, hf_count(hf));

    /* Todos devem ainda ser encontráveis */
    for (int i = 0; i < 50; i++) {
        char key[32];
        snprintf(key, sizeof(key), "key%03d", i);
        TEST_ASSERT_EQUAL_INT(0, hf_search(hf, key, NULL));
    }
    hf_close(hf);
}

/* ── Iteração ────────────────────────────────────────── */
typedef struct { int cnt; } IterCount;
static void count_cb(const HFRecord *r, void *ud) {
    (void)r;
    ((IterCount *)ud)->cnt++;
}

void test_iterate_conta_registros_corretos(void) {
    HashFile hf = hf_create(TEST_HF);
    for (int i = 0; i < 10; i++) {
        char k[16]; snprintf(k, sizeof(k), "k%d", i);
        hf_insert(hf, k, "v");
    }
    IterCount ic = {0};
    hf_iterate(hf, count_cb, &ic);
    TEST_ASSERT_EQUAL_INT(10, ic.cnt);
    hf_close(hf);
}

/* ── Dump ────────────────────────────────────────────── */
void test_dump_cria_arquivo_hfd(void) {
    HashFile hf = hf_create(TEST_HF);
    hf_insert(hf, "cep01", "dados");
    hf_dump(hf, "/tmp/test_ted_dump.hfd");
    hf_close(hf);

    FILE *f = fopen("/tmp/test_ted_dump.hfd", "r");
    TEST_ASSERT_NOT_NULL(f);
    if (f) { fclose(f); remove("/tmp/test_ted_dump.hfd"); }
}

/* ── Open em arquivo inválido ────────────────────────── */
void test_open_retorna_null_para_arquivo_invalido(void) {
    HashFile hf = hf_open("/tmp/nao_existe_ted_xyz.hf");
    TEST_ASSERT_NULL(hf);
}

/* ── main ────────────────────────────────────────────── */
int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_hash_nao_zero_para_string_valida);
    RUN_TEST(test_hash_deterministico);
    RUN_TEST(test_hash_diferente_para_chaves_distintas);
    RUN_TEST(test_dir_index_profundidade_1);
    RUN_TEST(test_dir_index_profundidade_2);
    RUN_TEST(test_create_retorna_handle_valido);
    RUN_TEST(test_create_arquivo_existe_apos_criacao);
    RUN_TEST(test_insert_retorna_zero_em_sucesso);
    RUN_TEST(test_insert_duplicata_retorna_menos_um);
    RUN_TEST(test_insert_multiplos_registros);
    RUN_TEST(test_search_encontra_registro_inserido);
    RUN_TEST(test_search_nao_encontra_chave_inexistente);
    RUN_TEST(test_search_com_out_rec_null_nao_crasha);
    RUN_TEST(test_remove_retorna_zero_para_chave_existente);
    RUN_TEST(test_remove_retorna_menos_um_para_chave_inexistente);
    RUN_TEST(test_remove_torna_chave_nao_encontravel);
    RUN_TEST(test_count_decresce_apos_remove);
    RUN_TEST(test_update_modifica_dado);
    RUN_TEST(test_update_retorna_menos_um_para_inexistente);
    RUN_TEST(test_persistencia_apos_close_open);
    RUN_TEST(test_split_e_expansao_com_muitos_registros);
    RUN_TEST(test_iterate_conta_registros_corretos);
    RUN_TEST(test_dump_cria_arquivo_hfd);
    RUN_TEST(test_open_retorna_null_para_arquivo_invalido);
    return UNITY_END();
}
