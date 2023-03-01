/**
 * libgen.c
 */

#include <stdio.h>
#include <sqlite3.h>

#include "libgen.h"

typedef struct
{
	char key[32];
	char val[1024];
} field;

const char *entry_types[] = {
	"@article",
	"@book",
	"@inbook",
	"@mvbook",
	"@booklet",
	"@collection",
	"@incollection",
	"@mvcollection",
	"@dataset",
	"@manual",
	"@misc",
	"@online",
	"@patent",
	"@periodical",
	"@proceedings",
	"@mvproceedings",
	"@report",
	"@thesis",
	"@unpublished"
};

static int ent_index; // entry index
static int sec_count; // section count
static cursor *pcr = NULL;
static config *cfg = NULL;
static target *targets = NULL;

static int init_entry(field *tmp, char *data)
{
	FILE *fp = NULL;
	char ch;
	int col, pos, len;

	if ((fp = fopen(data, "r")) == NULL)
	{
		fprintf(stderr, "argument error: %s\n", data);
		return 0;
	}

	col = 0; // column index
	pos = 0; // character position
	len = 0; // array length

	while ((ch = getc(fp)) != EOF)
	{
		if (ch == '\n') // LF
		{
			col = 0;
			pos = 0;
			len++;
		}
		else if (ch == '\t')
		{
			col = 1;
			pos = 0;
		}
		else if (col == 0)
		{
			tmp[len].key[pos] = ch;
			pos++;
		}
		else if (ch != '\r') // CR
		{
			tmp[len].val[pos] = ch;
			pos++;
		}
	}

	fclose(fp);
	return len;
}

static void delete(char *sql, char *key)
{
	sprintf(sql, "DELETE FROM entries WHERE entry_k='%s';", key);
}

static void insert(char *sql, char *data)
{
	int len, i;
	char key[2048], val[2048];
	field tmp[COL_COUNT];
	len = init_entry(tmp, data);
	if (len == 0)
		return;

	memset(key, 0, 2048);
	memset(val, 0, 2048);

	for (i = 0; i <= len; i++)
	{
		if (strlen(tmp[i].key) == 0)
			continue;

		strcat(key, tmp[i].key);
		strcat(val, tmp[i].val);
		strcat(key, ",");
		strcat(val, ",");
	}

	// remove last comma
	key[strlen(key) - 1] = 0;
	val[strlen(val) - 1] = 0;

	sprintf(sql, "INSERT INTO entries (%s) VALUES (%s);", key, val);
}

static void update(char *sql, char *data)
{
	int len, i;
	char key[256], pairs[2048];
	field tmp[COL_COUNT];
	len = init_entry(tmp, data);
	if (len == 0)
		return;

	memset(key, 0, 256);
	memset(pairs, 0, 2048);

	for (i = 0; i <= len; i++)
	{
		if (strlen(tmp[i].key) == 0)
			continue;

		if (strcmp(tmp[i].key, "entry_k") == 0)
		{
			strcpy(key, tmp[i].val);
			continue;
		}

		strcat(pairs, "\n");
		strcat(pairs, tmp[i].key);
		strcat(pairs, "=");
		strcat(pairs, tmp[i].val);
		strcat(pairs, ",");
	}

	// remove last comma
	pairs[strlen(pairs) - 1] = 0;

	sprintf(sql, "UPDATE entries \nSET %s \nWHERE entry_k=%s;", pairs, key);
}

static int call_0(__unused void *reserved, int argc, char **argv, char **col_name)
{
	//
	// print entries in bib format
	//
	int i, count = 0;
	char entry_k[32];
	char entry_t[128];
	field tmp[argc];

	for (i = 0; i < argc; i++)
	{
		if (argv[i] == NULL)
			continue;
		else if (strcmp(col_name[i], "entry_k") == 0)
		{
			memset(entry_k, 0, 32);
			strcpy(entry_k, argv[i]);
			continue;
		}
		else if (strcmp(col_name[i], "entry_t") == 0)
		{
			memset(entry_t, 0, 128);
			strcpy(entry_t, argv[i]);
			continue;
		}

		memset(tmp[count].key, 0, 32);
		memset(tmp[count].val, 0, 1024);

		strcpy(tmp[count].key, col_name[i]);
		strcpy(tmp[count].val, argv[i]);
		count++;
	}

	printf("%s{%s,\n", entry_t, entry_k);

	for (i = 0; i < count; i++)
	{
		printf("    %-18s= {%s},\n", tmp[i].key, tmp[i].val);
	}

	printf("}\n");
	printf("\n");
	return 0;
}

