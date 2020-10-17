#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <crypt.h>

#include "log.h"
#include "defines.h"

#define DICTIONARY_DIMENSION 1000
#define OUTPUT_PWD_FILENAME "dictionary.txt"
#define OUTPUT_FILENAME "dictionary_hash.txt"

char generated_list[DICTIONARY_DIMENSION][PWD_DIMENSION];


char random_alnum() {
    // The "-1" is necessary otherwise the string ending can be returned as a result
    return PWD_CHAR_SPACE[random() % (sizeof(PWD_CHAR_SPACE)/sizeof(PWD_CHAR_SPACE[0]) - 1)];
}

bool check_existing(const char to_test[PWD_DIMENSION]) {
    for(uint32_t index = 0; index < DICTIONARY_DIMENSION; index++) {
        if(strncmp(generated_list[index], to_test, PWD_DIMENSION) == 0) {
            return true;
        }
    }

    return false;
}

int main() {
    FILE *out_file;
    FILE *out_pwd_file;
    // Init the random seed, this should be done at least one time
    srandom(time(NULL));

    log_info("Starting dictionary generation..");

    remove(OUTPUT_FILENAME);

    out_file = fopen(OUTPUT_FILENAME, "a");
    out_pwd_file = fopen(OUTPUT_PWD_FILENAME, "a");

    if(out_file == NULL) {
        log_error("Cannot open dictionary file!");
        return 1;
    }

    if(out_pwd_file == NULL) {
        log_error("Cannot open dictionary file!");
        return 1;
    }

    for(uint32_t index = 0; index < DICTIONARY_DIMENSION; index++) {
        struct crypt_data data = { 0x00 };
        char new_pwd[PWD_DIMENSION];

        do {
            for(uint8_t pwd_index = 0; pwd_index < PWD_DIMENSION; pwd_index++) {
                new_pwd[pwd_index] = random_alnum();
            }
        } while (check_existing(new_pwd));

        crypt_r(new_pwd, SALT, &data);

        fwrite(new_pwd, 1, PWD_DIMENSION, out_file);
        fwrite(new_pwd, 1, PWD_DIMENSION, out_pwd_file);

        fwrite(";", 1, 1, out_file);
        fwrite(data.output, 1, strnlen(data.output, sizeof(data.output)), out_file);
        fwrite(";", 1, 1, out_file);
        fwrite(SALT, 1, strlen(SALT), out_file);

        fwrite("\n", 1, 1, out_file);
        fwrite("\n", 1, 1, out_pwd_file);
    }

    fwrite("\n", 1, 1, out_file);
    fwrite("\n", 1, 1, out_pwd_file);

    fclose(out_file);
    fclose(out_pwd_file);

    log_info("Terminated dictionary generation: generated %d passwords!", DICTIONARY_DIMENSION);

    return 0;
}
