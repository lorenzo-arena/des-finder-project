#include <stdio.h>

#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <crypt.h>

#include <time.h>

#include "log.h"
#include "defines.h"

#define SALT "sa"

#define PWD "testtest"

int main() {
    struct timespec start, end;
    struct crypt_data data = {
        .input = PWD,
        .initialized = 0
    };

    printf("Hello!\n");

    clock_gettime(CLOCK_MONOTONIC, &start);
    crypt_r(PWD, SALT, &data);
    clock_gettime(CLOCK_MONOTONIC, &end);

    printf("Crypted pwd: %s\n", data.output);
    printf("Elapsed %ldns\n", end.tv_nsec - start.tv_nsec);

    return 0;
}
