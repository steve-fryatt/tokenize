/* Copyright 2014, Stephen Fryatt (info@stevefryatt.org.uk)
 *
 * This file is part of Tokenize:
 *
 *   http://www.stevefryatt.org.uk/software/
 *
 * Licensed under the EUPL, Version 1.1 only (the "Licence");
 * You may not use this work except in compliance with the
 * Licence.
 *
 * You may obtain a copy of the Licence at:
 *
 *   http://joinup.ec.europa.eu/software/page/eupl
 *
 * Unless required by applicable law or agreed to in
 * writing, software distributed under the Licence is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES
 * OR CONDITIONS OF ANY KIND, either express or implied.
 *
 * See the Licence for the specific language governing
 * permissions and limitations under the Licence.
 */

/**
 * \file parse.c
 *
 * Basic line parser, implementation.
 */

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Local source headers. */

#include "parse.h"


#define MAX_TOKENISED_LINE 1024

struct keyword {
	char		*name;		/**< The name of the keyword.				*/
	int		abbrev;		/**< The minimum number of characters allowed.		*/
	unsigned	start;		/**< The token if at the start of a statement.		*/
	unsigned	elsewhere;	/**< The token if elsewhere in a statement.		*/
};

/**
 * The table of known keywords and their tokens.  The keywords *must* be in
 * alphabetical order, to allow the search routine to operate.
 */

#define MAX_KEYWORDS 6

