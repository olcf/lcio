#include <stdio.h>
#include "conf_parser.h"


int main(int argc, char** argv) {
    char *name;
    if (argc == 1) name = strdup("./test.ini");
    parse_ini_file(name);

    exit(0);
}