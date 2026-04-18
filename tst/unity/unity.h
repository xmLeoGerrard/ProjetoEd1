/**
 * unity.h - Unity Test Framework (stub offline compatível com API real do Unity)
 * Fonte real: https://github.com/ThrowTheSwitch/Unity
 */
#ifndef UNITY_H
#define UNITY_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

extern int         unity_failures;
extern int         unity_tests;
extern const char *unity_current_test;

/* setUp e tearDown declarados pelo arquivo de teste */
extern void setUp(void);
extern void tearDown(void);

#define UNITY_BEGIN() \
    do { unity_failures = 0; unity_tests = 0; \
         write(2, "--- TEST RUN ---\n", 17); } while(0)

#define UNITY_END()  unity_end()

#define RUN_TEST(fn) \
    do { \
        unity_tests++; \
        unity_current_test = #fn; \
        printf("  RUN: %s\n", #fn); \
        fflush(stdout); \
        setUp(); \
        fn(); \
        tearDown(); \
    } while(0)

#define TEST_ASSERT_TRUE(cond) \
    _unity_assert((cond), "Expected TRUE: " #cond, __FILE__, __LINE__)

#define TEST_ASSERT_FALSE(cond) \
    _unity_assert(!(cond), "Expected FALSE: " #cond, __FILE__, __LINE__)

#define TEST_ASSERT_NULL(p) \
    _unity_assert((p)==NULL, #p " should be NULL", __FILE__, __LINE__)

#define TEST_ASSERT_NOT_NULL(p) \
    _unity_assert((p)!=NULL, #p " should NOT be NULL", __FILE__, __LINE__)

#define TEST_ASSERT_EQUAL_INT(exp,act) \
    _unity_assert_int((int)(exp),(int)(act), #act, __FILE__, __LINE__)

#define TEST_ASSERT_EQUAL_STRING(exp,act) \
    _unity_assert_str((exp),(act), #act, __FILE__, __LINE__)

#define TEST_ASSERT_EQUAL_UINT32(exp,act) \
    _unity_assert_int((int)(exp),(int)(act), #act, __FILE__, __LINE__)

void _unity_assert(int cond, const char *msg, const char *file, int line);
void _unity_assert_int(int exp, int act, const char *msg,
                       const char *file, int line);
void _unity_assert_str(const char *exp, const char *act, const char *msg,
                       const char *file, int line);
int  unity_end(void);

#endif /* UNITY_H */
