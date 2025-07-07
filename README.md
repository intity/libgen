# libgen-tools

`libgen` is an open source software designed to configure and store 
bibliographic data in a local database.

First of all, it is a command-line tool, with a very simple set of commands, 
which provides the following features:

- inserting an entry from a metadata file;
- updating an entry from a metadata file;
- deleting a entry by key from the database;
- processing requests.

In addition, `libgen` can be run interactively, which provides a simple 
interface for accessing the local database.

Available features in interactive mode:

- sorting entries by keywords;
- launch the file viewer:
    - MSYS2: currently only PDF and DJVU formats are supported.

## Dependencies

- `ncurses >= 6.5`
- `sqlite3 >= 3.47`

## Building and installation of tools

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

Set the viewer by default. For example:

```sh
export LIBGEN_VID=zathura
```

If you want to change the location of the database file, then you need to set a 
`LIBGEN_LDB` variable to override the path. By default, the path to the local 
database is set as `/usr/local/libgen/library.db`.

## Example of use

Create an empty file in a text editor:

```sh
$EDITOR computer-systems-3ed-en.epub.txt
```

Fill in the fields in text format [key\tvalue\n], for example:

```txt
entry_k	"17211a61042ffc033762b87eea867beb"
entry_t	"@book"
subject "computer-sci"
edition	"3"
author	"Randal E. Bryant, David R. O'Hallaron"
date    "2016"
isbn	"978-1-488-67207-1"
file	"computer-systems-3ed-en.epub"
pages	"1105"
title	"Computer Systems"
subtitle	"A Programmer's Perspective"
keywords	"computer-sci,systems"
language	"english"
publisher	"Pearson Education Ltd"
```

It is important that the separator between the field and the value is a tab 
character. In addition, the value of the `entry_k` field must be unique. The 
key for entry can be obtained using the `md5sum` command.

Next, add an entry to the database:

```sh
libgen -i "computer-systems-3ed-en.epub.txt"
```

For additional control, you can print the screen recording:

```sh
lingen -o "17211a61042ffc033762b87eea867beb"
```

To update a record in the database, use the same metadata file you applied earlier, for example:

```sh
libgen -u "computer-systems-3ed-en.pdf.txt"
```

>It is important that the key is not changed when updating the record, otherwise you will get an error.

And if you need to delete a record from the database, do this:

```sh
libgen -d "17211a61042ffc033762b87eea867beb"
```

As you can see, it's pretty simple.
