/*
 * Copyright (c) by CryptoLab inc.
 * This program is licensed under a
 * Creative Commons Attribution-NonCommercial 3.0 Unported License.
 * You should have received a copy of the license along with this
 * work.  If not, see <http://creativecommons.org/licenses/by-nc/3.0/>.
 */

#include "../src/TestScheme.h"
#include <getopt.h>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vector>

using namespace std;

int main(int argc, const char *argv[]) {

  int opt;
  int optind;
  struct test_conf_t {
    bool need_check = false;
    long logN = -1;
    string case_name;
    long nr_thread = -1;
  } global_conf;

  decltype(TestScheme::testEncodeBatch) *case_ptr;
  int test_num = -1;

  struct option longopts[] = {{"check", 0, NULL, 'c'},
                              {"case", 1, NULL, 'k'},
                              {"logN", 1, NULL, 'N'},
                              {"nr_thread", 1, NULL, 't'}};
  const char *optstring = "ck:N:t:";

  while ((opt = getopt_long(argc, (char *const *)argv, optstring, longopts,
                            &optind)) != -1) {
    switch (opt) {
    case 'c': {
      global_conf.need_check = true;
      break;
    }
    case 'k': {
      global_conf.case_name = string(optarg);
      break;
    }
    case 'N': {
      global_conf.logN = strtol(optarg, NULL, 10);
      break;
    }
    case 't': {
      global_conf.nr_thread = strtol(optarg, NULL, 10);
      // printf("%s\n", optarg);
      break;
    }
    default: {
      printf("invalid option %c", opt);
      exit(1);
    }
    }
  }

  cout << "test case: " << global_conf.case_name << endl
       << (global_conf.need_check ? "with check\n" : "")
       << "Threads Num: " << global_conf.nr_thread << endl
       << "logN: " << global_conf.logN << endl;

  void testCase(bool need_check, long logN, string case_name, long nr_thread);
  testCase(global_conf.need_check, global_conf.logN, global_conf.case_name,
           global_conf.nr_thread);

  return 0;
}
