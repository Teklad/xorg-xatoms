#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <getopt.h>
#include <xcb/xcb.h>

#define VERSION_MAJOR 0
#define VERSION_MINOR 1
#define VERSION_PATCH 0

#define MAX_ATOMS 10000
#define BUF_SIZE  1024

// Stuff for matching modes
enum  MatchType {MATCH_NONE = 0, MATCH_PARTIAL, MATCH_FULL};
char  match[BUF_SIZE] = {'\0'};
int   match_type = MATCH_NONE;

// Format and range specifiers
char  format[BUF_SIZE] = "%d\t%s\n";
int   range[2] = {0, MAX_ATOMS};

static void print_atoms()
{
    xcb_connection_t *conn = xcb_connect(NULL, NULL);
    xcb_get_atom_name_cookie_t cookies[MAX_ATOMS];

    // Generate all of our cookies at once.
    for (int i = range[0]; i < range[1]; ++i) {
        cookies[i] = xcb_get_atom_name(conn, i);
    }

    // Ready a few of our variables.
    char name[BUF_SIZE];
    xcb_get_atom_name_reply_t *reply = NULL;

    // Loop through the replies and print data based on conditions
    for (int i = range[0]; i < range[1]; ++i) {
        reply = xcb_get_atom_name_reply(conn, cookies[i], NULL);
        if (!reply)
            continue;
        int  name_len = xcb_get_atom_name_name_length(reply);
        char *name_ptr = xcb_get_atom_name_name(reply);
        snprintf(name, name_len + 1, name_ptr);
        
        // Skip printing atoms that don't match conditions
        if (match_type == MATCH_PARTIAL && strstr(name, match) == NULL) {
            continue;
        }else if (match_type == MATCH_FULL && strcmp(name, match) != 0) {
            continue;
        }

        // Safe print
        char *p = format;
        while(*p != 0) {
            switch(*p) {
                case '%':
                    if (*(p+1) == 'd') {
                        fprintf(stdout, "%i", i);
                    }else if (*(p+1) == 's') {
                        fputs(name, stdout);
                    }else if (*(p+1) == '%') {
                        fputs("%", stdout);
                    }
                    p++;
                    break;
                default:
                    fputc(*p, stdout);
                    break;
            }
            p++;
        }
    }
    xcb_disconnect(conn);
}


static void print_help()
{
    puts("XAtoms Usage:");
    puts("\t-h, --help\t\tPrint this message.");
    puts("\t-v, --version\t\tProgram version information.");
    puts("\t-f, --format\t\tFormat the output using printf style.");
    puts("\t-n, --name\t\tOptional. Name of atom to look for.");
    puts("\t-p, --partial\t\tPartial match mode.  Combine with --name");
    puts("\t\t\t\tto match against partial atom names.");
    puts("\t\t\t\tExample:");
    puts("\t\t\t\t\txatoms --name=NET_WM -p");
    puts("\t-r, --range\t\t[num-num] Range of atoms to search for.");
    puts("\t\t\t\tExample:");
    puts("\t\t\t\t\t--range=-20     (from 0 to 20)");
    puts("\t\t\t\t\t--range=300-    (from 300 onward)");
    puts("\t\t\t\t\t--range=300-400 (from 300 to 400)");
}

static void print_version()
{
    printf("XAtoms (%i.%i.%i)\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
}

static void set_format(char *fmt)
{
    char *c = fmt;
    char *p = format;
    while (*c != 0) {
        switch(*c) {
            case '\\':
                c++;
                if (*c == 'n')     { *p = '\n'; }
                else if(*c == 'r') { *p = '\r'; }
                else if(*c == 't') { *p = '\t'; }
                else                 { *p = *(--c);}
                break;
            default:
                *p = *c;
                break;
        }
        p++;
        c++;
    }
    *p = '\0';
}

static void set_range(char* rng)
{
    char* sep = strstr(rng, "-");
    if (sep) {
        range[1] = atoi(sep+1);
    }
    range[0] = atoi(rng);
}

int main(int argc, char* argv[])
{
    struct option longopts[] = {
        {"format", required_argument, 0, 'f'},
        {"help", no_argument, 0, 'h'},
        {"name", required_argument, 0, 'n'},
        {"partial", no_argument, 0, 'p'},
        {"range", required_argument, 0, 'r'},
        {"version", no_argument, 0, 'v'}
    };
    int opt;
    int i, option_index = 0;
    while ((opt = getopt_long(argc, argv, "f:hn:pr:v", 
                    longopts, &option_index)) != -1) {
        switch (opt) {
            case 'f':
                set_format(optarg);
                break;
            case 'n':
                if (match_type == MATCH_NONE) {
                    match_type = MATCH_FULL;
                }
                strncpy(match, optarg, BUF_SIZE);
                break;
            case 'p':
                match_type = MATCH_PARTIAL;
                break;
            case 'r':
                set_range(optarg);
                break;
            case 'v':
                print_version();
                return 0;
            case 'h':
                print_help();
                return 0;
            case '?':
            default:
                print_help();
                return 1;
        }

    }
    for (int i = optind; i < argc; ++i) {
        printf("Unknown argument - %s\n", argv[i]);
        return 1;
    }
    print_atoms();
    return 0;
}
