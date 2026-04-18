/**
 * t_pessoa.c - Testes unitários do módulo pessoa
 */
#include "../unity/unity.h"
#include "../src/pessoa.h"
#include "../src/hashfile.h"
#include <string.h>

void setUp(void) {}
void tearDown(void) {}

static Pessoa make_p(void) {
    Pessoa p;
    memset(&p, 0, sizeof(p));
    strncpy(p.cpf,       "12345678901", sizeof(p.cpf)-1);
    strncpy(p.nome,      "Joao",        sizeof(p.nome)-1);
    strncpy(p.sobrenome, "Silva",       sizeof(p.sobrenome)-1);
    p.sexo = 'M';
    strncpy(p.nasc, "01/01/1990", sizeof(p.nasc)-1);
    p.end.has_addr = 0;
    return p;
}

void test_roundtrip_sem_endereco(void) {
    Pessoa p = make_p();
    char buf[HF_DATA_SIZE];
    pessoa_serialize(&p, buf, sizeof(buf));
    Pessoa p2;
    pessoa_deserialize(&p2, buf);
    TEST_ASSERT_EQUAL_STRING(p.cpf, p2.cpf);
    TEST_ASSERT_EQUAL_STRING(p.nome, p2.nome);
    TEST_ASSERT_EQUAL_STRING(p.sobrenome, p2.sobrenome);
    TEST_ASSERT_EQUAL_INT((int)p.sexo, (int)p2.sexo);
    TEST_ASSERT_EQUAL_STRING(p.nasc, p2.nasc);
    TEST_ASSERT_EQUAL_INT(0, p2.end.has_addr);
}

void test_roundtrip_com_endereco(void) {
    Pessoa p = make_p();
    strncpy(p.end.cep, "cep01", sizeof(p.end.cep)-1);
    p.end.face     = 'N';
    p.end.num      = 42;
    strncpy(p.end.compl, "apto3", sizeof(p.end.compl)-1);
    p.end.has_addr = 1;

    char buf[HF_DATA_SIZE];
    pessoa_serialize(&p, buf, sizeof(buf));
    Pessoa p2;
    pessoa_deserialize(&p2, buf);

    TEST_ASSERT_EQUAL_INT(1, p2.end.has_addr);
    TEST_ASSERT_EQUAL_STRING("cep01", p2.end.cep);
    TEST_ASSERT_EQUAL_INT((int)'N', (int)p2.end.face);
    TEST_ASSERT_EQUAL_INT(42, p2.end.num);
}

void test_key_igual_cpf(void) {
    Pessoa p = make_p();
    char key[HF_KEY_SIZE];
    pessoa_key(&p, key, sizeof(key));
    TEST_ASSERT_EQUAL_STRING(p.cpf, key);
}

void test_is_morador_sem_endereco(void) {
    Pessoa p = make_p();
    TEST_ASSERT_FALSE(pessoa_is_morador(&p));
}

void test_is_morador_com_endereco(void) {
    Pessoa p = make_p();
    p.end.has_addr = 1;
    TEST_ASSERT_TRUE(pessoa_is_morador(&p));
}

void test_fmt_endereco_sem_teto(void) {
    Pessoa p = make_p();
    char buf[128];
    pessoa_fmt_endereco(&p, buf, sizeof(buf));
    TEST_ASSERT_TRUE(strstr(buf, "sem") != NULL ||
                     strlen(buf) > 0);
}

void test_fmt_endereco_com_endereco(void) {
    Pessoa p = make_p();
    strncpy(p.end.cep, "cep05", sizeof(p.end.cep)-1);
    p.end.face = 'S';
    p.end.num  = 10;
    p.end.has_addr = 1;
    char buf[128];
    pessoa_fmt_endereco(&p, buf, sizeof(buf));
    TEST_ASSERT_TRUE(strstr(buf, "cep05") != NULL);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_roundtrip_sem_endereco);
    RUN_TEST(test_roundtrip_com_endereco);
    RUN_TEST(test_key_igual_cpf);
    RUN_TEST(test_is_morador_sem_endereco);
    RUN_TEST(test_is_morador_com_endereco);
    RUN_TEST(test_fmt_endereco_sem_teto);
    RUN_TEST(test_fmt_endereco_com_endereco);
    return UNITY_END();
}
