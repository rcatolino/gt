#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char *strtok_re(char *str, const char *delim, char **saveptr);

int main(int argc, char *argv[]) {

  char *saveptr = NULL;
  char test_strs[][30] = {
    "test test",
    "test\\ test",
    "test\\\\ test",
    "",
    "\\test",
    " ",
    "\\",
    "test\\test ",
    "test \\",
    "test\\ ",
    "test \\\\",
    "test\\\\ ",
    "test\\",
    "test test 1",
    "testa1 2"
  };

  char *result[][2] = {
    {  "test", "test" },
    {  "test\\ test", NULL },
    {  "test\\\\", "test" },
    {  "", NULL },
    {  "\\test", NULL },
    {  "", "" },
    {  "\\", NULL },
    {  "test\\test", ""},
    {  "test", "\\" },
    {  "test\\ ", NULL },
    {  "test", "\\\\" },
    {  "test\\\\", "" },
    {  "test\\", NULL },
    {  "test", "test" },
    {  "test", "1" }
  };

  for (int i = 0; i < sizeof(test_strs)/sizeof(test_strs[0]); i++) {
    printf("testing '%s'\n", test_strs[i]);
    char *res = strtok_re(test_strs[i], "\\ a", &saveptr);
    if (strcmp(res, result[i][0]) != 0) {
      printf("Error in test %d, first token. Found '%s', expected '%s'\n",
             i, res, result[i][0]);
      return 1;
    }
    res = strtok_re(NULL, "\\ a", &saveptr);
    if (res != NULL && (result[i][1] == NULL || strcmp(res, result[i][1]) != 0)) {
      printf("Error in test %d, second token. Found '%s', expected '%s'\n",
             i, res, result[i][1]);
      return 1;
    }
  }

  return 0;
}
