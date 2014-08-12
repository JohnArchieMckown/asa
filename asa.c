/* 
 *  asa.c - interpret ASA carriage control characters
 *  Copyright (C) 1994 Thomas Koenig
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


/* System Headers */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* Macros */

#define LINESIZE 135
#define NUMLINES 5
#define MAX(a,b) ((a) > (b) ? (a) : (b))

/* Structures and unions */

typedef struct
{
    char *chr;
    size_t len;
} str;

/* File scope variables */

static char rcsid[] = "$Id: asa.c,v 1.4 1994/04/12 22:43:11 tom Exp $";
static str *line_buffer = (str *) 0;
static size_t line_num = 0;
static size_t linebuf_size = 0;

/* Function declarations */

static void *mymalloc(size_t);
static size_t readline(FILE *fp, char **a);
static void form_feed(void);
static void add_line(str);
static void flush(void);
static void copy_file(FILE *fp);
  
/* Local functions */

static void*
mymalloc(size_t n)
{
    void *p;
    if ((p=malloc(n))) {
	return p;
    }
    else
    {
	fprintf(stderr,"Virtual memory exhausted\n");
	exit(EXIT_FAILURE);
    }
}

static void*
myrealloc(void *p, size_t n)
{
    if ((p=realloc(p,n))) {
	return p;
    }
    else
    {
	fprintf(stderr,"Virtual memory exhausted\n");
	exit(EXIT_FAILURE);
    }
}
static void
form_feed()
{
    putchar('\f');
}

static void
new_line()
{
    putchar('\n');
}

static void
add_line(str a)
{
    if (line_num >= linebuf_size)
    {
	linebuf_size += NUMLINES;
	line_buffer = myrealloc(line_buffer, linebuf_size*sizeof(str));
    }
    line_buffer[line_num]=a;
    line_num++;
}

static void
flush()
{
    size_t i,j;
    size_t max_len;

    if (line_num == 0)
	return;
    if (line_num == 1)
    {
	printf("%s\n", line_buffer[0].chr +1);
	line_num = 0;
	return;
    }
    max_len = 0;
    for (j=0; j< line_num; j++)
	max_len = MAX(max_len, line_buffer[j].len);

    for (i=1; i<= max_len;  i++)
    {
	int printed = 0;

	for (j=0; j< line_num; j++)
	{
	    if (line_buffer[j].len > i)
	    {
		int ch = line_buffer[j].chr[i];

		if (ch != ' ') {
	            if (printed)
			putchar('\b');
		    putchar(ch);
		    printed = 1;
		}
	    }
	}
	if (!printed)
	    putchar(' ');
    }
    for (j=0; j<line_num; j++)
	free(line_buffer[j].chr);

    line_num = 0;
    putchar('\n');
}

static size_t
readline(FILE *fp, char **ret)
{
    static char *buffer=(char *) 0;
    char *ret_buff;
    static int bufsize = LINESIZE;
    int ch;
    size_t len=0;
    int inc;
    int i;

    if (buffer == (char*) 0)
	buffer = (char *) mymalloc(LINESIZE);

    while(1)
    {
        ch = fgetc(fp);
	if (ch == EOF)
	    break;
	if (ch == '\t')
	{
	    ch = ' ';
	    inc = 8 - (len % 8);
	}
	else
	    inc = 1;

	if (len + inc > bufsize-2)
	{
	    bufsize += LINESIZE;
	    buffer = myrealloc(buffer, bufsize);
	}
	for (i=0; i<inc; i++)
	    buffer[len+i] = ch;
	len += inc;
	if (ch == '\n')
	    break;
    }
    buffer[len]='\0';
    ret_buff = mymalloc(len+1);
    strcpy(ret_buff, buffer);
    *ret = ret_buff;
    return len;
}

static void
copy_file(FILE *fp)
{
    str line;
    static first_line = 1;

    while ((line.len=readline(fp, &(line.chr))))
    {
	if (line.chr[line.len-1] == '\n')
	{
	   line.chr[line.len-1] = '\0';
	   line.len--;
	}

	switch (line.chr[0])
	{
	case '+':
	    add_line(line);
	    break;

	case '0':
	    flush();
	    new_line();
	    add_line(line);
	    break;

	case '1':
	    flush();
	    if (! first_line)
		form_feed();
	    add_line(line);
	    break;

	case ' ':
	    flush();
	    add_line(line);
	    break;

	default:
	    flush();
	    add_line(line);
	    break;
	}
        first_line = 0;
    }
}

/* Global functions */

int
main(int argc, char **argv)
{

    if (argc == 1)
	copy_file(stdin);
    else
    {
	FILE *fp;
	char *fname;

	while(--argc > 0)
	{
	    fname = *++argv;
	    fp = fopen(fname, "r");
	    if (fp == NULL)
	    {
		fprintf(stderr,"Cannot open %s: %s\n",fname,strerror(errno));
		exit(EXIT_FAILURE);
	    }
	    else {
		copy_file(fp);
		fclose(fp);
	    }
	}
    }
    flush();
    exit(EXIT_SUCCESS);
}
