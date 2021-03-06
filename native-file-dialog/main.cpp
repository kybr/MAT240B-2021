#include <stdio.h>
#include <stdlib.h>

#include "nfd.h"

int main(void) {
  nfdchar_t *outPath = NULL;
  nfdresult_t result = NFD_OpenDialog(NULL, NULL, &outPath);

  if (result == NFD_OKAY) {
    puts("Success!");
    puts(outPath);
    free(outPath);
  } else if (result == NFD_CANCEL) {
    puts("User pressed cancel.");
  } else {
    printf("Error: %s\n", NFD_GetError());
  }

  return 0;
}
