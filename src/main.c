/**
 * main.c
 */

#include <argp.h>
#include <unistd.h>
#include <locale.h>
#include <sys/wait.h>

#include "libgen.h"

#define SHELL "/bin/sh"
static config cfg;
static target targets[SEC_COUNT];

static int get_index(cursor *cr)
{
	int index = 0;
	switch (cr->col_index)
	{
	case 0:
		index = cr->sec_index;
		break;
	case 1:
		index = cr->ent_index;
		break;
	}
	return index;
}

static int get_count(cursor *cr)
{
	int count = 0;
	switch (cr->col_index)
	{
	case 0:
		count = cr->sec_count;
		break;
	case 1:
		count = cr->ent_count;
		break;
	}
	return count;
}

static int get_width(cursor *cr)
{
	int width = 0;
	switch (cr->col_index)
	{
	case 0:
		width = cr->sec_width;
		break;
	case 1:
		width = cr->ent_width;
		break;
	}
	return width;
}

static int get_state(cursor *cr)
{
	int state = 0;
	if (cr->col_index == 0)
		state = targets[cr->sec_index].state;
	return state;
}

static int get_pos_x(cursor *cr)
{
	int pos = 0;
	switch (cr->col_index)
	{
	case 1:
		pos = cr->sec_width;
		break;
	}
	return pos;
}

static int set_index(cursor *cr, int val)
{
	int index = get_index(cr);
	switch (cr->col_index)
	{
	case 0:
		cr->sec_index = val;
		break;
	case 1:
		cr->ent_index = val;
		break;
	}
	return index; // old index
}

static int set_state(cursor *cr, int val)
{
	int state = get_state(cr);
	if (state != val)
	{
		targets[cr->sec_index].state = val;
	}
	return state; // old state
}

static int run_command(const char *cmd)
{
	pid_t pid;
	int status;

	pid = fork();
	if (pid == 0)
	{
		execl(SHELL, SHELL, "-c", cmd, NULL);
		_exit(EXIT_FAILURE);
	}
	else if (pid < 0)
	{
		status = -1;
	}
	else if (waitpid(pid, &status, WNOHANG) != pid)
	{
		status = -1;
	}

	return status;
}

static char *get_viewer(const char *file)
{
	char *ext = strrchr(file, '.');
	if (ext == NULL)
	{
		return NULL;
	}
	else if (strcmp(ext, ".pdf") == 0)
	{
		return cfg.vi1;
	}
	else if (strcmp(ext, ".djvu") == 0)
	{
		return cfg.vi2;
	}
	return NULL;
}

const char *argp_program_version = "libgen 0.1.0";
const char *argp_program_bug_address = "<https://github.com/intity/libgen/issues>";

