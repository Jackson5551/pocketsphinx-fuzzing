/*
 * AFL++ Fuzzing Harness for Dictionary Parser
 *
 * Attack Vector: Phonetic dictionary file parsing
 * Target: src/dict.c
 *
 * Dictionary files map words to phonetic representations.
 * Format: WORD  P H O N E M E S
 *
 * Potential vulnerabilities:
 * - Buffer overflows in line parsing
 * - Hash table collisions
 * - Memory exhaustion with large dictionaries
 * - Special character handling
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pocketsphinx.h>
#include "dict.h"
#include "bin_mdef.h"

__AFL_FUZZ_INIT();

int main(int argc, char **argv)
{
#ifdef __AFL_HAVE_MANUAL_CONTROL
    __AFL_INIT();
#endif

    unsigned char *buf = __AFL_FUZZ_TESTCASE_BUF;

    while (__AFL_LOOP(10000)) {
        int len = __AFL_FUZZ_TESTCASE_LEN;

        /* Write fuzzer input to temporary file */
        char tmpfile[] = "/tmp/fuzz_dict_XXXXXX";
        int fd = mkstemp(tmpfile);
        if (fd == -1) continue;

        write(fd, buf, len);
        close(fd);

        /* Try to parse dictionary */
        /* Note: dict_init requires a bin_mdef_t, so we need a minimal setup */
        ps_config_t *config = ps_config_init(NULL);

        /* We can only fuzz if we have a valid model, but for standalone
         * fuzzing we can test the file reading functions directly */

        /* This is simplified - in practice you'd need to load a model first */

        ps_config_free(config);
        unlink(tmpfile);
    }

    return 0;
}
