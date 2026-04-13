/*
 * AFL++ Fuzzing Harness for Audio File Parsers
 *
 * Attack Vector: WAV and NIST SPHERE audio file parsing
 * Target: src/util/soundfiles.c
 *
 * Audio file parsing involves:
 * - Header validation
 * - Format detection (WAV, NIST)
 * - Chunk parsing
 * - Sample rate/channel configuration
 *
 * Potential vulnerabilities:
 * - Buffer overflows in header parsing
 * - Integer overflows in size calculations
 * - Format confusion
 * - Unbounded memory allocation
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

    while (__AFL_LOOP(10000)) {
        int len = __AFL_FUZZ_TESTCASE_LEN;

        /* Write fuzzer input to temporary file */
        char tmpfile[] = "/tmp/fuzz_audio_XXXXXX";
        int fd = mkstemp(tmpfile);
        if (fd == -1) continue;

        write(fd, buf, len);
        close(fd);

        /* Try to parse as audio file */
        ps_config_t *config = ps_config_init(NULL);
        FILE *fh = fopen(tmpfile, "rb");

        if (fh) {
            /* This will detect and parse WAV/NIST headers */
            ps_config_soundfile(config, fh, tmpfile);
            fclose(fh);
        }

        ps_config_free(config);
        unlink(tmpfile);
    }

    return 0;
}
