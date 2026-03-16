/*
 * Just future proofing
 */

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    ARG_BOOL,
    ARG_INT,
    ARG_STRING,
    ARG_CALLBACK
} ArgType;

typedef void (*ArgCallback)(const char *value);

typedef struct {
    const char *long_name;
    char short_name;
    ArgType type;
    void *value;
    const char *help;
} ArgOption;

void print_help(ArgOption *opts, int count)
{
    int i;
    printf("Options:\n");
    for (i = 0; i < count; i++) {
        if (opts[i].short_name) printf("  -%c, ", opts[i].short_name);
    	else printf("      ");

        if (opts[i].long_name) printf("--%-12s ", opts[i].long_name);
        else printf("  %-12s ", "");
        printf("%s\n",
               opts[i].help ? opts[i].help : "");
    }
}

int parse_args(int argc, char **argv, ArgOption *opts, int opt_count)
{
    int i, j;

    for (i = 1; i < argc; i++) {
        char *arg = argv[i];

        if (arg[0] != '-')
            return i; /* stop parsing */

        char *name;
        char *value = NULL;

        if (strncmp(arg, "--", 2) == 0) {
            name = arg + 2;

            char *eq = strchr(name, '=');
            if (eq) {
                *eq = '\0';
                value = eq + 1;
            }
        } else {
            name = arg + 1;
        }

        for (j = 0; j < opt_count; j++) {
            ArgOption *opt = &opts[j];

            int match =
                (arg[1] == '-' && opt->long_name && strcmp(name, opt->long_name) == 0) ||
                (arg[1] != '-' && opt->short_name && name[0] == opt->short_name);

            if (!match)
                continue;

            if ((opt->type != ARG_BOOL) && value == NULL) {
                if (i + 1 < argc)
                    value = argv[++i];
                else {
                    fprintf(stderr, "Missing value for %s\n", arg);
                    exit(1);
                }
            }

            if (!opt->value) return argc;

            switch (opt->type) {

                case ARG_BOOL:
                    *(int*)opt->value = 1;
                    break;

                case ARG_INT:
                    *(int*)opt->value = atoi(value);
                    break;

                case ARG_STRING:
                    *(char**)opt->value = value;
                    break;

                case ARG_CALLBACK:
                    if (opt->value)
                        ((ArgCallback)opt->value)(value);
                    break;
            }

            break;
        }

        if (strcmp("help", name) == 0 || strcmp("h", name) == 0) {
        	print_help(opts, opt_count);
        	exit(0);
        }
    }

    return argc;
}
