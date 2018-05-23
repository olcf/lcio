#include <stdio.h>
#include "conf_parser.h"


int main(int argc, char** argv) {
    char *name;
    struct conf* cfg;

    if (argc == 1) {
        name = strdup("./test.ini");
    } else {
        name = argv[1];
    }
    cfg = parse_conf_file(name);
    print_cfg(cfg);

    exit(0);
}