/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_DEC, TK_EQ,

  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"[0-9]+", TK_DEC},   // decimal
  {"\\+", '+'},         // plus
  {"-", '-'},           // minus
  {"\\*", '*'},         // multi
  {"/", '/'},           // divide
  {"\\(", '('},         // left bracket
  {"\\)", ')'},         // right bracke
  {"==", TK_EQ},        // equal
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        if (rules[i].token_type == TK_NOTYPE) {
          break;
        }

        switch (rules[i].token_type) {
          case TK_DEC:
          case TK_EQ:
            tokens[nr_token].type = rules[i].token_type;
            strncpy(tokens[nr_token].str, substr_start, substr_len);
        }
        ++nr_token;

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

bool check_parentheses(uint32_t p, uint32_t q) {
  if (tokens[p].type != '(' || tokens[q].type != ')') {
      return false;
  }

  int count = 0;
  for (uint32_t i = p + 1; i <= q - 1; i++) {
      if (tokens[i].type == '(') {
          count++;
      }
      else if (tokens[i].type == ')') {
          if (count == 0) return false;
          count--;
      }
  }

  return count == 0;
}

int get_precedence(int tk_type) {
  switch (tk_type) {
    case '*': case '/': return 2;
    case '+': case '-': return 1;
    default: panic("Unsupported token");
  }
  return 0;  // Should never reach here
}

int find_major(int p, int q) {
  int ret = 0, paren_cnt = 0, opt_type = 0;

  for (int i = p; i <= q; ++i) {
    int tk_type = tokens[i].type;

    if (tk_type == TK_DEC) {
      continue;
    }

    if (tk_type == '(') {
      ++paren_cnt;
    }
    else if (tk_type == ')') {
      if (paren_cnt == 0) {
        return -1;
      }
      --paren_cnt;
    }
    else {
      if (paren_cnt > 0) {
        continue;
      }

      int tmp_type = get_precedence(tk_type);

      if (tmp_type >= opt_type) {
        opt_type = tmp_type;
        ret = i;
      }
    }
  }

  if (paren_cnt != 0) {
    return -1;
  }
  return ret;
}

word_t eval(int p, int q, bool* success) {
  if (p > q) {
    *success = false;
    return 0;
  }
  else if (p == q) {
    if (tokens[p].type != TK_DEC) {
      *success = false;
      return 0;
    }
    return strtoul(tokens[p].str, NULL, 10);
  }
  else if (check_parentheses(p, q)) {
    return eval(p + 1, q - 1, success);
  }
  else {
    int major_pos = find_major(p, q);
    if (major_pos == -1) {
      *success = false;
      return 0;
    }

    word_t val1 = eval(p, major_pos - 1, success);
    if (*success == false) return 0;

    word_t val2 = eval(major_pos + 1, q, success);
    if (*success == false) return 0;

    switch (tokens[major_pos].type) {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/': if (val2 == 0) {
        *success = false;
        return 0;
      }
      return val1 / val2;
      default: panic("Unsupported opt");
    }
  }
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  return eval(0, nr_token - 1, success);
}
