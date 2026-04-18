/**
 * t_args.c - Testes unitários do módulo args
 */
#include "../unity/unity.h"
#include "../src/args.h"
#include <string.h>

void setUp(void) {}
void tearDown(void) {}

void test_parse_args_minimos(void) {
    char *argv[] = {"ted", "-f", "cidade.geo", "-o", "/tmp/saida"};
    TedArgs args;
    int ret = args_parse(5, argv, &args);
    TEST_ASSERT_EQUAL_INT(0, ret);
    TEST_ASSERT_EQUAL_STRING("cidade.geo", args.arq_geo);
    TEST_ASSERT_EQUAL_STRING("/tmp/saida", args.base_saida);
}

void test_parse_falta_parametro_obrigatorio(void) {
    char *argv[] = {"ted", "-o", "/tmp/saida"};
    TedArgs args;
    int ret = args_parse(3, argv, &args);
    TEST_ASSERT_EQUAL_INT(-1, ret);
}

void test_parse_com_todos_parametros(void) {
    char *argv[] = {
        "ted", "-e", "/entrada", "-f", "t.geo",
        "-o", "/saida", "-q", "q.qry", "-pm", "p.pm"
    };
    TedArgs args;
    int ret = args_parse(11, argv, &args);
    TEST_ASSERT_EQUAL_INT(0, ret);
    TEST_ASSERT_EQUAL_STRING("/entrada", args.base_entrada);
    TEST_ASSERT_EQUAL_STRING("t.geo",   args.arq_geo);
    TEST_ASSERT_EQUAL_STRING("/saida",  args.base_saida);
    TEST_ASSERT_EQUAL_INT(1, args.has_qry);
    TEST_ASSERT_EQUAL_INT(1, args.has_pm);
}

void test_build_path_simples(void) {
    char dest[256];
    args_build_path("/base", "arq.geo", dest, sizeof(dest));
    TEST_ASSERT_EQUAL_STRING("/base/arq.geo", dest);
}

void test_build_path_caminho_absoluto_nao_prefixa(void) {
    char dest[256];
    args_build_path("/base", "/abs/path.geo", dest, sizeof(dest));
    TEST_ASSERT_EQUAL_STRING("/abs/path.geo", dest);
}

void test_basename_remove_extensao(void) {
    char dest[64];
    args_basename("/foo/bar/t001.geo", dest, sizeof(dest));
    TEST_ASSERT_EQUAL_STRING("t001", dest);
}

void test_basename_sem_diretorio(void) {
    char dest[64];
    args_basename("arquivo.qry", dest, sizeof(dest));
    TEST_ASSERT_EQUAL_STRING("arquivo", dest);
}

void test_parse_remove_barra_final(void) {
    char *argv[] = {"ted", "-f", "a.geo", "-o", "/saida/"};
    TedArgs args;
    args_parse(5, argv, &args);
    int len = (int)strlen(args.base_saida);
    TEST_ASSERT_TRUE(args.base_saida[len-1] != '/');
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_parse_args_minimos);
    RUN_TEST(test_parse_falta_parametro_obrigatorio);
    RUN_TEST(test_parse_com_todos_parametros);
    RUN_TEST(test_build_path_simples);
    RUN_TEST(test_build_path_caminho_absoluto_nao_prefixa);
    RUN_TEST(test_basename_remove_extensao);
    RUN_TEST(test_basename_sem_diretorio);
    RUN_TEST(test_parse_remove_barra_final);
    return UNITY_END();
}