static error_t parse_opt(int key, char *arg, __unused struct argp_state *state)
{
	//
	// option parse
	//
	switch (key)
	{
	case ARGS_KEY_DELETE:
	case ARGS_KEY_INSERT:
	case ARGS_KEY_UPDATE:
	case ARGS_KEY_QUERY:
		query_task(key, 0, arg);
		break;
	case ARGP_KEY_END:
		//printf("parse_opt [state->next: %d]\n", state->next);
		break;
	default:
		return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

static char doc[] = "command-line software for a configure bibliography data";
static char args_doc[] = "[ARGS...]";

static struct argp_option options[] = {
	{"delete", ARGS_KEY_DELETE, "KEY", 0, "delete entry from database", 0},
	{"insert", ARGS_KEY_INSERT, "FILE", 0, "insert a new entry to database", 0},
	{"update", ARGS_KEY_UPDATE, "FILE", 0, "update exist entry of database", 0},
	{"query", ARGS_KEY_QUERY, "SQL", 0, "query SQL", 0},
	{0, 0, 0, 0, 0, 0}
};

static struct argp argp = {options, parse_opt, args_doc, doc, 0, 0, 0};

void key_bindings(int ch, cursor *cr)
{
	int y, x, col_width, row_index, row_count, old_state, cur_state, cur_pos_x;
	col_width = get_width(cr);
	row_index = get_index(cr);
	row_count = get_count(cr);
	old_state = get_state(cr);
	getyx(stdscr, y, x);
	switch (ch)
	{
	case KEY_DOWN:
		if (row_index < row_count - 1)
		{
			row_index = y + 1;
			set_index(cr, row_index);
			cur_state = get_state(cr);
			chgat(col_width, A_NORMAL, old_state, NULL);
			if (cr->col_index == 1)
				update_ui(cr);
			mvchgat(row_index, x, col_width, A_REVERSE, cur_state, NULL);
		}
		break;
	case KEY_UP:
		if (row_index > 0)
		{
			row_index = y - 1;
			set_index(cr, row_index);
			cur_state = get_state(cr);
			chgat(col_width, A_NORMAL, old_state, NULL);
			if (cr->col_index == 1)
				update_ui(cr);
			mvchgat(row_index, x, col_width, A_REVERSE, cur_state, NULL);
		}
		break;
	case KEY_LEFT:
		if (cr->col_index == 1)
		{
			cr->col_index--;
			row_index = get_index(cr);
			cur_state = get_state(cr);
			x -= cr->sec_width;
			chgat(cr->ent_width, A_NORMAL, old_state, NULL);
			mvchgat(row_index, x, cr->sec_width, A_REVERSE, cur_state, NULL);
		}
		break;
	case KEY_RIGHT:
		if (cr->col_index == 0)
		{
			cr->col_index++;
			row_index = get_index(cr);
			cur_state = get_state(cr);
			x += cr->sec_width;
			chgat(cr->sec_width, A_NORMAL, old_state, NULL);
			mvchgat(row_index, x, cr->ent_width, A_REVERSE, cur_state, NULL);
		}
		break;
	case KEY_ENTER:
	case 10:
		if (cr->col_index == 0)
		{
			cur_state = old_state ? 0 : 1;
			set_state(cr, cur_state);
			update_ui(cr);
			mvchgat(row_index, 0, cr->sec_width, A_REVERSE, cur_state, NULL);
		}
		else // commands
		{
			char command[PATH_MAX];
			char *viewer = get_viewer(cr->file);
			if (viewer == NULL)
				return;
			sprintf(command, "%s %s/%s", viewer, cfg.lib, cr->file);
			run_command(command);
		}
		break;
	case KEY_RESIZE:
		getmaxyx(stdscr, cr->szh, cr->szw);
		cr->sec_width = cr->szw * COL0;
		cr->ent_width = cr->szw * COL1;
		cur_state = get_state(cr);
		col_width = get_width(cr);
		cur_pos_x = get_pos_x(cr);
		update_ui(cr);
		mvchgat(row_index, cur_pos_x, col_width, A_REVERSE, cur_state, NULL);
		break;
	}
}

int main(int argc, char *argv[])
{
	int ch;
	cursor cr = {
		.col_index = 0,
		.ent_index = 0,
		.sec_index = 0,
		.szh = 0,
		.szw = 0};

	setlocale(LC_ALL, "en_US.UTF-8");
	init_cfg(&cfg);

	if (argc > 1)
	{
		argp_parse(&argp, argc, argv, 0, 0, 0);
		exit(0);
	}

	initscr();
	start_color();
	init_pair(1, COLOR_CYAN, COLOR_BLACK);
	raw();
	keypad(stdscr, TRUE);
	noecho();
	curs_set(0);
	getmaxyx(stdscr, cr.szh, cr.szw);

	cr.sec_width = cr.szw * COL0;
	cr.ent_width = cr.szw * COL1;

	init_targets(targets);
	update_ui(&cr);
	mvchgat(0, 0, cr.sec_width, A_REVERSE, 0, NULL);
	refresh();

	while ((ch = getch()) != 'q')
	{
		key_bindings(ch, &cr);
		refresh();
	}

	noraw();
	keypad(stdscr, FALSE);
	echo();
	endwin();

	return 0;
}
