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
static column columns[COL_COUNT];

static int get_index(cursor *cr)
{
	int index = 0;
	switch (cr->col_index)
	{
	case 0:
		index = columns[0].cur_index;
		break;
	case 1:
		if (columns[1].cur_index < columns[1].cur_count)
			index = columns[1].cur_index;
		else
			index = columns[1].cur_count - 1;
		break;
	}
	return index;
}

static int get_state(cursor *cr)
{
	int state = 0;
	int index = columns[0].cur_index;
	if (cr->col_index == 0)
		state = targets[index].state;
	return state;
}

static int get_pos_x(cursor *cr)
{
	int pos = 0;
	switch (cr->col_index)
	{
	case 1:
		pos = columns[0].cur_width;
		break;
	}
	return pos;
}

static int set_state(cursor *cr, int val)
{
	int state = get_state(cr);
	int index = columns[0].cur_index;
	if (state != val)
	{
		targets[index].state = val;
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
		return cfg.vid != NULL ? cfg.vid : cfg.vi1;
	}
	else if (strcmp(ext, ".djvu") == 0)
	{
		return cfg.vid != NULL ? cfg.vid : cfg.vi2;
	}
	else
	{
		return cfg.vid;
	}
	return NULL;
}

static void resize_ui(cursor *cr)
{
	int cur_index, cur_pos_x, cur_state, cur_width, row_index;

	getmaxyx(stdscr, cr->szh, cr->szw);
	columns[0].cur_width = cr->szw * COL0;
	columns[1].cur_width = cr->szw * COL1;

	row_index = columns[0].row_index;
	if (cr->szh > row_index)
		columns[0].row_index = row_index;
	else
	{
		columns[0].row_index = 0;
		columns[0].off_count = 0;
	}

	row_index = columns[1].row_index;
	if (cr->szh > row_index)
		columns[1].row_index = row_index;
	else
	{
		columns[1].row_index = 0;
		columns[1].off_count = 0;
	}

	cur_index = columns[0].cur_index;
	if (cr->szh > cur_index)
		columns[0].cur_index = cur_index;
	else
		columns[0].cur_index = 0;

	cur_index = columns[1].cur_index;
	if (cr->szh > cur_index)
		columns[1].cur_index = cur_index;
	else
		columns[1].cur_index = 0;

	cur_index = columns[cr->col_index].cur_index;
	cur_width = columns[cr->col_index].cur_width;
	cur_state = get_state(cr);
	cur_pos_x = get_pos_x(cr);
	update_ui(cr);
	mvchgat(cur_index, cur_pos_x, cur_width, A_REVERSE, cur_state, NULL);
}

const char *argp_program_version = "libgen 0.2.0";
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
	int y, x, cur_index, cur_state, cur_width, old_state, row_index, row_count;
	cur_index = get_index(cr);
	old_state = get_state(cr);
	row_index = columns[cr->col_index].row_index;
	row_count = columns[cr->col_index].row_count;
	getyx(stdscr, y, x);
	switch (ch)
	{
	case KEY_DOWN:
		if (cur_index < columns[cr->col_index].cur_count - 1)
		{
			cur_index = y + 1;
			columns[cr->col_index].cur_index = cur_index;
			cur_width = columns[cr->col_index].cur_width;
			cur_state = get_state(cr);
			chgat(cur_width, A_NORMAL, old_state, NULL);
			if (cr->col_index == 1)
				update_ui(cr);
			mvchgat(cur_index, x, cur_width, A_REVERSE, cur_state, NULL);
			columns[cr->col_index].row_index++;
		}
		else if (row_index < row_count - 1)
		{
			row_index++;
			columns[cr->col_index].off_count++;
			columns[cr->col_index].row_index = row_index;
			cur_width = columns[cr->col_index].cur_width;
			cur_state = get_state(cr);
			update_ui(cr);
			mvchgat(cur_index, x, cur_width, A_REVERSE, cur_state, NULL);
		}
		break;
	case KEY_UP:
		if (cur_index > 0)
		{
			cur_index = y - 1;
			columns[cr->col_index].cur_index = cur_index;
			cur_state = get_state(cr);
			cur_width = columns[cr->col_index].cur_width;
			chgat(cur_width, A_NORMAL, old_state, NULL);
			if (cr->col_index == 1)
				update_ui(cr);
			mvchgat(cur_index, x, cur_width, A_REVERSE, cur_state, NULL);
			columns[cr->col_index].row_index--;
		}
		else if (row_index > 0)
		{
			row_index--;
			columns[cr->col_index].off_count--;
			columns[cr->col_index].row_index = row_index;
			cur_width = columns[cr->col_index].cur_width;
			cur_state = get_state(cr);
			update_ui(cr);
			mvchgat(cur_index, x, cur_width, A_REVERSE, cur_state, NULL);
		}
		break;
	case KEY_LEFT:
		if (cr->col_index == 1)
		{
			cr->col_index--;
			cur_index = get_index(cr);
			cur_state = get_state(cr);
			cur_width = columns[0].cur_width;
			x -= cur_width;
			chgat(columns[1].cur_width, A_NORMAL, old_state, NULL);
			mvchgat(cur_index, x, cur_width, A_REVERSE, cur_state, NULL);
		}
		break;
	case KEY_RIGHT:
		if (cr->col_index == 0)
		{
			cr->col_index++;
			cur_index = get_index(cr);
			cur_state = get_state(cr);
			cur_width = columns[1].cur_width;
			x += columns[0].cur_width;
			chgat(columns[0].cur_width, A_NORMAL, old_state, NULL);
			mvchgat(cur_index, x, cur_width, A_REVERSE, cur_state, NULL);
		}
		break;
	case KEY_ENTER:
	case 10:
		if (cr->col_index == 0)
		{
			cur_state = old_state ? 0 : 1;
			cur_width = columns[0].cur_width;
			set_state(cr, cur_state);
			update_ui(cr);
			mvchgat(cur_index, 0, cur_width, A_REVERSE, cur_state, NULL);
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
		resize_ui(cr);
		break;
	}
}

int main(int argc, char *argv[])
{
	int ch;
	cursor cr = {
		.col_index = 0,
		.szh = 0,
		.szw = 0
	};
	columns[0].cur_index = 0;
	columns[1].cur_index = 0;

	setlocale(LC_ALL, "en_US.UTF-8");
	init_cfg(&cfg);

	if (argc > 1)
	{
		argp_parse(&argp, argc, argv, 0, 0, 0);
		exit(0);
	}

	initscr();
	use_default_colors();
	start_color();
	init_pair(1, COLOR_CYAN, COLOR_BLACK);
	keypad(stdscr, TRUE);
	cbreak();
	noecho();
	curs_set(0);
	getmaxyx(stdscr, cr.szh, cr.szw);

	columns[0].cur_width = cr.szw * COL0;
	columns[1].cur_width = cr.szw * COL1;

	init_structs(targets, columns);
	update_ui(&cr);
	mvchgat(0, 0, columns[0].cur_width, A_REVERSE, 0, NULL);
	refresh();

	while ((ch = getch()) != 'q')
	{
		key_bindings(ch, &cr);
		refresh();
	}

	endwin();
	exit(0);
}
