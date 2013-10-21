/**
 * print character code in hex digit
 */
#include <stdio.h>

int main() {
  int i;
  for (i=0x20;i<=0x80;i++) {
    printf("%x => %c\n" ,i ,i);
  }
}