static int call_1(__unused void *reserved, int argc, char **argv, char **col_name)
{
	//
	// updating cursor data by SQL query
	//
	for (int i = 0; i < argc; i++)
	{
		if (strcmp(col_name[i], "title") == 0)
		{
			if (pcr->ent_index == ent_index)
			{
				memset(pcr->title, 0, 256);
				strcpy(pcr->title, argv[i]);
			}

			mvprintw(pcr->ent_count, pcr->sec_width, " %s", argv[i]);
			pcr->ent_count++;
		}
		else if (pcr->ent_index != ent_index)
		{
			continue;
		}
		else if (strcmp(col_name[i], "entry_k") == 0)
		{
			memset(pcr->entry_k, 0, 128);
			strcpy(pcr->entry_k, argv[i]);
		}
		else if (strcmp(col_name[i], "entry_t") == 0)
		{
			memset(pcr->entry_t, 0, 128);
			strcpy(pcr->entry_t, argv[i]);
		}
		else if (strcmp(col_name[i], "author") == 0)
		{
			memset(pcr->author, 0, 256);
			if (argv[i] == NULL)
				continue;
			strcpy(pcr->author, argv[i]);
		}
		else if (strcmp(col_name[i], "edition") == 0)
		{
			memset(pcr->edition, 0, 256);
			if (argv[i] == NULL)
				continue;
			strcpy(pcr->edition, argv[i]);
		}
		else if (strcmp(col_name[i], "editor") == 0)
		{
			memset(pcr->editor, 0, 256);
			if (argv[i] == NULL)
				continue;
			strcpy(pcr->editor, argv[i]);
		}
		else if (strcmp(col_name[i], "file") == 0)
		{
			memset(pcr->file, 0, 256);
			if (argv[i] == NULL)
				continue;
			strcpy(pcr->file, argv[i]);
		}
		else if (strcmp(col_name[i], "translator") == 0)
		{
			memset(pcr->translator, 0, 256);
			if (argv[i] == NULL)
				continue;
			strcpy(pcr->translator, argv[i]);
		}
		else if (strcmp(col_name[i], "keywords") == 0)
		{
			memset(pcr->keywords, 0, 256);
			if (argv[i] == NULL)
				continue;
			strcpy(pcr->keywords, argv[i]);
		}
		else if (strcmp(col_name[i], "year") == 0)
		{
			memset(pcr->year, 0, 256);
			if (argv[i] == NULL)
				continue;
			strcpy(pcr->year, argv[i]);
		}
	}
	ent_index++;
	return 0;
}

static int call_2(__unused void *reserved, __unused int argc, char **argv, __unused char **col_name)
{
	//
	// initializing targets
	//
	char *tmp;
	char keywords[256];

	if (argv[0] == NULL)
		return 0;

	memset(keywords, 0, 256);
	strcpy(keywords, argv[0]);

	tmp = strtok(keywords, ",");
	while (tmp != NULL)
	{
		for (int i = 0; i < SEC_COUNT; i++)
		{
			if (strcmp(tmp, targets[i].token) == 0)
				break;
			else if (strlen(targets[i].token))
				continue;
			memset(targets[sec_count].token, 0, 128);
			strcpy(targets[sec_count].token, tmp);
			sec_count++;
			break;
		}
		tmp = strtok(NULL, ",");
	}
	free(tmp);
	return 0;
}

