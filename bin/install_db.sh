#!/bin/sh

LIBGEN_DIR=/usr/local/libgen
LIBGEN_LDB=$LIBGEN_DIR/library.db
[ ! -d $LIBGEN_DIR ] && mkdir $LIBGEN_DIR

# install local database
if [ ! -f $LIBGEN_LDB ]; then
	sqlite3 $LIBGEN_LDB < ../dataset/scheme.sql
fi