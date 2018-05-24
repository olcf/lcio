//
// Created by Matthew Bachstein on 5/23/18.
//

#include "conf_parser.h"

void print_cfg(struct conf* cfg){
    int i,j;

    fprintf(stderr, "%s\nCONTENTS:\n", cfg->file_name);
    fprintf(stderr, "  num_sections: %d \n", cfg->num);
    for(i=0; i < cfg->num; ++i){
        fprintf(stderr, "  %s: num attrs %d\n", cfg->sections[i]->key, cfg->sections[i]->num);
        for(j=0; j < cfg->sections[i]->num; ++j){
            fprintf(stderr, "    %s = %s\n", cfg->sections[i]->attrs[j]->key, cfg->sections[i]->attrs[j]->val);
        }
    }
    fprintf(stderr, "\nEND\n");
}

void cfg_read_attr(char* buf, struct section* s, int num){
    struct attr *a;

    a = malloc(sizeof(struct attr));
    a->key = malloc(sizeof(char) * 64);
    a->val = malloc(sizeof(char) * 64);

    sscanf(buf, "%[^= ] = %s", a->key, a->val);
    s->attrs[num] = a;
}

void cfg_read_section(struct conf *cfg, char* name, int sec_num, FILE* fd){
    struct section *sec;
    off_t pos;
    char* buf;
    int count = 0;

    buf = malloc(sizeof(char) * 128);

    sec = malloc(sizeof(struct section));
    sec->key = strdup(name);

    //get current position in file. Know that it is at the first line after the section header
    pos = ftell(fd);

    while (1){
        if(fgets(buf, 128, fd) == NULL) break;
        if(buf[0] == '[') break;
        if(buf[0] == '#' || buf[0] == ';') continue;
        if(isalnum(buf[0]) == 0) continue;
        count += 1;
    }
    fseek(fd, pos, SEEK_SET);
    //have found the number of attrs in this section, now read them
    sec->attrs = malloc(sizeof(struct attr*) * count);
    sec->num = count;
    count = 0;
    while (fgets(buf, 128, fd) != NULL){
        if(buf[0] == '[') break;
        if(buf[0] == '#' || buf[0] == ';') continue;
        if(isalnum(buf[0]) == 0) continue;
        cfg_read_attr(buf, sec, count);
        count += 1;
    }
    fseek(fd, pos, SEEK_SET);
    cfg->sections[sec_num] = sec;

}

void cfg_alloc_sections(struct conf *cfg){
    int i;
    cfg->sections = malloc(sizeof(struct section*) * cfg->num);
    for(i = 0; i < cfg->num; ++i){
        cfg->sections[i] = malloc(sizeof(struct section));
    }
}

struct section* get_section(char* section_name, struct conf* cfg){
    int i;

    for(i=0; i < cfg->num; ++i){
        if(!strcmp(section_name, cfg->sections[i]->key)) return cfg->sections[i];
    }
    return NULL;
}

char* get_attr(char* key, struct section* sec){
    int i;

    for(i=0; i < sec->num; ++i){
        if(!strcmp(key, sec->attrs[i]->key)) return sec->attrs[i]->val;
    }
    return NULL;
}

struct conf* parse_conf_file(char *name){
    FILE* fd;
    struct conf *cfg;
    char* buf;
    char* sec_name;

    sec_name = malloc(sizeof(char) * 64);
    buf = malloc(sizeof(char) * 128); // buffer size of 128 for attr field in INI file
    cfg = malloc(sizeof(struct conf));

    fd = fopen(name, "r");
    int num_sections = 0;

    while(fgets(buf, 128, fd) != NULL){
        if(buf[0] == '[') {
            num_sections += 1;
        }
    }
    rewind(fd);
    cfg->num = num_sections;
    cfg->file_name = strdup(name);
    num_sections = 0;
    cfg_alloc_sections(cfg);
    while(fgets(buf, 128, fd) != NULL){
        //if(buf[0] == '#' || buf[0] == ';') continue;
        if (buf[0] == '['){
            sscanf(buf, "[%[^][]]", sec_name);
            cfg_read_section(cfg, sec_name, num_sections, fd);
            num_sections += 1;
        }
    }

    return cfg;
}
