#include <CUnit/Basic.h>

#define CHECK_RESULT(error_condition) \
  if(error_condition) { \
    CU_cleanup_registry(); \
    return CU_get_error(); \
  }

#define DECLARE_SUITE(suite) \
  CU_pSuite suite_##suite = NULL;

#define ADD_SUITE(suite) \
  suite_##suite = CU_add_suite(#suite, suite_##suite##_init, suite_##suite##_clean); \
  CHECK_RESULT(suite_##suite == NULL);

#define ADD_TEST(suite, test) \
  CHECK_RESULT(CU_add_test(suite_##suite, #test, suite_##suite##_test_##test) == NULL);


int suite_autodiscover_init() {
  return 0;
}
int suite_autodiscover_clean() {
  return 0;
}
void suite_autodiscover_test_foo() {
  CU_ASSERT_PTR_NOT_NULL((void*)0x100);
}


int main() {
   DECLARE_SUITE(autodiscover);

   /* initialize the CUnit test registry */
   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();

   /* add a suite to the registry */
   ADD_SUITE(autodiscover);
   ADD_TEST(autodiscover, foo);

   /* run tests */
   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();

   /* clean up */
   CU_cleanup_registry();
   return CU_get_error();
}
