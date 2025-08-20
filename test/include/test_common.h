#ifndef __TEST_COMMON_H__
#define __TEST_COMMON_H__

#include "unity.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// Common test macros
#define TEST_ASSERT_ARRAY_EQUAL(expected, actual, length) \
    for (size_t i = 0; i < (length); i++) { \
        TEST_ASSERT_EQUAL_HEX8((expected)[i], (actual)[i]); \
    }

// Test utilities
void setUp(void);
void tearDown(void);

#endif /* __TEST_COMMON_H__ */