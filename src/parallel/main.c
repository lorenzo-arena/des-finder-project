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
#include <pthread.h>

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

#define THREAD_NUM 8

#define PWD_SET_DIMENSION 300

struct thread_arg
{
    char pwd_set[PWD_SET_DIMENSION][PWD_DIMENSION];
    const char * hash;
    const char * salt;
    char * found_pwd;
};

void *test_pwd_set(void *p)
{
    struct thread_arg *args = (struct thread_arg *)p;
    for(int pwd_idx = 0; pwd_idx < PWD_SET_DIMENSION; pwd_idx++)
    {
        char pwd[PWD_DIMENSION + 1] = { 0 };
        strncpy(pwd, args->pwd_set[pwd_idx], PWD_DIMENSION);

        if(test_password(pwd, args->hash, args->salt))
        {
            args->found_pwd = args->pwd_set[pwd_idx];
        }
    }

    return NULL;
}

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

    int thread_idx = 0;
    int pwd_idx = 0;
    struct thread_arg temp_arg[THREAD_NUM];
    pthread_t threads[THREAD_NUM];

    while(((read = getline(&line, &line_len, file)) != -1) && !pwd_found)
    {
        // I need to check the string length as getline seems
        // to allocate at least 120 bytes by default even for
        // line containing just '\n'
        if(strnlen(line, line_len) >= PWD_DIMENSION)
        {
            // Fill the set
            memcpy(temp_arg[thread_idx].pwd_set[pwd_idx], line, PWD_DIMENSION);
            pwd_idx++;
        }

        if(pwd_idx == PWD_SET_DIMENSION)
        {
            // Fill the remaining attributes
            pwd_idx = 0;
            temp_arg[thread_idx].hash = hash;
            temp_arg[thread_idx].salt = salt;
            temp_arg[thread_idx].found_pwd = NULL;

            // Spin up a thread
            pthread_create(&threads[thread_idx], NULL, test_pwd_set, &temp_arg[thread_idx]);
            thread_idx++;

            if(thread_idx == THREAD_NUM)
            {
                for(int i = 0; i < THREAD_NUM; i++)
                {
                    pthread_join(threads[i], NULL);

                    if(temp_arg[i].found_pwd != NULL)
                    {
                        // The pointer points to the middle of the array..
                        char found_pwd[PWD_DIMENSION + 1] = { 0 };
                        strncpy(found_pwd, temp_arg[i].found_pwd, PWD_DIMENSION);
                        log_info("PASSWORD FOUND: %s", found_pwd);
                        log_info("Exiting..");
                        pwd_found = true;
                    }
                }

                thread_idx = 0;
            }
        }
    }

    // Wait for the remaining threads then check for the result
    for(int i = 0; i < thread_idx; i++)
    {
        pthread_join(threads[i], NULL);
    }

    if(!pwd_found)
    {
        for(int i = 0; i < thread_idx; i++)
        {
            if(temp_arg[i].found_pwd != NULL)
            {
                // The pointer points to the middle of the array..
                char found_pwd[PWD_DIMENSION + 1] = { 0 };
                strncpy(found_pwd, temp_arg[i].found_pwd, PWD_DIMENSION);
                log_info("PASSWORD FOUND: %s", found_pwd);
                log_info("Exiting..");
                pwd_found = true;
            }
        }

        // TODO : it the result was not found for now perform the
        // last part sequentiallY but this has to be refactored
        if(pwd_idx > 0)
        {
            for(int i = 0; i < pwd_idx; i++)
            {
                if(test_password(temp_arg[thread_idx].pwd_set[i], hash, salt))
                {
                    // The pointer points to the middle of the array..
                    char found_pwd[PWD_DIMENSION + 1] = { 0 };
                    strncpy(found_pwd, temp_arg[thread_idx].pwd_set[i], PWD_DIMENSION);
                    log_info("PASSWORD FOUND: %s", found_pwd);
                    log_info("Exiting..");
                }
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
