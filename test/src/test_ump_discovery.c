#include "test_common.h"
#include "ump_discovery.h"
#include <string.h>

void setUp(void) {
}

void tearDown(void) {
}

void test_MIDICI_GenerateMUID_UniqueValues(void) {
    muid_t muid1 = MIDICI_GenerateMUID();
    muid_t muid2 = MIDICI_GenerateMUID();
    muid_t muid3 = MIDICI_GenerateMUID();
    
    TEST_ASSERT_NOT_EQUAL(0, muid1);
    TEST_ASSERT_NOT_EQUAL(0, muid2);
    TEST_ASSERT_NOT_EQUAL(0, muid3);
    
    TEST_ASSERT_NOT_EQUAL(muid1, muid2);
    TEST_ASSERT_NOT_EQUAL(muid2, muid3);
    TEST_ASSERT_NOT_EQUAL(muid1, muid3);
}

void test_MIDICI_GenerateMUID_ValidRange(void) {
    muid_t muid;
    
    for (int i = 0; i < 10; i++) {
        muid = MIDICI_GenerateMUID();
        
        TEST_ASSERT_NOT_EQUAL(0, muid);
        TEST_ASSERT_TRUE(muid <= 0x0FFFFFFF);
    }
}

void test_MIDICI_GenerateMUID_NotBroadcast(void) {
    muid_t muid;
    const muid_t BROADCAST_MUID = 0x0FFFFFFF;
    
    for (int i = 0; i < 100; i++) {
        muid = MIDICI_GenerateMUID();
        TEST_ASSERT_NOT_EQUAL(BROADCAST_MUID, muid);
    }
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_MIDICI_GenerateMUID_UniqueValues);
    RUN_TEST(test_MIDICI_GenerateMUID_ValidRange);
    RUN_TEST(test_MIDICI_GenerateMUID_NotBroadcast);
    
    return UNITY_END();
}