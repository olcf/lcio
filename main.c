

#include <stdio.h>
#include "conf_parser.h"
#include "lcio.h"


int main(int argc, char** argv) {
    char *name;
    struct conf* cfg;
    lcio_param_t* params;

    if (argc == 1) {
        name = strdup("./test.ini");
    } else {
        name = argv[1];
    }
    cfg = parse_conf_file(name);
    params = fill_parameters(cfg);
    print_cfg(cfg);

    file_test(params->jobs[0]);

    exit(0);
}
