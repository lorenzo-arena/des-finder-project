#include <stdio.h>
#include <argp.h>

#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <crypt.h>

#include <time.h>

#include "log.h"
#include "defines.h"

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
    {"dictionary", 'd', DICTIONARY_FILENAME, 0, "Set the dictionary file path", 0}
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

int main(int argc, char *argv[]) {
    const char *hash;
    const char *salt;
    const char *dictionary_path;
    struct arguments arguments;

    set_default_arguments(&arguments);

    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    hash = arguments.args[0];
    salt = arguments.args[1];
    dictionary_path = arguments.dictionary_path;

    return 0;
}
