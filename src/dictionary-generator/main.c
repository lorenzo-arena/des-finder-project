#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <argp.h>

#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <crypt.h>

#include "log.h"
#include "defines.h"

const char *argp_program_version =
" 1.0";

struct arguments
{
    char *args[1];                /* the number of passwords to generate */
};

void set_default_arguments(struct arguments *arguments)
{
    arguments->args[0] = "";
}

static struct argp_option options[] =
{
    {0}
};

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
    struct arguments *arguments = state->input;

    switch (key)
    {
        case ARGP_KEY_ARG:
            if (state->arg_num >= 1)
            {
                argp_usage(state);
            }
            arguments->args[state->arg_num] = arg;
            break;
        case ARGP_KEY_END:
            if (state->arg_num < 1)
            {
                argp_usage(state);
            }
            break;
        default:
        return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

static char args_doc[] = "pwd_number";

static char doc[] =
"dictoinary-generator -- Used to generate the dictionary of passwords.";

static struct argp argp = {options, parse_opt, args_doc, doc, NULL, NULL, NULL};

char random_alnum() {
    // The "-1" is necessary otherwise the string ending can be returned as a result
    return PWD_CHAR_SPACE[random() % (sizeof(PWD_CHAR_SPACE)/sizeof(PWD_CHAR_SPACE[0]) - 1)];
}

bool check_existing(const char to_test[PWD_DIMENSION], const char *list, uint32_t dictionary_dimension) {

    for(uint32_t index = 0; index < dictionary_dimension; index++) {
        if(strncmp(&list[index * PWD_DIMENSION], to_test, PWD_DIMENSION) == 0) {
            return true;
        }
    }

    return false;
}

int main(int argc, char *argv[]) {
    FILE *dictionary_hash_file;
    FILE *dictionary_file;
    char *generated_list;
    uint32_t pwd_number = 0;

    // Get the input options
    struct arguments arguments;
    set_default_arguments(&arguments);
    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    {
        char *next;
        pwd_number = strtol(arguments.args[0], &next, 10);

        if(*next != '\0')
        {
            log_error("Given password number not correct!");
            return 1;
        }
    }

    log_info("Starting dictionary generation..");

    generated_list = malloc(pwd_number * PWD_DIMENSION);

    if(generated_list == NULL)
    {
        log_error("Error during password generation!");
        return 1;
    }

    // Init the random seed, this should be done at least one time
    srandom(time(NULL));

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

    for(uint32_t index = 0; index < pwd_number; index++) {
        struct crypt_data data = { 0x00 };
        char new_pwd[PWD_DIMENSION];

        do {
            for(uint8_t pwd_index = 0; pwd_index < PWD_DIMENSION; pwd_index++) {
                new_pwd[pwd_index] = random_alnum();
            }
        } while (check_existing((const char *)new_pwd, (const char *)generated_list, pwd_number));

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

    log_info("Terminated dictionary generation: generated %d passwords!", pwd_number);

    free(generated_list);
    return 0;
}
