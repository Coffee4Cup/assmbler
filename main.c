#include <stdio.h>
#include "assmbler.h"
int main(int argc,char *argv[]) {
    FILE *input_file;
    input_file = fopen(argv[1], "r");

    assemble(input_file);
    return 0;
}