static struct keyword parse_keywords[] = {
	{"ABS",		3,	0x94,	0x94	},	/* 0	*/
	{"ACS",		3,	0x95,	0x95	},	/* 1	*/
	{"ADVAL",	2,	0x96,	0x96	},	/* 2	*/
	{"AND",		1,	0x80,	0x80	},	/* 3	*/
	{"APPEND",	2,	0x8ec7,	0x8ec7	},	/* 4	*/
	{"ASC",		3,	0x97,	0x97	},	/* 5	*/
	{"ASN",		3,	0x98,	0x98	},	/* 6	*/
	{"ATN",		3,	0x99,	0x99	},	/* 7	*/
	{"AUTO",	2,	0x8fc7,	0x8fc7	},	/* 8	*/
	{"BEAT",	4,	0x8fc6,	0x8fc6	},	/* 9	*/
	{"BEATS",	3,	0x9ec8,	0x9ec8	},	/* 10	*/
	{"BGET",	1,	0x9a,	0x9a	},	/* 11	*/
	{"BPUT",	2,	0xd5,	0xd5	},	/* 12	*/
	{"CALL",	2,	0xd6,	0xd6	},	/* 13	*/
	{"CASE",	4,	0x8ec8,	0x8ec8	},	/* 14	*/
	{"CHAIN",	2,	0xd7,	0xd7	},	/* 15	*/
	{"CHR$",	4,	0xbd,	0xbd	},	/* 16	*/
	{"CIRCLE",	2,	0x8fc8,	0x8fc8	},	/* 17	*/
	{"CLEAR",	2,	0xd8,	0xd8	},	/* 18	*/
	{"CLG",		3,	0xda,	0xda	},	/* 19	*/
	{"CLOSE",	3,	0xd9,	0xd9	},	/* 20	*/
	{"CLS",		3,	0xdb,	0xdb	},	/* 21	*/
	{"COLOR",	1,	0xfb,	0xfb	},	/* 22	*/
	{"COLOUR",	1,	0xfb,	0xfb	},	/* 23	*/
	{"COS",		3,	0x9b,	0x9b	},	/* 24	*/
	{"COUNT",	3,	0x9c,	0x96	},	/* 25	*/
	{"CRUNCH",	2,	0x90c7,	0x90c7	},	/* 26	*/
	{"DATA",	1,	0xdc,	0xdc	},	/* 27	*/
	{"DEF",		3,	0xdd,	0xdd	},	/* 28	*/
	{"DEG",		3,	0x9d,	0x9d	},	/* 29	*/
	{"DELETE",	3,	0x91c7,	0x91c7	},	/* 30	*/
	{"DIM",		3,	0xde,	0xde	},	/* 31	*/
	{"DIV",		3,	0x81,	0x81	},	/* 32	*/
	{"DRAW",	2,	0xdf,	0xdf	},	/* 33	*/
	{"EDIT",	2,	0x92c7,	0x92c7	},	/* 34	*/
	{"ELLIPSE",	3,	0x9dc8,	0x9dc8	},	/* 35	*/
	{"ELSE",	2,	0xcc,	0xcc	},	/* 36	*/
	{"END",		3,	0xe0,	0xe0	},	/* 37	*/
	{"ENDCASE",	4,	0xcb,	0xcb	},	/* 38	*/
	{"ENDIF",	5,	0xcd,	0xcd	},	/* 39	*/
	{"ENDPROC",	1,	0xe1,	0xe1	},	/* 40	*/
	{"ENDWHILE",	4,	0xce,	0xce	},	/* 41	*/
	{"EOF",		3,	0xc5,	0xc5	},	/* 42	*/
	{"EOR",		3,	0x82,	0x82	},	/* 43	*/
	{"ERL",		3,	0x93,	0x93	},	/* 44	*/
	{"ERR",		3,	0x9f,	0x9f	},	/* 45	*/
	{"ERROR",	3,	0x85,	0x85	},	/* 46	*/
	{"EVAL",	2,	0xa0,	0xa0	},	/* 47	*/
	{"EXP",		3,	0xa1,	0xa1	},	/* 48	*/
	{"EXT",		3,	0xa2,	0xa2	},	/* 49	*/
	{"FALSE",	2,	0xa3,	0xa3	},	/* 50	*/
	{"FILL",	2,	0x90c8,	0x90c8	},	/* 51	*/
	{"FN",		2,	0xa4,	0xa4	},	/* 52	*/
	{"FOR",		1,	0xe3,	0xe3	},	/* 53	*/
	{"GCOL",	2,	0xe6,	0xe6	},	/* 54	*/
	{"GET",		3,	0xa5,	0xa5	},	/* 55	*/
	{"GET$",	2,	0xbe,	0xbe	},	/* 56	*/
	{"GOSUB",	3,	0xe4,	0xe4	},	/* 57	*/
	{"GOTO",	1,	0xe5,	0xe5	},	/* 58	*/
	{"HELP",	2,	0x93c7,	0x93c7	},	/* 59	*/
	{"HIMEM",	1,	0xd3,	0x93	},	/* 60	*/
	{"IF",		2,	0xe7,	0xe7	},	/* 61	*/
	{"INKEY",	5,	0xa6,	0xa6	},	/* 62	*/
	{"INKEY$",	3,	0xbf,	0xbf	},	/* 63	*/
	{"INPUT",	1,	0xe8,	0xe8	},	/* 64	*/
	{"INSTALL",	5,	0x9ac8,	0x9ac8	},	/* 65	*/
	{"INSTR(",	3,	0xa7,	0xa7	},	/* 66	*/
	{"INT",		3,	0xa8,	0xa8	},	/* 67	*/
	{"LEFT$",	2,	0xc0,	0xc0	},	/* 68	*/
	{"LEN",		3,	0xa9,	0xa9	},	/* 69	*/
	{"LET",		3,	0xe9,	0xe9	},	/* 70	*/
	{"LIBRARY",	3,	0x9bc8,	0x9bc8	},	/* 71	*/
	{"LINE",	4,	0x86,	0x86	},	/* 72	*/
	{"LIST",	1,	0x94c7,	0x94c7	},	/* 73	*/
	{"LN",		2,	0xaa,	0xaa	},	/* 74	*/
	{"LOAD",	2,	0x95c7,	0x95c7	},	/* 75	*/
	{"LOCAL",	3,	0xea,	0xea	},	/* 76	*/
	{"LOG",		3,	0xab,	0xab	},	/* 77	*/
	{"LOMEM",	3,	0xd2,	0x92	},	/* 78	*/
	{"LVAR",	2,	0x96c7,	0x96c7	},	/* 79	*/
	{"MID$(",	1,	0xc1,	0xc1	},	/* 80	*/
	{"MOD",		3,	0x83,	0x83	},	/* 81	*/
	{"MODE",	2,	0xeb,	0xeb	},	/* 82	*/
	{"MOUSE",	3,	0x97c8,	0x97c8	},	/* 83	*/
	{"MOVE",	4,	0xec,	0xec	},	/* 84	*/
	{"NEW",		3,	0x97c7,	0x97c7	},	/* 85	*/
	{"NEXT",	1,	0xed,	0xed	},	/* 86	*/
	{"NOT",		3,	0xab,	0xab	},	/* 87	*/
	

	{"REM",		3,	0xf4,	0xf4	},	/* 88	*/

	{"SYS",		3,	0x99c8,	0x99c8	},	/* 89	*/

	{"[",		1,	0x00,	0x00	}
};

/**
 * Indexes into the keywords table for the various initial letters. -1
 * indicates that there are no keywords starting with that letter.
 */

