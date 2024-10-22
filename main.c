#include <stdint.h>

struct kermit_pkg { 
  uint8_t mark;        
  union {
    uint16_t pkg; /* size, seq, type */
    uint8_t half[2];
   };
   uint8_t data[64];
   uint8_t crc;
};

void client_backup(char *pathname);

/* --- Client --- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INPUT_BUFFER_SIZE 8 + 1 + 63 

char input_buffer[INPUT_BUFFER_SIZE];
const char *DELIM = " ";

int main() {
  char *pathname;

  for(;;) {
    puts("Comand: ");

    fgets(&input_buffer[0], INPUT_BUFFER_SIZE, stdin);
    strtok(&input_buffer[0], DELIM);

    pathname = strtok(NULL, "\n");

    switch(*input_buffer) {
      case 'b':
        client_backup(pathname); break;
      case 'r':
        puts("restaura"); break;
      case 'v':
        puts("verifica"); break;
    }

  }
}


void client_backup(char *pathname) {
  FILE *file;
  struct kermit_pkg pkg;
  size_t size;

  puts("bkp start");

  int i = 0;

  if(file = fopen(pathname, "r")) {
    while(size = fread(pkg.data, sizeof(uint8_t), 64, file)) {
      pkg.pkg = size;
      printf("%d -> %d\n", ++i, pkg.pkg);

      //send(pkg)

    }

    fclose(file);
  } 

  puts("bkp End");

}

