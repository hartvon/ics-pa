#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <regex.h>

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

int main(int argc, char *argv[]) {

    char holder[] = "abc 123 def";
    const char* pattern = "^[a-z]+\\s[0-9]+";
    printf("-%s\n", reg_match(holder, pattern));

    return 0;
}