static int parse_keyword_index[] = {
	0,	/**< A	*/
	9,	/**< B	*/
	13,	/**< C	*/
	27,	/**< D	*/
	34,	/**< E	*/
	50,	/**< F	*/
	54,	/**< G	*/
	59,	/**< H	*/
	61,	/**< I	*/
	-1,	/**< J	*/
	-1,	/**< K	*/
	68,	/**< L	*/
	80,	/**< M	*/
	85,	/**< N	*/
	-1,	/**< O	*/
	-1,	/**< P	*/
	-1,	/**< Q	*/
	88,	/**< R	*/
	89,	/**< S	*/
	-1,	/**< T	*/
	-1,	/**< U	*/
	-1,	/**< V	*/
	-1,	/**< W	*/
	-1,	/**< X	*/
	-1,	/**< Y	*/
	-1	/**< Z	*/
};

static char parse_buffer[MAX_TOKENISED_LINE];


int parse_match_token(char **buffer);

/**
 * Parse a line of BASIC, returning a pointer to the tokenised form which will
 * remain valid until the function is called again.
 *
 * \param *line		Pointer to the line to process,
 * \param *assembler	Pointer to a boolean which is TRUE if we are in an
 *			assember section and FALSE otherwise; updated on exit.
 * \return		Pointer to the tokenised line, or NULL on error.
 */

char *parse_process_line(char *line, bool *assembler)
{
	char			*read = line, *write = parse_buffer;
	unsigned		line_number = 0;
	int			token;
	enum parse_error	error = PARSE_NO_ERROR;

	bool	statement_start = true;		/**< True while we're at the start of a statement.	*/

	/* Skip any leading whitespace on the line. */

	while (*read != '\n' && isspace(*read))
		read++;

	/* If there's a line number, read and process it. */

	while (*read != '\n' && isdigit(*read))
		*write++ = *read++;

	if (write > parse_buffer) {
		*write = '\0';
		line_number = atoi(parse_buffer);
		write = parse_buffer;
	}

	/* Again, trim any whitespace that followed the line number. */

	while (*read != '\n' && isspace(*read))
		read++;

	while (*read != '\n') {
		if (*assembler == true) {
			/* Assembler is a special case. We just copy text across
			 * to the buffer until we find a closing delimiter.
			 */

			if (*read == ']')
				*assembler = false;

			*write++ = *read++;
		} else if (*read == '[') {
			/* This is the start of an assembler block. */
		
			*assembler = true;
			*write++ = *read++;
		} else if (*read == '\"') {
			/* Copy strings as a lump. */

			bool string_closed = false;

			*write++ = *read++;

			while (*read != '\n' && !string_closed) {
				if (*read == '\"' && *(read + 1) != '\"')
					string_closed = true;
				else if (*read == '\"' && *(read + 1) == '\"')
					*write++ = *read++;

				*write++ = *read++;
			}

			if (!string_closed) {
				printf("Missing string terminator");
			}
		} else if (*read == ':') {
			/* Handle breaks in statements. */
		
			read++;
		} else if (*read >= 'A' && *read <= 'Z' && (token = parse_match_token(&read)) != -1) {
			printf("Found token %d: %s\n", token, parse_keywords[token].name);
		} else {
			read++;
		}
	}

	*write = '\0';

	printf("Line (%d): %s\n", line_number, parse_buffer);


	return NULL;
}


int parse_match_token(char **buffer)
{
	char	*start = *buffer;
	int	keyword = 0;
	int	result = 0;
	int	full = -1;
	char	*full_end = NULL;
	int	partial = -1;
	char	*partial_end = NULL;

	/* If the code doesn't start with an upper case letter, it's not a keyword */

	if (buffer == NULL || *start < 'A' || *start > 'Z')
		return -1;

	/* Find the first entry in the keyword table that will match. */

	keyword = parse_keyword_index[*start - 'A'];
	printf ("Try to match on %c, index %d\n", *start, keyword);
	if (keyword == -1)
		return -1;

	do {
		char *test = start, *match = parse_keywords[keyword].name;

		printf("Entering loop: keyword=%d, test=%c, match=%c\n", keyword, *test, *match);
		
		while (*test == *match && *match != '\0' && *test != '.' && *test >= 'A' && *test <= 'Z') {
			printf("Comparing test=%c with match=%c\n", *test, *match);
			test++;
			match++;
		}

		printf("At end, test=%c and match=%c\n", *test, *match);

		if (*match == '\0') {
			result = 0;
			full = keyword;
			full_end = test;
		} else if (*test == '.') {
			printf("Backstep, test=%c and match=%c\n", *(test - 1), *(match - 1));
			result = *(match - 1) - *(test - 1);
			if ((test - start) >= parse_keywords[keyword].abbrev) {
				partial = keyword;
				partial_end = test;
			}
		} else {
			result = *match - *test;
		}
		
		printf("result=%d\n", result);
		
		keyword++;
	} while (result < 0);

	if (full != -1) {
		*buffer = full_end;
		return full;
	}

	if (partial != -1) {
		*buffer = partial_end;
		return partial;
	}

	return -1;
}