int query_task(int ch, int mode, char *data)
{
	sqlite3 *db = NULL;
	char *error = NULL;
	char sql[4096];
	int rc;
	int (*callback)(void *, int, char **, char **);

	switch (mode)
	{
	case 0:
		callback = &call_0;
		break;
	case 1:
		callback = &call_1;
		break;
	case 2:
		callback = &call_2;
		break;
	}

	memset(sql, 0, 4096);

	rc = sqlite3_open(cfg->ldb, &db);

	if (rc)
	{
		fprintf(stderr, "error open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return rc;
	}

	switch (ch)
	{
	case ARGS_KEY_DELETE:
		delete (sql, data);
		break;
	case ARGS_KEY_INSERT:
		insert(sql, data);
		break;
	case ARGS_KEY_UPDATE:
		update(sql, data);
		break;
	case ARGS_KEY_QUERY:
		sprintf(sql, "%s", data);
		break;
	}

	if (strlen(sql) == 0)
	{
		fprintf(stderr, "invalid SQL query string\n");
		sqlite3_close(db);
		return -1;
	}

	rc = sqlite3_exec(db, sql, callback, 0, &error);

	if (rc == SQLITE_OK)
	{
		switch (ch)
		{
		case ARGS_KEY_DELETE:
			printf("entry delete successful\n");
			break;
		case ARGS_KEY_INSERT:
			printf("entry insert successful\n");
			break;
		case ARGS_KEY_UPDATE:
			printf("entry update successful\n");
			break;
		}
	}
	else
	{
		fprintf(stderr, "SQL error: %s\n", error);
		sqlite3_free(error);
	}

	sqlite3_close(db);
	return rc;
}

void init_cfg(config *pcfg)
{
	cfg = pcfg;
	cfg->ldb = getenv("LIBGEN_LDB");
	cfg->lib = getenv("LIBGEN_LIB");
	cfg->vi1 = getenv("LIBGEN_VI1");
	cfg->vi2 = getenv("LIBGEN_VI2");

	if (cfg->ldb == NULL)
	{
		cfg->ldb = calloc(strlen(LIBGEN_DB), sizeof(char));
		strcpy(cfg->ldb, LIBGEN_DB);
	}
}

void init_targets(target *ptargets)
{
	targets = ptargets;
	char sql[] = "SELECT keywords FROM entries;";
	query_task(ARGS_KEY_QUERY, 2, sql);
}

void update_ui(cursor *cr)
{
	int len = 0;
	int ind = 1;
	char sql[4096];
	char col[] = "entry_k,entry_t,keywords,author,edition,editor,title,translator,file,year";
	char tokens[2048];
	pcr = cr;
	pcr->sec_count = 0;
	pcr->ent_count = 0;
	ent_index = 0;
	clear();
	memset(tokens, 0, 2048);
	for (int i = 0; i < sec_count; i++)
	{
		mvprintw(pcr->sec_count, 0, " %s", targets[i].token);
		pcr->sec_count++;

		if (targets[i].state == 1)
		{
			mvchgat(i, 0, cr->sec_width, A_NORMAL, 1, NULL);
			if (len > 0)
				strcat(tokens, " AND ");
			strcat(tokens, "keywords LIKE '%%");
			strcat(tokens, targets[i].token);
			strcat(tokens, "%%'");
			len++;
		}
		else
		{
			mvchgat(i, 0, cr->sec_width, A_NORMAL, 0, NULL);
		}
	}
	//
	// query config
	//
	if (len > 0)
	{
		sprintf(sql, "SELECT %s FROM entries WHERE %s;", col, tokens);
	}
	else
	{
		sprintf(sql, "SELECT %s FROM entries;", col);
	}
	query_task(ARGS_KEY_QUERY, 1, sql);
	//
	// print details for selected entry
	//
	mvprintw(0, pcr->szw * COL2, "entry_k   : %s", pcr->entry_k);
	mvprintw(1, pcr->szw * COL2, "entry_t   : %s", pcr->entry_t);
	mvprintw(2, pcr->szw * COL2, "keywords  : %s", pcr->keywords);
	mvprintw(3, pcr->szw * COL2, "author    : %s", pcr->author);
	mvprintw(4, pcr->szw * COL2, "edition   : %s", pcr->edition);
	mvprintw(5, pcr->szw * COL2, "editor    : %s", pcr->editor);
	mvprintw(6, pcr->szw * COL2, "title     : %s", pcr->title);
	mvprintw(7, pcr->szw * COL2, "translator: %s", pcr->translator);
	mvprintw(8, pcr->szw * COL2, "file      : %s", pcr->file);
	mvprintw(9, pcr->szw * COL2, "year      : %s", pcr->year);
}