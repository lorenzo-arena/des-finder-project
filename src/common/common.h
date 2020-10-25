#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdbool.h>

#include "defines.h"

bool test_password(char password[PWD_DIMENSION], const char *hash, const char *salt);

#endif
