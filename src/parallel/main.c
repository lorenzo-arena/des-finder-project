#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <argp.h>
#include <sys/sysinfo.h>

#include <time.h>

#include "log.h"
#include "defines.h"
#include "stopwatch.h"
#include "common.h"
#include "processor.h"

const char *argp_program_version =
" 1.0";

struct arguments
{
    char *args[2];                /* hash and salt */
    char *dictionary_path;        /* Argument for -d */
    int threads;
    bool stopwatch;
};

void set_default_arguments(struct arguments *arguments)
{
    arguments->args[0] = "";
    arguments->args[1] = "";
    arguments->dictionary_path = DICTIONARY_FILENAME;
    arguments->threads = get_nprocs_conf();
    arguments->stopwatch = false;
}

static struct argp_option options[] =
{
    {"dictionary", 'd', "", 0, "Set the dictionary file path", 0},
    {"threads", 't', "", 0, "Number of threads to use, from 1 to the maximum number of CPU for this machine", 0},
    {"stopwatch", 's', 0, 0, "Enable stopwatch usage", 0},
    {0}
};

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
    struct arguments *arguments = state->input;

    switch (key)
    {
        case 't':
        {
            char *next;
            arguments->threads = strtol(arg, &next, 10);

            if(*next != '\0')
            {
                log_error("Invalid threads number %s", arg);
                argp_usage(state);
            }
            else if((arguments->threads < 1) || (arguments->threads > get_nprocs_conf()))
            {
                log_error("Invalid threads number: %d", arguments->threads);
                argp_usage(state);
            }
            break;
        }
        case 'd':
        {
            arguments->dictionary_path = arg;
            break;
        }
        case 's':
        {
            arguments->stopwatch = true;
            break;
        }
        case ARGP_KEY_ARG:
        {
            if (state->arg_num >= 2)
            {
                argp_usage(state);
            }
            arguments->args[state->arg_num] = arg;
            break;
        }
        case ARGP_KEY_END:
        {
            if (state->arg_num < 2)
            {
                argp_usage(state);
            }
            break;
        }
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
    struct arguments arguments;

    set_default_arguments(&arguments);

    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    log_info("Starting processing with %d threads", arguments.threads);

    if(arguments.stopwatch)
    {
        stopwatch_start();
    }

    if(process_file(arguments.dictionary_path, arguments.args[0], arguments.args[1], arguments.threads) != 0)
    {
        log_error("Error during file processing!");
        return 1;
    }

    if(arguments.stopwatch)
    {
        stopwatch_stop();

        struct timespec elapsed = stopwatch_get_elapsed();

        printf("%ld.%09ld\n",
            elapsed.tv_sec,
            elapsed.tv_nsec);
    }

    return 0;
}
