/*
 * AFL++ Fuzzing Harness for JSGF Grammar Parser
 *
 * Attack Vector: JSGF grammar file parsing
 * Target: src/lm/jsgf.c, jsgf_parser.c, jsgf_scanner.c
 *
 * JSGF (Java Speech Grammar Format) is a complex text format with
 * recursive grammar rules. Parser bugs could lead to:
 * - Buffer overflows
 * - Stack exhaustion
 * - Use-after-free
 * - Integer overflows
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pocketsphinx.h>

#ifdef __AFL_FUZZ_TESTCASE_LEN
/* AFL++ persistent mode */
__AFL_FUZZ_INIT();
#endif

int main(int argc, char **argv)
{
#ifdef __AFL_FUZZ_TESTCASE_LEN
    /* AFL++ mode - use persistent fuzzing */
#ifdef __AFL_HAVE_MANUAL_CONTROL
    __AFL_INIT();
#endif

    unsigned char *buf = __AFL_FUZZ_TESTCASE_BUF;

    while (__AFL_LOOP(10000)) {
        int len = __AFL_FUZZ_TESTCASE_LEN;

        /* Write fuzzer input to temporary file */
        char tmpfile[] = "/tmp/fuzz_jsgf_XXXXXX";
        int fd = mkstemp(tmpfile);
        if (fd == -1) continue;

        write(fd, buf, len);
        close(fd);

        /* Try to parse the JSGF grammar */
        jsgf_t *jsgf = jsgf_parse_file(tmpfile, NULL);

        /* Cleanup */
        if (jsgf) {
            jsgf_grammar_free(jsgf);
        }

        unlink(tmpfile);
    }
#else
    /* Standalone mode - read from stdin for crash reproduction */
    unsigned char buf[1024 * 1024];  /* 1MB max */
    int len = read(STDIN_FILENO, buf, sizeof(buf));

    if (len < 0) {
        perror("read");
        return 1;
    }

    /* Write fuzzer input to temporary file */
    char tmpfile[] = "/tmp/fuzz_jsgf_XXXXXX";
    int fd = mkstemp(tmpfile);
    if (fd == -1) {
        perror("mkstemp");
        return 1;
    }

    write(fd, buf, len);
    close(fd);

    /* Try to parse the JSGF grammar */
    jsgf_t *jsgf = jsgf_parse_file(tmpfile, NULL);

    /* Cleanup */
    if (jsgf) {
        jsgf_grammar_free(jsgf);
    }

    unlink(tmpfile);
#endif

    return 0;
}
