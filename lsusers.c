#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>


#define NR_PASSWD_FIELDS 7
#define PASSWD_SPRTR_LINE_SPRTR ":"
#define PRNT_FMT  "%-10s %-10s %-10s %-10s %-10s %-10s %10s\n"

enum{
	FIELD_USERNAME,
	FIELD_PASSWORD,
	FIELD_UID,
	FIELD_GID,
	FIELD_GECOS,
	FIELD_HOME,
	FIELD_SHELL,
};

static void prnt_info(char **tokens)
{
	printf(PRNT_FMT, tokens[FIELD_USERNAME], tokens[FIELD_PASSWORD],
		tokens[FIELD_UID], tokens[FIELD_GID], tokens[FIELD_GECOS],
		tokens[FIELD_HOME], tokens[FIELD_SHELL]);
}

static void prnt_str_field(char **tokens,  char *to, u_int8_t idx)
{
	if (tokens[idx] && strcmp(tokens[idx], to) == 0)
		prnt_info(tokens);
}

static void prnt_int_field(char *to, int64_t val, char **tokens)
{
	if (to && atoi(to) == val)
		prnt_info(tokens);
}

static void str_noop(char **tokens, char *val, u_int8_t idx){}
static void int_noop(char *to, int64_t val, char **tokens){}

void (*prnt_usr)(char **, char *, u_int8_t) = str_noop;
void (*prnt_shell)(char **, char *, u_int8_t) = str_noop;
void (*prnt_home)(char **, char *, u_int8_t) = str_noop;

void (*prnt_uid)(char *, int64_t, char **) = int_noop;
void (*prnt_gid)(char *, int64_t, char **) = int_noop;

int main(int argc, char **argv)
{
	const char *usage = "lsusers -[usrghi]\n\n\
    -u    get info of this user\n\
    -g    show users in this GID\n\
    -i    show user with this UID\n\
    -s    show users with this shell\n\
    -p    use this alternative path to passwd\n\
    -H    show user with this home\n\
    -h    show help and exit";


    char *usr = NULL;
    char *passwd_path  = "/etc/passwd";
    int64_t gid = -1;
    int64_t uid = -1;
    char *shell = NULL;
    char *home = NULL;
    int optc = -1;


    while (-1 != (optc = getopt(argc, argv, "u:g:i:s:p:H:h"))) {
	switch(optc) {
	case 'u':
		usr = optarg;
		prnt_usr = prnt_str_field;
		break;
	case 'g':
		gid = atoi(optarg);
		prnt_gid = prnt_int_field;
		break;
	case 'i':
		uid = atoi(optarg);
		prnt_uid = prnt_int_field;
		break;
	case 's':
		shell = optarg;
		prnt_shell = prnt_str_field;
		break;
	case 'p':
		passwd_path = optarg;
		break;
	case 'H':
		home = optarg;
		prnt_home = prnt_str_field;
		break;
	case 'h':
	default:
		goto prnt_help_exit;
	}
    }

    if (gid == -1 && uid == -1 && !shell && !usr && !home)
	goto prnt_help_exit;

    FILE *f_passwd = fopen(passwd_path, "r");

    if (!f_passwd) {
	perror(passwd_path);
	return 0;
    }

    printf(PRNT_FMT, "USERNAME", "PASSWORD", "UID", "GID", "GECOS", "HOME",
	"SHELL");

    while (1){
	char *line = NULL;
	size_t line_sz = 0;
	ssize_t nr_chars = getline(&line, &line_sz, f_passwd);
	if (nr_chars <= 0) {
		free(line);
		break;
	}

	line[strlen(line) - 1] = '\0';

	char *token = strtok(line, PASSWD_SPRTR_LINE_SPRTR);
	if (token) {
		u_int8_t idx = 0;
		char **tokens = calloc(NR_PASSWD_FIELDS, sizeof(char *));
		do{
			tokens[idx++] = strdup(token);
		}while((token = strtok(NULL, PASSWD_SPRTR_LINE_SPRTR)));

		prnt_gid(tokens[FIELD_GID], gid, tokens);
		prnt_uid(tokens[FIELD_UID], uid, tokens);
		prnt_usr(tokens, usr, FIELD_USERNAME);
		prnt_shell(tokens, shell, FIELD_SHELL) ;
		prnt_home(tokens, home, FIELD_HOME) ;

		for (u_int8_t tok_idx = 0; tok_idx < NR_PASSWD_FIELDS;
			tok_idx++) {
			if (!tokens[tok_idx])
				continue;
			free(tokens[tok_idx]);
		}

		free(tokens);
	}
	free(line);
    }

    fclose(f_passwd);
    return 0;

prnt_help_exit:
	fprintf(stderr, "%s\n", usage);
	return 1;
}
