#define _GNU_SOURCE

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef _MSC_VER
# include "../../../src/bsd/getopt.h"
#else
# include <getopt.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <CUnit/Basic.h>

#include "test_support.h"

#include "oxws.h"
#include "autodiscover.h"
#include "find_item.h"
#include "create_item.h"


void print_usage() {
  fprintf(stderr, "usage: test_oxws [options] (suite|test)*\n\n");
  fprintf(stderr, "options:\n" \
                  "    --ssl-cert  -c [FILE]   Server certificate to trust (optional).\n" \
                  "    --list      -l          List available tests.\n" \
                  "    --help      -? -h       Show this help.\n\n" \
                  "Identify a suite by its name. Identify a test with \"<suite>.<test>\".\n" \
                  "Omitting suite or test names will run all tests of all suites.\n");
}

void list_suites_and_tests() {
  printf("Available tests:\n");
  unsigned int ui_suite, ui_test;
  for(ui_suite = 1; ui_suite <= CU_get_registry()->uiNumberOfSuites; ui_suite++) {
    CU_pSuite suite = CU_get_suite_at_pos(ui_suite);
    printf("\n");
    for(ui_test = 1; ui_test <= suite->uiNumberOfTests; ui_test++)
      printf("%s.%s\n", suite->pName, CU_get_test_at_pos(suite, ui_test)->pName);
  }
}

unsigned int parse_options(int argc, char* const* argv, char** ca_file) {

  static const char* short_options = "c:lh?";
#if HAVE_GETOPT_LONG
  static struct option long_options[] = {
    {"ssl-cert",  1, NULL, 'c'},
    {"list",      0, NULL, 'l'},
    {"help",      0, NULL, '?'},
    {"help",      0, NULL, 'h'},
    {NULL,        0, NULL, 0},
  };
#endif
  int r = 0;

  if(ca_file == NULL) {
    fprintf(stderr, "parse_options: internal error\n");
    exit(EXIT_FAILURE);
  }

  *ca_file = NULL;

  while (r != -1) {
#if HAVE_GETOPT_LONG
    r = getopt_long(argc, argv, short_options, long_options, NULL);
#else
    r = getopt(argc, argv, short_options);
#endif
    switch (r) {
    case 'c': /* ssl-cert */
      *ca_file = strdup(optarg);
      break;
    case 'l': /* list */
      list_suites_and_tests();
      exit(EXIT_SUCCESS);

    case '?': /* help */
    case 'h': /* help */
      print_usage();
      exit(EXIT_FAILURE);

    case -1: /* no more arguments */
      break;

    default:  /* invalid option */
      fprintf(stderr, "\n");
      print_usage();
      exit(EXIT_FAILURE);
    }
  }

  return optind;
}

#define OXWS_TEST_ARG_ERROR(message) { fprintf(stderr, message); continue; }
#define OXWS_TEST_ARG_ERROR_1(message, arg1) { fprintf(stderr, message, arg1); continue; }
#define OXWS_TEST_ARG_ERROR_2(message, arg1, arg2) { fprintf(stderr, message, arg1, arg2); continue; }

void parse_arguments(int argc, const char** argv) {
  if(argc <= 0) {
    /* run all tests of all suites */
    return;
  }

  /* selective runs: deactivate all tests of all suites */
  unsigned int ui_suite, ui_test;
  for(ui_suite = 1; ui_suite <= CU_get_registry()->uiNumberOfSuites; ui_suite++) {
    CU_pSuite suite = CU_get_suite_at_pos(ui_suite);
    for(ui_test = 1; ui_test <= suite->uiNumberOfTests; ui_test++)
      CU_set_test_active(CU_get_test_at_pos(suite, ui_test), CU_FALSE);
  }

  /* activate selected tests */
  int i;
  for(i = 0; i < argc; i++) {
    const char* pos = strchr(argv[i], '.');
    if(pos == NULL) {
      /* suite name: activate all tests of this suite */
      if(argv[i][0] == 0) OXWS_TEST_ARG_ERROR("warning: ignoring empty suite name.");
      CU_pSuite suite = CU_get_suite(argv[i]);
      if(suite == NULL) OXWS_TEST_ARG_ERROR_1("warning: ignoring unknown suite '%s'.", argv[i]);
      for(ui_test = 1; ui_test <= suite->uiNumberOfTests; ui_test++)
        CU_set_test_active(CU_get_test_at_pos(suite, ui_test), CU_TRUE);
    } else {
      /* test name: activate the test only */
      /*   find suite */
      if(pos == argv[i]) OXWS_TEST_ARG_ERROR_1("warning: ignoring empty suite name of argument '%s'.", argv[i]);
      char* suite_name = alloca(sizeof(char) * (pos - argv[i] + 1));
      strncpy(suite_name, argv[i], pos - argv[i]);
      suite_name[pos - argv[i]] = 0;
      CU_pSuite suite = CU_get_suite(suite_name);
      if(suite == NULL) OXWS_TEST_ARG_ERROR_1("warning: ignoring unknown suite '%s'.", suite_name);
      /* find test */
      const char* test_name = pos + 1;
      if(test_name[0] == 0) OXWS_TEST_ARG_ERROR_1("warning: ignoring empty test name of argument '%s'.", argv[i]);
      CU_pTest test = CU_get_test(suite, test_name);
      if(test == NULL) OXWS_TEST_ARG_ERROR_2("warning: ignoring unknown test '%s.%s'.", suite_name, test_name);
      /* activate it */
      CU_set_test_active(test, CU_TRUE);
    }
  }
}

#undef OXWS_TEST_ARG_ERROR
#undef OXWS_TEST_ARG_ERROR_1
#undef OXWS_TEST_ARG_ERROR_2


int main(int argc, const char** argv) {
  /* initialize the CUnit test registry */
  if (CUE_SUCCESS != CU_initialize_registry())
    return CU_get_error();

  /* add suites to the registry */
  OXWS_TEST_ADD_SUITE(oxws);
  OXWS_TEST_ADD_SUITE(autodiscover);
  OXWS_TEST_ADD_SUITE(find_item);
  OXWS_TEST_ADD_SUITE(create_item);

  /* parse options */
  unsigned int argv_entries_parsed = parse_options(argc, (char* const*) argv, &oxws_test_support_ca_file);
  /* parse arguments: activate / deactivate tests for selective test run */
  parse_arguments(argc - argv_entries_parsed, argv + argv_entries_parsed);

  /* run tests */
  CU_basic_set_mode(CU_BRM_VERBOSE);
  CU_basic_run_tests();

  /* clean up */
  CU_cleanup_registry();
  return CU_get_error();
}
