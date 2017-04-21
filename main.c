#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <getopt.h>
#include <xcb/xcb.h>

#define VERSION_MAJOR 0
#define VERSION_MINOR 1
#define VERSION_PATCH 0

#define BUF_SIZE  1024
#define MAX_ATOM  1000

typedef enum  {MATCH_NONE = 0, MATCH_PARTIAL, MATCH_FULL} MatchType;

static void print_atoms(const char* match, MatchType match_type, char* format,
        int range[2])
{
    xcb_connection_t *conn = xcb_connect(NULL, NULL);
    if (xcb_connection_has_error(conn)) {
        fputs("Display connection error.\n", stderr);
        exit(1);
    }
    xcb_get_atom_name_cookie_t cookies[MAX_ATOM];
    xcb_get_atom_name_reply_t *reply = NULL;
    char name[BUF_SIZE] = {0};

    for (int i = range[0]; i < range[1]; ++i) {
        cookies[i] = xcb_get_atom_name(conn, i);
    }

    for (int i = range[0]; i < range[1]; ++i) {
        reply = xcb_get_atom_name_reply(conn, cookies[i], NULL);
        if (!reply) {
            continue;
        }
        if (reply->name_len < 2) {
            free(reply);
            continue;
        }
        memcpy(name, xcb_get_atom_name_name(reply), reply->name_len);
        name[reply->name_len] = '\0';
        if (match_type == MATCH_PARTIAL && strstr(name, match) == NULL) {
            free(reply);
            continue;
        }else if (match_type == MATCH_FULL && strcmp(name, match) != 0) {
            free(reply);
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
        free(reply);
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

static void set_format(char *format, char *opt)
{
    char *c = opt;
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

static void set_range(int range[2], char* rng)
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
    char *match = NULL;
    char format[256] = "%d\t%s\n";
    int range[2] = {0, MAX_ATOM};
    MatchType match_type = MATCH_NONE;

    while ((opt = getopt_long(argc, argv, "f:hn:pr:v", 
                    longopts, &option_index)) != -1) {
        switch (opt) {
            case 'f':
                set_format(format, optarg);
                break;
            case 'n':
                if (match_type == MATCH_NONE) {
                    match_type = MATCH_FULL;
                }
                match = optarg;
                break;
            case 'p':
                match_type = MATCH_PARTIAL;
                break;
            case 'r':
                set_range(range, optarg);
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
        print_help();
        return 1;
    }
    print_atoms(match, match_type, format, range);
    return 0;
}
