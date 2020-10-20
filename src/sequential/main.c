#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <argp.h>

#include <time.h>

#include "log.h"
#include "defines.h"
#include "stopwatch.h"
#include "common.h"

const char *argp_program_version =
" 1.0";

struct arguments
{
    char *args[2];                /* hash and salt */
    char *dictionary_path;        /* Argument for -d */
};

void set_default_arguments(struct arguments *arguments)
{
    arguments->args[0] = "";
    arguments->args[1] = "";
    arguments->dictionary_path = DICTIONARY_FILENAME;
}

static struct argp_option options[] =
{
    {"dictionary", 'd', DICTIONARY_FILENAME, 0, "Set the dictionary file path", 0},
    {0}
};

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
    struct arguments *arguments = state->input;

    switch (key)
    {
        case 'd':
            arguments->dictionary_path = arg;
            break;
        case ARGP_KEY_ARG:
            if (state->arg_num >= 2)
            {
                argp_usage(state);
            }
            arguments->args[state->arg_num] = arg;
            break;
        case ARGP_KEY_END:
            if (state->arg_num < 2)
            {
                argp_usage(state);
            }
            break;
        default:
        return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

static char args_doc[] = "hash salt";

static char doc[] =
"des-finder-seq -- Used to find passwords from a dictionary and an hash and salt.";

static struct argp argp = {options, parse_opt, args_doc, doc, NULL, NULL, NULL};

int process_file(const char *filename, const char *hash, const char *salt)
{
    FILE *file = NULL;
    char *line = NULL;
    size_t line_len = 0;
    ssize_t read = 0;
    bool pwd_found = false;

    file = fopen(filename, "r");
    if(file == NULL)
    {
        log_error("Could't open file: %s", filename);
        return -1;
    }

    while(((read = getline(&line, &line_len, file)) != -1) && !pwd_found) {
        // I need to check the string length as getline seems
        // to allocate at least 120 bytes by default even for
        // line containing just '\n'
        if(strnlen(line, line_len) >= PWD_DIMENSION)
        {
            char pwd_to_test[PWD_DIMENSION];

            memcpy(pwd_to_test, line, PWD_DIMENSION);

#ifdef TRACE
            log_info("Processing pwd: %s", pwd_to_test);
#endif

            if(test_password(pwd_to_test, hash, salt))
            {
                log_info("PASSWORD FOUND: %s", pwd_to_test);
                log_info("Exiting..");
                pwd_found = true;
            }
        }
    }

    fclose(file);

    if(line)
    {
        free(line);
    }

    if(!pwd_found)
    {
        log_error("Password not found!");
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[]) {
    struct arguments arguments;

    stopwatch_start();

    set_default_arguments(&arguments);

    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    if(process_file(arguments.dictionary_path, arguments.args[0], arguments.args[1]) != 0)
    {
        log_error("Error during file processing!");
        return 1;
    }

    stopwatch_stop();

    struct timespec elapsed = stopwatch_get_elapsed();

    log_info("Stopwatch stopped: elapsed %d seconds and %lu nanoseconds",
             elapsed.tv_sec,
             elapsed.tv_nsec);

    return 0;
}
