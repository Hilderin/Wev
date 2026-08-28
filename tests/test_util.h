#ifndef WEV_TEST_UTIL_H
#define WEV_TEST_UTIL_H

#include <stdio.h>
#include <string.h>

#define WEV_MAX_TESTS 1024

typedef void (*TestFn)(void);

typedef struct
{
    const char* name;
    TestFn fn;
} TestEntry;

static TestEntry test_registry[WEV_MAX_TESTS];
static int test_count = 0;

static int test_asserts_run = 0;
static int test_asserts_failed = 0;

#define ASSERT_TRUE(expr)                                                   \
    do                                                                      \
    {                                                                       \
        test_asserts_run++;                                                 \
        if (!(expr))                                                        \
        {                                                                   \
            test_asserts_failed++;                                          \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        }                                                                   \
    } while (0)

#define ASSERT_EQ(actual, expected) ASSERT_TRUE((actual) == (expected))

#define ASSERT_STR_EQ(actual, expected) ASSERT_TRUE(strcmp((actual), (expected)) == 0)

// Registers the test function in the registry at startup and defines it.
#define TEST(test_name)                                    \
    static void test_name(void);                           \
    __attribute__((constructor)) static void               \
    test_name##_register(void)                             \
    {                                                      \
        if (test_count < WEV_MAX_TESTS)                    \
        {                                                  \
            test_registry[test_count].name = #test_name;   \
            test_registry[test_count].fn = test_name;      \
            test_count++;                                  \
        }                                                  \
    }                                                      \
    static void test_name(void)

// Generates the main that runs every registered test.
#define RUN_ALL_TESTS()                                             \
    int main(void)                                                  \
    {                                                               \
        for (int i = 0; i < test_count; i++)                        \
        {                                                           \
            const int fails_before = test_asserts_failed;           \
            test_registry[i].fn();                                  \
            if (test_asserts_failed > fails_before)                 \
            {                                                       \
                printf("FAIL %s\n", test_registry[i].name);         \
            }                                                       \
            else                                                    \
            {                                                       \
                printf("PASS %s\n", test_registry[i].name);         \
            }                                                       \
        }                                                           \
        if (test_asserts_failed > 0)                                \
        {                                                           \
            printf("%d/%d assertions failed\n",                     \
                   test_asserts_failed, test_asserts_run);          \
            return 1;                                               \
        }                                                           \
        printf("All %d assertions passed\n", test_asserts_run);     \
        return 0;                                                   \
    }

#endif //WEV_TEST_UTIL_H
