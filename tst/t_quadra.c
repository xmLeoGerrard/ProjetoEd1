/**
 * t_quadra.c - Testes unitários do módulo quadra
 */
#include "../unity/unity.h"
#include "../src/quadra.h"
#include "../src/hashfile.h"
#include <string.h>
#include <math.h>

void setUp(void) {}
void tearDown(void) {}

static Quadra make_q(void) {
    Quadra q;
    memset(&q, 0, sizeof(q));
    strncpy(q.cep, "cep01", sizeof(q.cep)-1);
    q.x = 10.0f; q.y = 20.0f;
    q.w = 100.0f; q.h = 50.0f;
    q.sw = 2.0f;
    strncpy(q.cfill, "orange", sizeof(q.cfill)-1);
    strncpy(q.cstrk, "black",  sizeof(q.cstrk)-1);
    return q;
}

void test_serialize_deserialize_roundtrip(void) {
    Quadra q = make_q();
    char buf[HF_DATA_SIZE];
    quadra_serialize(&q, buf, sizeof(buf));

    Quadra q2;
    quadra_deserialize(&q2, buf);
    TEST_ASSERT_EQUAL_STRING(q.cep, q2.cep);
    TEST_ASSERT_EQUAL_STRING(q.cfill, q2.cfill);
    TEST_ASSERT_EQUAL_STRING(q.cstrk, q2.cstrk);
}

void test_key_igual_ao_cep(void) {
    Quadra q = make_q();
    char key[HF_KEY_SIZE];
    quadra_key(&q, key, sizeof(key));
    TEST_ASSERT_EQUAL_STRING("cep01", key);
}

void test_anchor_canto_sudeste(void) {
    Quadra q = make_q();
    float ax, ay;
    quadra_anchor(&q, &ax, &ay);
    /* Âncora = (x+w, y+h) */
    TEST_ASSERT_TRUE(fabsf(ax - (q.x + q.w)) < 0.001f);
    TEST_ASSERT_TRUE(fabsf(ay - (q.y + q.h)) < 0.001f);
}

void test_face_N_dentro_da_quadra(void) {
    Quadra q = make_q();
    float ox, oy;
    quadra_face_pos(&q, 'N', &ox, &oy);
    TEST_ASSERT_TRUE(ox >= q.x && ox <= q.x + q.w);
    TEST_ASSERT_TRUE(oy >= q.y && oy <= q.y + q.h);
}

void test_face_S_dentro_da_quadra(void) {
    Quadra q = make_q();
    float ox, oy;
    quadra_face_pos(&q, 'S', &ox, &oy);
    TEST_ASSERT_TRUE(oy > q.y && oy <= q.y + q.h);
}

void test_face_L_dentro_da_quadra(void) {
    Quadra q = make_q();
    float ox, oy;
    quadra_face_pos(&q, 'L', &ox, &oy);
    TEST_ASSERT_TRUE(ox >= q.x);
}

void test_face_O_dentro_da_quadra(void) {
    Quadra q = make_q();
    float ox, oy;
    quadra_face_pos(&q, 'O', &ox, &oy);
    TEST_ASSERT_TRUE(ox <= q.x + q.w);
}

void test_serialize_contem_cep(void) {
    Quadra q = make_q();
    char buf[HF_DATA_SIZE];
    quadra_serialize(&q, buf, sizeof(buf));
    TEST_ASSERT_TRUE(strstr(buf, "cep01") != NULL);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_serialize_deserialize_roundtrip);
    RUN_TEST(test_key_igual_ao_cep);
    RUN_TEST(test_anchor_canto_sudeste);
    RUN_TEST(test_face_N_dentro_da_quadra);
    RUN_TEST(test_face_S_dentro_da_quadra);
    RUN_TEST(test_face_L_dentro_da_quadra);
    RUN_TEST(test_face_O_dentro_da_quadra);
    RUN_TEST(test_serialize_contem_cep);
    return UNITY_END();
}
