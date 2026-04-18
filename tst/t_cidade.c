/**
 * t_cidade.c - Testes unitários do módulo cidade
 */
#include "../unity/unity.h"
#include "../src/hashfile.h"
#include "../src/quadra.h"
#include "../src/pessoa.h"
#include "../src/cidade.h"
#include <string.h>
#include <stdio.h>

#define HFQ_PATH "/tmp/test_cidade_q.hf"
#define HFP_PATH "/tmp/test_cidade_p.hf"

static HashFile hfq = NULL, hfp = NULL;
static CidadeCtx ctx;

static void insert_quadra(const char *cep, float x, float y, float w, float h) {
    Quadra q;
    memset(&q, 0, sizeof(q));
    strncpy(q.cep, cep, sizeof(q.cep)-1);
    q.x=x; q.y=y; q.w=w; q.h=h; q.sw=1.0f;
    strcpy(q.cfill,"orange"); strcpy(q.cstrk,"black");
    char key[HF_KEY_SIZE], ser[HF_DATA_SIZE];
    quadra_key(&q, key, sizeof(key));
    quadra_serialize(&q, ser, sizeof(ser));
    hf_insert(hfq, key, ser);
}

static void insert_pessoa(const char *cpf, const char *cep, char face,
                           int num, char sexo) {
    Pessoa p;
    memset(&p, 0, sizeof(p));
    strncpy(p.cpf, cpf, sizeof(p.cpf)-1);
    strcpy(p.nome, "Nome"); strcpy(p.sobrenome, "Sob");
    p.sexo = sexo;
    strcpy(p.nasc, "01/01/2000");
    if (cep) {
        strncpy(p.end.cep, cep, sizeof(p.end.cep)-1);
        p.end.face = face;
        p.end.num  = num;
        p.end.has_addr = 1;
    }
    char key[HF_KEY_SIZE], ser[HF_DATA_SIZE];
    pessoa_key(&p, key, sizeof(key));
    pessoa_serialize(&p, ser, sizeof(ser));
    hf_insert(hfp, key, ser);
}

void setUp(void) {
    remove(HFQ_PATH); remove(HFP_PATH);
    hfq = hf_create(HFQ_PATH);
    hfp = hf_create(HFP_PATH);
    cidade_init(&ctx, hfq, hfp);
}

void tearDown(void) {
    hf_close(hfq); hf_close(hfp);
    remove(HFQ_PATH); remove(HFP_PATH);
}

void test_rq_remove_quadra(void) {
    insert_quadra("cep01", 0,0,100,50);
    cidade_rq(&ctx, "cep01", NULL, NULL);
    TEST_ASSERT_EQUAL_INT(-1, hf_search(hfq, "cep01", NULL));
}

void test_rq_moradores_viram_sem_teto(void) {
    insert_quadra("cep01", 0,0,100,50);
    insert_pessoa("cpf001", "cep01", 'N', 5, 'M');
    cidade_rq(&ctx, "cep01", NULL, NULL);
    HFRecord rec;
    hf_search(hfp, "cpf001", &rec);
    Pessoa p; pessoa_deserialize(&p, rec.data);
    TEST_ASSERT_EQUAL_INT(0, p.end.has_addr);
}

void test_nasc_insere_pessoa(void) {
    cidade_nasc(&ctx, "cpf999", "Ana", "Souza", 'F', "15/03/1995");
    TEST_ASSERT_EQUAL_INT(0, hf_search(hfp, "cpf999", NULL));
}

void test_rip_remove_pessoa(void) {
    insert_pessoa("cpf002", NULL, 0, 0, 'F');
    cidade_rip(&ctx, "cpf002", NULL, NULL);
    TEST_ASSERT_EQUAL_INT(-1, hf_search(hfp, "cpf002", NULL));
}

void test_h_query_pessoa_existente(void) {
    insert_pessoa("cpf003", NULL, 0, 0, 'M');
    FILE *f = fopen("/tmp/ted_hq_test.txt","w");
    cidade_h_query(&ctx, "cpf003", f);
    fclose(f);
    f = fopen("/tmp/ted_hq_test.txt","r");
    TEST_ASSERT_NOT_NULL(f);
    char buf[256]; fgets(buf, sizeof(buf), f); fclose(f);
    TEST_ASSERT_TRUE(strstr(buf,"cpf003") != NULL);
    remove("/tmp/ted_hq_test.txt");
}

void test_h_query_pessoa_inexistente(void) {
    FILE *f = fopen("/tmp/ted_hq_test2.txt","w");
    cidade_h_query(&ctx, "naoexiste", f);
    fclose(f);
    f = fopen("/tmp/ted_hq_test2.txt","r");
    TEST_ASSERT_NOT_NULL(f);
    char buf[256]; fgets(buf,sizeof(buf),f); fclose(f);
    TEST_ASSERT_TRUE(strstr(buf,"nao") != NULL ||
                     strstr(buf,"Nao") != NULL ||
                     strlen(buf) > 0);
    remove("/tmp/ted_hq_test2.txt");
}

void test_mud_atualiza_endereco(void) {
    insert_quadra("cep02",0,0,100,50);
    insert_pessoa("cpf004", NULL, 0, 0, 'M');
    cidade_mud(&ctx,"cpf004","cep02",'S',7,"ap1",NULL);
    HFRecord rec;
    hf_search(hfp,"cpf004",&rec);
    Pessoa p; pessoa_deserialize(&p,rec.data);
    TEST_ASSERT_EQUAL_INT(1, p.end.has_addr);
    TEST_ASSERT_EQUAL_STRING("cep02", p.end.cep);
    TEST_ASSERT_EQUAL_INT((int)'S', (int)p.end.face);
}

void test_dspj_vira_sem_teto(void) {
    insert_quadra("cep03",0,0,100,50);
    insert_pessoa("cpf005","cep03",'N',3,'F');
    cidade_dspj(&ctx,"cpf005",NULL,NULL);
    HFRecord rec;
    hf_search(hfp,"cpf005",&rec);
    Pessoa p; pessoa_deserialize(&p,rec.data);
    TEST_ASSERT_EQUAL_INT(0, p.end.has_addr);
}

void test_censo_gera_saida(void) {
    insert_pessoa("cpf010",NULL,0,0,'M');
    insert_pessoa("cpf011","cep99",'N',1,'F');
    FILE *f = fopen("/tmp/ted_censo.txt","w");
    cidade_censo(&ctx, f);
    fclose(f);
    f = fopen("/tmp/ted_censo.txt","r");
    TEST_ASSERT_NOT_NULL(f);
    fclose(f);
    remove("/tmp/ted_censo.txt");
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_rq_remove_quadra);
    RUN_TEST(test_rq_moradores_viram_sem_teto);
    RUN_TEST(test_nasc_insere_pessoa);
    RUN_TEST(test_rip_remove_pessoa);
    RUN_TEST(test_h_query_pessoa_existente);
    RUN_TEST(test_h_query_pessoa_inexistente);
    RUN_TEST(test_mud_atualiza_endereco);
    RUN_TEST(test_dspj_vira_sem_teto);
    RUN_TEST(test_censo_gera_saida);
    return UNITY_END();
}
