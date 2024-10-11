# libgen - Library Genesis Tools

`libgen` is an open source software designed to configure and store 
bibliographic data in a local database.

First of all, it is a command-line tool, with a very simple set of commands, 
which provides the following features:

- inserting an entry from a metadata file;
- updating an entry from a metadata file;
- deleting a entry by key from the database;
- processing requests.

In addition, `libgen` can be run interactively, which provides a simple 
interface for accessing the local database. Available features in interactive 
mode:

- sorting entries by keywords;
- launch the file viewer (currently only PDF and DJVU formats are supported).

## Dependencies

- `ncurses >= 6.3`
- `sqlite3 >= 3.34`

## Build and Install

To build and install libgen on your system, use the following commands:
```sh
meson setup build
ninja -C build/
ninja -C build/ install
```

## Environments

The `libgen` software package is configured by adding environment variables.

Set the path to the libgary directory. For example:
```sh
export LIBGEN_LIB=${HOME}/Library
```
Set the viewer for PDF files. For example:
```sh
export LIBGEN_VI1=mupdf
```
Set the viewer for DJVU files. For example:
```sh
export LIBGEN_VI2=djview
```
If you want to change the location of the database file, then you need to set a 
`LIBGEN_LDB` variable to override the path. By default, the path to the local 
database is set as `/usr/local/libgen/library.db`.

## Examples

To add an entry to the database, you need to create a metadata file, for example:
```
entry_k	"ebb9004fe72f0da390bdef45026b2786"
entry_t	"@book"
author	"Randal E. Bryant, David R. O'Hallaron"
edition	"3"
file	"computer-systems-3ed-en.pdf"
isbn	"978-1-488-67207-1"
keywords	"Computers,Systems"
language	"english"
pages	"1105"
publisher	"Pearson Education Ltd"
title	"Computer Systems"
year	"2016"
```
It is important that the separator between the field and the value is a tab 
character. In addition, the value of the `entry_k` field must be unique. The 
key for entry can be obtained using the `md5sum` command.

Now that the data is prepared, save the file with the `*.txt` extension, for 
example:
```
computer-systems-3ed-en.pdf.txt
```
Next, add an entry to the database:
```sh
libgen -i "computer-systems-3ed-en.pdf.txt"
```
If you need to update the entry, just change the metadata file and update the 
database:
```sh
libgen -u "computer-systems-3ed-en.pdf.txt"
```
It is important that the key is not changed when updating the entry. Otherwise, 
you will get an error.

Finally, if you need to delete this entry from the database, do the following:
```sh
libgen -d "ebb9004fe72f0da390bdef45026b2786"
```
As you can see, it's pretty simple.
