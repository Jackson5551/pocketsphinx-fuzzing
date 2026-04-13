/* Simple test harness to reproduce JSGF crashes without AFL */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pocketsphinx.h>

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <crash_file>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];

    /* Read the crash file */
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        perror("fopen");
        return 1;
    }

    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    unsigned char *buf = malloc(len);
    if (!buf) {
        perror("malloc");
        fclose(fp);
        return 1;
    }

    fread(buf, 1, len, fp);
    fclose(fp);

    /* Write to temp file like the fuzzer does */
    char tmpfile[] = "/tmp/test_jsgf_XXXXXX";
    int fd = mkstemp(tmpfile);
    if (fd == -1) {
        perror("mkstemp");
        free(buf);
        return 1;
    }

    write(fd, buf, len);
    close(fd);
    free(buf);

    printf("Testing JSGF file: %s\n", tmpfile);
    printf("Calling jsgf_parse_file()...\n");

    /* Try to parse the JSGF grammar */
    jsgf_t *jsgf = jsgf_parse_file(tmpfile, NULL);

    if (jsgf) {
        printf("Parse successful!\n");
        jsgf_grammar_free(jsgf);
    } else {
        printf("Parse failed (returned NULL)\n");
    }

    unlink(tmpfile);
    printf("Done.\n");

    return 0;
}
