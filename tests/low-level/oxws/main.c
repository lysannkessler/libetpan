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
#include "autodiscover.h"


int parse_options(int argc, char** argv,
      char** ca_file) {

  static const char* short_options = "c";
#if HAVE_GETOPT_LONG
  static struct option long_options[] = {
    {"ca-file",  1, 0, 'c'},
  };
#endif
  int r = 0;

  if(ca_file == NULL)
    return -1;

  *ca_file = NULL;

  while (r != -1) {
#if HAVE_GETOPT_LONG
    r = getopt_long(argc, argv, short_options, long_options, NULL);
#else
    r = getopt(argc, argv, short_options);
#endif
    switch (r) {
    case 'c': *ca_file = strdup(optarg); break;

    case -1: break;
    default: fprintf(stderr, "error: unknown argument %c.\n", r); break;
    }
  }

  return 0;
}

void print_usage() {
  fprintf(stderr, "usage: test_oxws [options]\n\n");
  fprintf(stderr, "options:\n" \
                  "    --ssl-cert  -c [FILE]   Server certificate to trust (optional).\n");
}


int main(int argc, char** argv) {
  DECLARE_SUITE(autodiscover);

  int r = parse_options(argc, argv, &oxws_test_support_ca_file);
  if(r < 0) {
    fprintf(stderr, "parse_options: internal error\n");
    exit(EXIT_FAILURE);
  } else if(r > 0) {
    print_usage();
    exit(EXIT_FAILURE);
  }

  /* initialize the CUnit test registry */
  if (CUE_SUCCESS != CU_initialize_registry())
    return CU_get_error();

  /* add suites to the registry */
  ADD_SUITE(autodiscover);

  /* run tests */
  CU_basic_run_tests();

  /* clean up */
  CU_cleanup_registry();
  return CU_get_error();
}
