#include "unity.h"

int      unity_failures    = 0;
int      unity_tests       = 0;
const char *unity_current_test = "";

void _unity_assert(int cond, const char *msg, const char *file, int line) {
    if (!cond) {
        unity_failures++;
        printf("  FAIL (%s:%d): %s\n", file, line, msg);
    }
}

void _unity_assert_int(int exp, int act, const char *msg,
                       const char *file, int line) {
    if (exp != act) {
        unity_failures++;
        printf("  FAIL (%s:%d): %s  expected=%d  actual=%d\n",
               file, line, msg, exp, act);
    }
}

void _unity_assert_str(const char *exp, const char *act, const char *msg,
                       const char *file, int line) {
    if (!exp || !act || strcmp(exp, act) != 0) {
        unity_failures++;
        printf("  FAIL (%s:%d): %s  expected='%s'  actual='%s'\n",
               file, line, msg,
               exp ? exp : "(null)",
               act ? act : "(null)");
    }
}

int unity_end(void) {
    printf("-----------------------\n");
    printf("%d Tests  %d Failures\n", unity_tests, unity_failures);
    if (unity_failures == 0) printf("OK\n");
    return unity_failures;
}
