#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <crypt.h>

#include <string.h>

#include "common.h"

bool test_password(char password[PWD_DIMENSION], const char *hash, const char *salt)
{
    struct crypt_data data = { 0x00 };

    crypt_r(password, salt, &data);

    return strncmp(data.output, hash, CRYPT_OUTPUT_SIZE) == 0;
}
