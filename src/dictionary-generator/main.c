#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <argp.h>
#include <unistd.h>

#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <crypt.h>

#include "log.h"
#include "defines.h"
#include "stopwatch.h"

const char *argp_program_version =
" 1.0";

struct arguments {
    char *args[1];                /* the number of passwords to generate */
    bool accept_duplicates;
    bool sequential;
};

void set_default_arguments(struct arguments *arguments) {
    arguments->args[0] = "";
    arguments->accept_duplicates = false;
    arguments->sequential = false;
}

static struct argp_option options[] = {
    {"accept-duplicates", 'd', 0, 0, "Accept duplicates in generated dictionary", 0},
    {"sequential", 's', 0, 0, "Creation a dictionary from 00000000 on", 0},
    {0}
};

static error_t parse_opt (int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = state->input;

    switch (key)
    {
        case 'd':
            arguments->accept_duplicates = true;
            break;
        case 's':
            arguments->sequential = true;
            break;
        case ARGP_KEY_ARG:
            if (state->arg_num >= 1) {
                argp_usage(state);
            }
            arguments->args[state->arg_num] = arg;
            break;
        case ARGP_KEY_END:
            if (state->arg_num < 1) {
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

static char *generated_list = NULL;

void dict_gen_rec(char * const p, const int index, const uint32_t max_pwd)
{
    static uint32_t generated_pwd_number = 0;
    for(uint32_t space_index = 0;
        space_index < (sizeof(PWD_CHAR_SPACE)/sizeof(PWD_CHAR_SPACE[0]) - 1);
        space_index++)
    {
        // Check forced return condition
        if(generated_pwd_number >= max_pwd)
        {
            return;
        }

        p[index] = PWD_CHAR_SPACE[space_index];
        if(index > 0)
        {
            dict_gen_rec(p, index - 1, max_pwd);
        }
        else
        {
            memcpy(&generated_list[generated_pwd_number * (PWD_DIMENSION + 1)], p, PWD_DIMENSION + 1);
            generated_pwd_number++;
        }
    }
}

int main(int argc, char *argv[]) {
    FILE *dictionary_hash_file;
    FILE *dictionary_file;
    uint32_t pwd_number = 0;

    stopwatch_start();

    // Get the input options
    struct arguments arguments;
    set_default_arguments(&arguments);
    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    {
        // This is used to check if the option for number is a number
        char *next;
        pwd_number = strtol(arguments.args[0], &next, 10);

        if(*next != '\0') {
            log_error("Given password number not correct!");
            return 1;
        }
    }

    log_info("Starting dictionary generation..");

    generated_list = malloc(pwd_number * (PWD_DIMENSION + 1));

    if(generated_list == NULL) {
        log_error("Error during password generation!");
        return 1;
    }

    remove(DICTIONARY_HASH_FILENAME);
    remove(DICTIONARY_FILENAME);

    dictionary_hash_file = fopen(DICTIONARY_HASH_FILENAME, "a");
    dictionary_file = fopen(DICTIONARY_FILENAME, "a");

    fseek(dictionary_file, ((PWD_DIMENSION + 1) * pwd_number) + 2, SEEK_SET);
    fseek(dictionary_hash_file, ((25 + 1) * pwd_number) + 2, SEEK_SET);

    fseek(dictionary_file, 0, SEEK_SET);
    fseek(dictionary_hash_file, 0, SEEK_SET);

    if((dictionary_hash_file == NULL) || (dictionary_file == NULL)) {
        log_error("Cannot open dictionary file!");
        return 1;
    }

    if(arguments.sequential)
    {
        char new_pwd[PWD_DIMENSION + 1] = { 0x00 };
        dict_gen_rec(new_pwd, PWD_DIMENSION - 1, pwd_number);
    }
    else
    {
        if(arguments.accept_duplicates)
        {
            log_info("Overriding duplicates check...");
        }

        // Init the random seed, this should be done at least one time
        srandom(time(NULL));

        for(uint32_t index = 0; index < pwd_number; index++) {
            char new_pwd[PWD_DIMENSION + 1] = { 0x00 };

            do {
                for(uint8_t pwd_index = 0; pwd_index < PWD_DIMENSION; pwd_index++) {
                    new_pwd[pwd_index] = random_alnum();
                }
            } while (!arguments.accept_duplicates && check_existing((const char *)new_pwd, (const char *)generated_list, pwd_number));

            memcpy(&generated_list[index * (PWD_DIMENSION + 1)], new_pwd, PWD_DIMENSION + 1);
        }
    }

    for(uint32_t index = 0; index < pwd_number; index++) {
        struct crypt_data data = { 0x00 };
        crypt_r(&generated_list[index * (PWD_DIMENSION + 1)], SALT, &data);

        fwrite(&generated_list[index * (PWD_DIMENSION + 1)], 1, PWD_DIMENSION, dictionary_hash_file);
        fwrite(&generated_list[index * (PWD_DIMENSION + 1)], 1, PWD_DIMENSION, dictionary_file);

        fwrite(";", 1, 1, dictionary_hash_file);
        fwrite(data.output, 1, HASH_DIMENSION, dictionary_hash_file);
        fwrite(";", 1, 1, dictionary_hash_file);
        fwrite(SALT, 1, SALT_SIZE, dictionary_hash_file);

        fwrite("\n", 1, 1, dictionary_hash_file);
        fwrite("\n", 1, 1, dictionary_file);
    }

    fwrite("\n", 1, 1, dictionary_hash_file);
    fwrite("\n", 1, 1, dictionary_file);

    fflush(dictionary_hash_file);
    fflush(dictionary_file);

    fclose(dictionary_hash_file);
    fclose(dictionary_file);

    log_info("Terminated dictionary generation: generated %d passwords!", pwd_number);

    stopwatch_stop();

    struct timespec elapsed = stopwatch_get_elapsed();

    log_info("Stopwatch stopped: elapsed %d seconds and %lu nanoseconds",
             elapsed.tv_sec,
             elapsed.tv_nsec);

    free(generated_list);

    return 0;
}
