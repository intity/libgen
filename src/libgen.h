/**
 * libgen.h
 */

#ifndef LIBGEN_H
#define LIBGEN_H

#define LIBGEN_DB "/usr/local/libgen/library.db"
#define COL_COUNT 100
#define SEC_COUNT 100

/* width columns */
#define COL0 0.2 // (20%) 1/5
#define COL1 0.3 // (30%) 1/2-1/5
#define COL2 0.5 // (50%) 1/2

/* option keys */
#define ARGS_KEY_DELETE	'd'
#define ARGS_KEY_INSERT	'i'
#define ARGS_KEY_UPDATE	'u'
#define ARGS_KEY_QUERY	'q'

#include <stdlib.h>
#include <curses.h>
#include <string.h>

#ifndef __unused
#define __unused __attribute__((__unused__))
#endif

typedef struct
{
	int state; // (0) unselected, (1) selected
	char token[128];
} target;

typedef struct
{
	char *ldb; // local database
	char *lib; // library directory
	char *vi1; // viewer for PDF files
	char *vi2; // viewer for DJVU files
} config;

typedef struct
{
	int col_index; // column index
	int ent_index; // entry index
	int ent_width; // entry column width
	int ent_count; // entry row count
	int sec_index; // section index
	int sec_width; // section column width
	int sec_count; // section row count
	int szh; // window height/row height
	int szw; // window width
	/* custom fields */
	char entry_k[128]; // entry cite key
	char entry_t[128]; // entry type
	/* specific fields */
	char keywords[256];
	/* standard fields */
	char author[256];
	char edition[256];
	char editor[256];
	char translator[256];
	char title[256];
	char file[256]; // book filename (example: book_name.djvu)
	char year[128];
} cursor;

/**
 * Initialize config
 * @pcfg config pointer
 */
void init_cfg(config *pcfg);

/**
 * Initialize keywords
 * @ptargets
 */
void init_targets(target *ptargets);

/**
 * the update UI function
 * @pcr cursor pointer
 */
void update_ui(cursor *pcr);

/**
 * Query task function
 * @ch option
 * @mode (0) print, (1) configure, (2) init
 * @data metadata file (optarg)
 */
int query_task(int ch, int mode, char *data);

#endif /* LIBGEN_H */