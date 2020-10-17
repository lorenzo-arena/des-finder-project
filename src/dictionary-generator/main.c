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
    FILE *dictionary_hash_file;
    FILE *dictionary_file;
    // Init the random seed, this should be done at least one time
    srandom(time(NULL));

    log_info("Starting dictionary generation..");

    remove(DICTIONARY_HASH_FILENAME);
    remove(DICTIONARY_FILENAME);

    dictionary_hash_file = fopen(DICTIONARY_HASH_FILENAME, "a");
    dictionary_file = fopen(DICTIONARY_FILENAME, "a");

    if(dictionary_hash_file == NULL) {
        log_error("Cannot open dictionary file!");
        return 1;
    }

    if(dictionary_file == NULL) {
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

        fwrite(new_pwd, 1, PWD_DIMENSION, dictionary_hash_file);
        fwrite(new_pwd, 1, PWD_DIMENSION, dictionary_file);

        fwrite(";", 1, 1, dictionary_hash_file);
        fwrite(data.output, 1, strnlen(data.output, sizeof(data.output)), dictionary_hash_file);
        fwrite(";", 1, 1, dictionary_hash_file);
        fwrite(SALT, 1, strlen(SALT), dictionary_hash_file);

        fwrite("\n", 1, 1, dictionary_hash_file);
        fwrite("\n", 1, 1, dictionary_file);
    }

    fwrite("\n", 1, 1, dictionary_hash_file);
    fwrite("\n", 1, 1, dictionary_file);

    fclose(dictionary_hash_file);
    fclose(dictionary_file);

    log_info("Terminated dictionary generation: generated %d passwords!", DICTIONARY_DIMENSION);

    return 0;
}
