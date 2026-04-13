/*
 * AFL++ Fuzzing Harness for FSG (Finite State Grammar) Parser
 *
 * Attack Vector: FSG file parsing
 * Target: src/lm/fsg_model.c
 *
 * FSG files define state machines for grammar-based recognition.
 * Format includes:
 * - State definitions
 * - Transitions
 * - Probabilities
 *
 * Potential vulnerabilities:
 * - State machine cycles
 * - Invalid state references
 * - Float parsing issues
 * - Graph construction bugs
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
    logmath_t *lmath = logmath_init(1.0001, 0, 0);

    while (__AFL_LOOP(10000)) {
        int len = __AFL_FUZZ_TESTCASE_LEN;

        /* Write fuzzer input to temporary file */
        char tmpfile[] = "/tmp/fuzz_fsg_XXXXXX";
        int fd = mkstemp(tmpfile);
        if (fd == -1) continue;

        write(fd, buf, len);
        close(fd);

        /* Try to parse FSG file */
        fsg_model_t *fsg = fsg_model_readfile(tmpfile, lmath, 1.0);

        if (fsg) {
            fsg_model_free(fsg);
        }

        unlink(tmpfile);
    }

    logmath_free(lmath);
    return 0;
}
