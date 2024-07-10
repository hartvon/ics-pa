#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <regex.h>
#include <stdbool.h>
#include <stdint.h>

char* reg_match(char* holder, const char* pattern) {
    regex_t re;
    regmatch_t pmatch;
    char buf[1024];
    int ret;

    if (ret = regcomp(&re, pattern, REG_EXTENDED)) {
        regerror(ret, &re, buf, sizeof(buf));
        fprintf(stderr, "regcomp failed: %s\n", buf);
        exit(EXIT_FAILURE);
    }

    if (ret = regexec(&re, holder, 1, &pmatch, 0)) {
        regerror(ret, &re, buf, sizeof(buf));
        fprintf(stderr, "regexec failed: %s\n", buf);
        exit(EXIT_FAILURE);
    }

    char* matched_substr = holder + pmatch.rm_so;
    holder[pmatch.rm_eo] = '\0';

    regfree(&re);

    return matched_substr;
}

bool check_parentheses(char* e, uint32_t p, uint32_t q) {
  assert(e && p < q);

  if (e[p] != '(' || e[q] != ')') {
      return false;
  }

  int count = 0;
  for (uint32_t i = p + 1; i <= q - 1; i++) {
      if (e[i] == '(') {
          count++;
      } else if (e[i] == ')') {
          if (count == 0) return false;
          count--;
      }
  }

  return count == 0;
}

void test_reg_match() {
    char holder[] = "abc 123 def";
    const char* pattern = "^[a-z]+\\s[0-9]+";
    printf("-%s\n", reg_match(holder, pattern));
}

void test_check_parentheses() {
    char expr1[] = "(2 - 1)";
    char expr2[] = "(4 + 3 * (2 - 1))";
    char expr3[] = "4 + 3 * (2 - 1)";
    char expr4[] = "(4 + 3)) * ((2 - 1)";
    char expr5[] = "(4 + 3) * (2 - 1)";

    assert(check_parentheses(expr1, 0, strlen(expr1) - 1));
    assert(check_parentheses(expr2, 0, strlen(expr2) - 1));
    assert(!check_parentheses(expr3, 0, strlen(expr3) - 1));
    assert(!check_parentheses(expr4, 0, strlen(expr4) - 1));
    assert(!check_parentheses(expr5, 0, strlen(expr5) - 1));
}

void test_common() {
    char str[] = "hello";
    char* str1 = "world";
    printf("%lu, %lu\n", strlen(str), sizeof(str));
    printf("%lu, %lu\n", strlen(str1), sizeof(str1));
}

int main(int argc, char *argv[]) {
    // test_reg_match();
    test_check_parentheses();
    // test_common();

    return 0;
}
