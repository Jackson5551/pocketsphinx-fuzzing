/* Simple test harness to reproduce crashes outside AFL++ */
#include <stdio.h>
#include <stdlib.h>
#include <pocketsphinx.h>

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <jsgf_file>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    printf("Parsing JSGF file: %s\n", filename);

    jsgf_t *jsgf = jsgf_parse_file(filename, NULL);

    if (jsgf) {
        printf("Parse successful!\n");
        jsgf_grammar_free(jsgf);
    } else {
        printf("Parse failed\n");
    }

    return 0;
}
