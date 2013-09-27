/**
 * print character code in hex digit
 */
#include <stdio.h>

int main() {
  int i;
  for (i=0x41;i<=0x66;i++) {
    printf("%x => %c\n" ,i ,i);
  }
}
