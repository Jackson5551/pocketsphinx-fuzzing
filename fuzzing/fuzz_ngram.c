/*
 * AFL++ Fuzzing Harness for N-gram Language Model Parser
 *
 * Attack Vector: Language model file parsing (ARPA, DMP, BIN formats)
 * Target: src/lm/ngram_model.c, ngram_model_trie.c
 *
 * N-gram language models can be in multiple formats:
 * - ARPA (text-based)
 * - DMP/BIN (binary format)
 * - Compressed variants (.gz, .bz2)
 *
 * Potential vulnerabilities:
 * - Integer overflows in size calculations
 * - Buffer overflows in string parsing
 * - Format confusion attacks
 * - Memory exhaustion
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pocketsphinx.h>

__AFL_FUZZ_INIT();

int main(int argc, char **argv)
{
#ifdef __AFL_HAVE_MANUAL_CONTROL
    __AFL_INIT();
#endif

    unsigned char *buf = __AFL_FUZZ_TESTCASE_BUF;

    /* Create minimal config */
    ps_config_t *config = ps_config_init(NULL);
    logmath_t *lmath = logmath_init(1.0001, 0, 0);

    while (__AFL_LOOP(10000)) {
        int len = __AFL_FUZZ_TESTCASE_LEN;

        /* Write fuzzer input to temporary file */
        char tmpfile[] = "/tmp/fuzz_ngram_XXXXXX";
        int fd = mkstemp(tmpfile);
        if (fd == -1) continue;

        write(fd, buf, len);
        close(fd);

        /* Try to parse as different language model formats */
        ngram_model_t *lm = NULL;

        /* Try ARPA format */
        lm = ngram_model_read(config, tmpfile, NGRAM_ARPA, lmath);
        if (lm) {
            ngram_model_free(lm);
        }

        /* Try BIN format */
        lm = ngram_model_read(config, tmpfile, NGRAM_BIN, lmath);
        if (lm) {
            ngram_model_free(lm);
        }

        /* Try auto-detection */
        lm = ngram_model_read(config, tmpfile, NGRAM_AUTO, lmath);
        if (lm) {
            ngram_model_free(lm);
        }

        unlink(tmpfile);
    }

    logmath_free(lmath);
    ps_config_free(config);
    return 0;
}
