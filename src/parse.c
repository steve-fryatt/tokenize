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

#include "library.h"


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
	{"ELSE",	2,	0xcc,	0x8b	},	/* 36	*/
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
	{"NOT",		3,	0xac,	0xac	},	/* 87	*/
	{"OF",		2,	0xca,	0xca	},	/* 88	*/
	{"OFF",		3,	0x87,	0x87	},	/* 89	*/
	{"OLD",		1,	0x98c7,	0x98c7	},	/* 90	*/
	{"ON",		2,	0xee,	0xee	},	/* 91	*/
	{"OPENIN",	2,	0x8e,	0x8e	},	/* 92	*/
	{"OPENOUT",	5,	0xae,	0xae	},	/* 93	*/
	{"OPENUP",	6,	0xad,	0xad	},	/* 94	*/
	{"OR",		2,	0x84,	0x84	},	/* 95	*/
	{"ORIGIN",	2,	0x91c8,	0x91c8	},	/* 96	*/
	{"OSCLI",	2,	0xff,	0xff	},	/* 97	*/
	{"OTHERWISE",	2,	0x7f,	0x7f	},	/* 98	*/
	{"OVERLAY",	2,	0xa3c8,	0xa3c8	},	/* 99	*/
	{"PAGE",	2,	0xd0,	0x90	},	/* 100	*/
	{"PI",		2,	0xaf,	0xaf	},	/* 101	*/
	{"PLOT",	2,	0xf0,	0xf0	},	/* 102	*/
	{"POINT",	5,	0x92c8,	0x92c8	},	/* 103	*/
	{"POINT(",	2,	0xb0,	0xb0	},	/* 104	*/
	{"POS",		3,	0xb1,	0xb1	},	/* 105	*/
	{"PRINT",	1,	0xf1,	0xf1	},	/* 106	*/
	{"PROC",	4,	0xf2,	0xf2	},	/* 107	*/
	{"PTR",		3,	0xcf,	0x8f	},	/* 108	*/
	{"QUIT",	1,	0x98c8,	0x98c8	},	/* 109	*/
	{"READ",	4,	0xf3,	0xf3	},	/* 110	*/
	{"RECTANGLE",	3,	0x93c8,	0x93c8	},	/* 111	*/
	{"REM",		3,	0xf4,	0xf4	},	/* 112	*/
	{"RENUMBER",	3,	0x99c7,	0x99c7	},	/* 113	*/
	{"REPEAT",	3,	0xf5,	0xf5	},	/* 114	*/
	{"REPORT",	4,	0xf6,	0xf6	},	/* 115	*/
	{"RESTORE",	3,	0xf7,	0xf7	},	/* 116	*/
	{"RETURN",	1,	0xf8,	0xf8	},	/* 117	*/
	{"RIGHT$(",	2,	0xc2,	0xc2	},	/* 118	*/
	{"RND",		3,	0xb3,	0xb3	},	/* 119	*/
	{"RUN",		3,	0xf9,	0xf9	},	/* 120	*/
	{"SAVE",	2,	0x9ac7,	0x9ac7	},	/* 121	*/
	{"SGN",		3,	0xb4,	0xb4	},	/* 122	*/
	{"SIN",		3,	0xb5,	0xb5	},	/* 123	*/
	{"SOUND",	2,	0xd4,	0xd4	},	/* 124	*/
	{"SPC",		3,	0x89,	0x89	},	/* 125	*/
	{"SQR",		3,	0xb6,	0xb6	},	/* 126	*/
	{"STEP",	1,	0x88,	0x88	},	/* 127	*/
	{"STEREO",	4,	0xa2c8,	0xa2c8	},	/* 128	*/
	{"STOP",	4,	0xfa,	0xfa	},	/* 129	*/
	{"STR$",	4,	0xc3,	0xc3	},	/* 130	*/
	{"STRING$(",	4,	0xc4,	0xc4	},	/* 131	*/
	{"SUM",		3,	0x8ec6,	0x8ec6	},	/* 132	*/
	{"SWAP",	2,	0x94c8,	0x94c8	},	/* 133	*/
	{"SYS",		3,	0x99c8,	0x99c8	},	/* 134	*/
	{"TAB(",	4,	0x8a,	0x8a	},	/* 135	*/
	{"TAN",		1,	0xb7,	0xb7	},	/* 136	*/
	{"TEMPO",	2,	0x9fc8,	0x9fc8	},	/* 137	*/
	{"TEXTLOAD",	5,	0x9bc7,	0x9bc7	},	/* 138	*/
	{"TEXTSAVE",	5,	0x9cc7,	0x9cc7	},	/* 139	*/
	{"THEN",	2,	0x8c,	0x8c	},	/* 140	*/
	{"TIME",	2,	0xd1,	0x91	},	/* 141	*/
	{"TINT",	4,	0x9cc8,	0x9cc8	},	/* 142	*/
	{"TO",		2,	0xb8,	0xb8	},	/* 143	*/
	{"TRACE",	2,	0xfc,	0xfc	},	/* 144	*/
	{"TRUE",	4,	0xb9,	0xb9	},	/* 145	*/
	{"TWIN",	4,	0x9dc7,	0x9dc7	},	/* 146	*/
	{"TWINO",	2,	0x9ec7,	0x9ec7	},	/* 147	*/
	{"UNTIL",	1,	0xfd,	0xfd	},	/* 148	*/
	{"USR",		3,	0xba,	0xba	},	/* 149	*/
	{"VAL",		3,	0xbb,	0xbb	},	/* 150	*/
	{"VDU",		1,	0xef,	0xef	},	/* 151	*/
	{"VOICE",	5,	0xa1c8,	0xa1c8	},	/* 152	*/
	{"VOICES",	2,	0xa0c8,	0xa0c8	},	/* 153	*/
	{"VPOS",	2,	0xbc,	0xbc	},	/* 154	*/
	{"WAIT",	2,	0x96c8,	0x96c8	},	/* 155	*/
	{"WHEN",	4,	0xc9,	0xc9	},	/* 156	*/
	{"WHILE",	1,	0x95c8,	0x95c8	},	/* 157	*/
	{"WIDTH",	2,	0xfe,	0xfe	},	/* 158	*/
	{"[",		1,	0x00,	0x00	}	/* TERM	*/
};

/**
 * Indexes into the keywords table for the various initial letters. -1
 * indicates that there are no keywords starting with that letter.
 */

static int parse_keyword_index[] = {
	KWD_ABS,	/**< A	*/
	KWD_BEAT,	/**< B	*/
	KWD_CALL,	/**< C	*/
	KWD_DATA,	/**< D	*/
	KWD_EDIT,	/**< E	*/
	KWD_FALSE,	/**< F	*/
	KWD_GCOL,	/**< G	*/
	KWD_HELP,	/**< H	*/
	KWD_IF,		/**< I	*/
	-1,		/**< J	*/
	-1,		/**< K	*/
	KWD_LEFT$,	/**< L	*/
	KWD_MID$,	/**< M	*/
	KWD_NEW,	/**< N	*/
	KWD_OF,		/**< O	*/
	KWD_PAGE,	/**< P	*/
	KWD_QUIT,	/**< Q	*/
	KWD_READ,	/**< R	*/
	KWD_SAVE,	/**< S	*/
	KWD_TAB,	/**< T	*/
	KWD_UNTIL,	/**< U	*/
	KWD_VAL,	/**< V	*/
	KWD_WAIT,	/**< W	*/
	-1,		/**< X	*/
	-1,		/**< Y	*/
	-1		/**< Z	*/
};

static char parse_buffer[MAX_TOKENISED_LINE];
static char library_path[MAX_TOKENISED_LINE];


static enum parse_status parse_process_statement(char **read, char **write, int *real_pos, struct parse_options *options, bool *assembler, bool line_start);
static int parse_match_token(char **buffer);
static bool parse_process_string(char **read, char **write, char *dump);
static void parse_process_numeric_constant(char **read, char **write);
static void parse_process_binary_constant(char **read, char **write, int *extra_spaces);
static void parse_process_fnproc(char **read, char **write);
static void parse_process_variable(char **read, char **write);
static void parse_process_to_line_end(char **read, char **write);

/**
 * Parse a line of BASIC, returning a pointer to the tokenised form which will
 * remain valid until the function is called again.
 *
 * \param *line		Pointer to the line to process,
 * \param *options	Parse options block to set the configuration
 * \param *assembler	Pointer to a boolean which is TRUE if we are in an
 *			assember section and FALSE otherwise; updated on exit.
 * \param *line_number	Pointer to a variable to hold the proposed next line
 *			number; updated on exit if a number was found.
 * \return		Pointer to the tokenised line, or NULL on error.
 */

char *parse_process_line(char *line, struct parse_options *options, bool *assembler, unsigned *line_number)
{
	char			*read = line, *write = parse_buffer;
	unsigned		read_number = 0;
	int			leading_spaces = 0;

	bool	line_start = true;		/**< True while we're at the start of a line.				*/
	int	real_pos = 0;			/**< The real position in the line, including expanded  keywords.	*/
	bool	all_deleted = true;		/**< True while all the statements on the line have been deleted.	*/

	/* Skip any leading whitespace on the line. */

	while (*read != '\n' && isspace(*read)) {
		if (*read == '\t' && options->tab_indent > 0) {
			leading_spaces = ((leading_spaces + 1) / options->tab_indent) + options->tab_indent;
		} else if (options->tab_indent > 0) {
			leading_spaces++;
		}
	
		read++;
	}

	/* If there's a line number, read and process it. */

	while (*read != '\n' && isdigit(*read))
		*write++ = *read++;

	if (write > parse_buffer) {
		*write = '\0';
		read_number = atoi(parse_buffer);
		write = parse_buffer;
		leading_spaces = 0;

		if (read_number > *line_number)
			*line_number = read_number;
		else if (read_number < *line_number) {
			printf("Line number error\n");
			return NULL;
		}
	}

	/* Again, trim any whitespace that followed the line number. */

	while (*read != '\n' && isspace(*read)) {
		if (*read == '\t' && options->tab_indent > 0) {
			leading_spaces = ((leading_spaces + 1) / options->tab_indent) + options->tab_indent;
		} else if (options->tab_indent > 0) {
			leading_spaces++;
		}
	
		read++;
	}

	/* Output the line start (CR, LineNo HI, LineNo LO, Length). */

	*write++ = 0x0d;
	*write++ = (*line_number & 0xff00) >> 8;
	*write++ = (*line_number & 0x00ff);
	*write++ = 0;

	/* Unless we're stripping all whitespace, output the line indent. */

	if (!options->crunch_all_whitespace) {
		real_pos = leading_spaces;

		for (; leading_spaces > 0; leading_spaces--)
			*write++ = ' ';
	}

	/* Process statements from the line, sending them to the output buffer. */

	while (*read != '\n') {
		enum parse_status status = parse_process_statement(&read, &write, &real_pos, options, assembler, line_start);

		if (status != PARSE_DELETED) {
			if (all_deleted == true)
				all_deleted = false;

			if (*read == ':') {
				*write++ = *read++;
				real_pos++;

				if (options->crunch_body_rems == true && options->crunch_rems == false && status != PARSE_COMMENT)
					options->crunch_rems = true;
			}
		} else if (*read == ':') {
			read++;
		}

		line_start = false;
	}

	/* Write the line length, and terminate the buffer. */

	if (all_deleted == true) {
		*parse_buffer = '\0';
	} else {
		*(parse_buffer + 3) = (write - parse_buffer) & 0xff;
		*write = '\0';
	}

	return parse_buffer;
}


/**
 * Process a single statement from the input buffer (up to the next colon or
 * line end), writing the tokenised form to the output buffer.
 *
 * \param **read	Pointer to the pointer to the input buffer.
 * \param **write	Pointer to the pointer to the output buffer.
 * \param *real_pos	Pointer to the variable holding the real line pos.
 * \param *options	The number of spaces used for a tab (0 to disable).
 * \param *assembler	Pointer to a variable indicating if we're in an assember block.
 * \param line_start	True if this is the first statement on a line.
 * \return		The status of the parsed statement.
 */

static enum parse_status parse_process_statement(char **read, char **write, int *real_pos, struct parse_options *options, bool *assembler, bool line_start)
{
	enum parse_status	status = PARSE_WHITESPACE;

	bool	statement_start = true;		/**< True while we're at the start of a statement.		*/
	bool	constant_due = false;		/**< True if a line number constant could be coming up.		*/
	bool	library_path_due = false;	/**< True if we're expecting a library path.			*/
	bool	clean_to_end = false;		/**< True if no non-whitespace has been found since set.	*/
	bool	in_whitespace = false;		/**< True if we've just output some whitespace.			*/

	int	extra_spaces = 0;		/**< Extra spaces taken up by expended keywords.		*/
	int	token = 0;			/**< Storage for any keyword tokens that we look up.		*/

	char	*start_pos = *write;		/**< A pointer to the start of the statement.			*/

	while (**read != '\n' && **read != ':') {
		if (!isspace(**read)) {
			/* If this isn't whitepspacce, then reset any flags that
			 * track the presence of whitespace.
			 */

			/* We're not currently in some whitespace. */

			if (in_whitespace)
				in_whitespace = false;

			/* The line can't be entirely whitespace. */

			if (status == PARSE_WHITESPACE)
				status = PARSE_COMPLETE;
		}

		/* Now start to work out what the next character might be. */

		if (*assembler == true) {
			/* Assembler is a special case. We just copy text across
			 * to the buffer until we find a closing delimiter.
			 */

			if (**read == ']')
				*assembler = false;

			*(*write)++ = *(*read)++;
		} else if (**read == '[') {
			/* This is the start of an assembler block. */
		
			*assembler = true;
			*(*write)++ = *(*read)++;

			statement_start = false;
			line_start = false;
			constant_due = false;
			library_path_due = false;
			clean_to_end = false;
		} else if (**read == '\"') {
			/* Copy strings as a lump. */
			if (!parse_process_string(read, write, (library_path_due == true) ? library_path : NULL))
				return PARSE_ERROR_OPEN_STRING;

			clean_to_end = false;

			if (library_path_due && *library_path != '\0' && options->link_libraries) {
				library_add_file(library_path);
				clean_to_end = true;
				status = PARSE_DELETED;
			}

			statement_start = false;
			line_start = false;
			constant_due = false;
			library_path_due = false;
		} else if (**read >= 'A' && **read <= 'Z' && (token = parse_match_token(read)) != -1) {
			/* Handle keywords */
			unsigned bytes;

			/* ELSE needs to be tokenised differently if it is at the
			 * start of a line. Oterwise what matters is if we're at
			 * the start of a statement or not, to handle the pseudo-
			 * variables.
			 */

			if (token == KWD_ELSE && line_start)
				bytes = parse_keywords[token].start;
			else if (token != KWD_ELSE && statement_start)
				bytes = parse_keywords[token].start;
			else
				bytes = parse_keywords[token].elsewhere;

			/* Insert the token value, and track the difference
			 * between that and the actual displayed text so that
			 * we can properly handle tabs.
			 */

			extra_spaces += strlen(parse_keywords[token].name) - 1;
			*(*write)++ = bytes & 0xff;
			if ((bytes & 0xff00) != 0) {
				*(*write)++ = (bytes & 0xff00) >> 8;
				extra_spaces--;
			}

			constant_due = false;
			library_path_due = false;
			clean_to_end = false;

			switch (token) {
			case KWD_ELSE:
			case KWD_GOSUB:
			case KWD_GOTO:
			case KWD_RESTORE:
			case KWD_THEN:
				constant_due = true;
				break;
			case KWD_FN:
			case KWD_PROC:
				parse_process_fnproc(read, write);
				break;
			case KWD_REM:
				if (options->crunch_rems) {
					status = PARSE_DELETED;
					clean_to_end = true;
				} else {
					status = PARSE_COMMENT;
				}
			case KWD_DATA:
				parse_process_to_line_end(read, write);
				break;
			case KWD_LIBRARY:
				library_path_due = true;
				break;
			}

			statement_start = false;
			line_start = false;
		} else if ((**read >= '0' && **read <= '9') && constant_due) {
			/* Handle binary line number constants. */
			parse_process_binary_constant(read, write, &extra_spaces);

			statement_start = false;
			line_start = false;
			library_path_due = false;
			clean_to_end = false;
		} else if ((**read >= 'a' && **read <= 'z') || (**read >= 'A' && **read <= 'Z')) {
			/* Handle variable names */
			parse_process_variable(read, write);

			statement_start = false;
			line_start = false;
			library_path_due = false;
			clean_to_end = false;
		} else if ((**read >= '0' && **read <= '9') || **read == '&' || **read == '%' || **read == '.') {
			/* Handle numeric constants. */
			parse_process_numeric_constant(read, write);
			
			statement_start = false;
			line_start = false;
			library_path_due = false;
			clean_to_end = false;
		} else if (**read == '*' && statement_start) {
			/* It's a star command, so run out to the end of the line. */

			parse_process_to_line_end(read, write);
			clean_to_end = false;
		} else {
			/* Handle everything else. */
			if (!isspace(**read)) {
				/* If this isn't whitespace, then it must affect the
				 * line status in some way.
				 */
			
				statement_start = false;
				line_start = false;
				library_path_due = false;
				clean_to_end = false;
			}

			/* Whitespace or commas are the only valid things separating
			 * keywords from following line number constants.
			 */

			if (!(isspace(**read) || **read == ','))
				constant_due = false;

			/* Output the character, if necessary. */

			if (**read == '\t' && options->tab_indent > 0 && !(options->crunch_whitespace || options->crunch_all_whitespace)) {
				/* If this is a tab, and we're expanding tabs, and
				 * we're not crunching whitespace, then output spaces
				 * until the next tab stop.
				 */

				int insert = options->tab_indent - ((*real_pos + (*write - start_pos) + extra_spaces) % options->tab_indent);
				if (insert == 0)
					insert = options->tab_indent;

				for (; insert > 0; insert--)
					*(*write)++ = ' ';
				
				(*read)++;

				in_whitespace = true;
			} else {
				/* If it's whitespace, and we're not stripping it,
				 * then output a space and note that we're in some
				 * whitespace.
				 *
				 * If it isn't whitespace, just copy it across.
				 */

				if (isspace(**read)) {
					if (!(options->crunch_all_whitespace || (options->crunch_whitespace && in_whitespace)))
						*(*write)++ = ' ';
					in_whitespace = true;
					(*read)++;
				} else {
					*(*write)++ = *(*read)++;
				}
			}
		}
	}

	/* If the statement is only whitespace, and we're removing empty statements,
	 * flag the statement to be deleted.
	 */

	if (status == PARSE_WHITESPACE && options->crunch_empty)
		status = PARSE_DELETED;

	/* Depending on whether we will be keeping the statement, either
	 * update the line pointer or rewind the write buffer.
	 */

	if (status == PARSE_DELETED) {
		*write = start_pos;
		if (!clean_to_end)
			printf("Error in removed statement\n");
	} else {
		*real_pos += (*write - start_pos) + extra_spaces;
	}

	return status;
}


/**
 * Test the contents of *buffer for a valid tokenisable keyword. If one is found,
 * return its keyword ID and advance *buffer to point to the character after the
 * end of the match.
 *
 * \param **buffer	Pointer to a pointer to the start of the text to match
 *			(updated on a successful match to point to the character
 *			after the matched text).
 * \return		The ID of any matching keyword, or -1 for none found.
 */

static int parse_match_token(char **buffer)
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

	/* Find the first entry in the keyword table that will match. If there isn't
	 * one, then report a fail immediately.
	 */

	keyword = parse_keyword_index[*start - 'A'];
	if (keyword == -1)
		return -1;

	/* Scan through the keyword table from the start point identified above
	 * until we find that we've alphabetically passed the text to be matched.
	 * For each keyword, run a scan to try and match it with the text either
	 * in full or in abbreviated form.
	 */

	do {
		char *test = start, *match = parse_keywords[keyword].name;

		/* Scan forward from the start of the buffer, comparing the
		 * characters until:
		 *
		 * a) the characters differ, or
		 * b) we reach the end of the keyword.
		 *
		 * At the end, the pointers will be pointing to the characters
		 * after the last pair to match.
		 *
		 * NB: We're assuming that the characters in the keyword list
		 * are all valid (ie. uppercase A to Z only, plus [ as the
		 * character after Z in ASCII). If not, there's a danger that
		 * the comparison will fail and we will run off the end of
		 * something.
		 */

		while (*test == *match && *match != '\0') {
			test++;
			match++;
		}

		/* Process the result. */

		if (*test == '.' && ((test - start) >= parse_keywords[keyword].abbrev)) {
			/* If we've hit a . in the string to be matched, then
			 * the characters before it must match the start of the
			 * keyword. If enough have passed to give us the minimum
			 * abbreviation, then match it.
			 *
			 * In the situation that a valid abbreviation matches a
			 * valid keyword (eg. OR. and OR), then the abbreviation
			 * will trump the keyword.
			 */

			result = *(match - 1) - *(test - 1);
			partial = keyword;
			partial_end = test;
		} else if (*match == '\0') {
			/* Otherwise, if we're at the end of the keyword, then
			 * this must be an exact match.
			 */

			result = 0;
			full = keyword;
			full_end = test;
		} else if (*test == '.') {
			/* Otherwise, if we've hit a . then this must have been
			 * a full abbreviated match which wasn't long enough.
			 * Therefore set result on the last pair of characters,
			 * so that we can carry on through the table.
			 */

			result = *(match - 1) - *(test - 1);
		} else  {
			/* Otherwise, we just failed. Set the result on the
			 * current pair of characters.
			 */

			result = *match - *test;
		}

		keyword++;
	} while (result <= 0);

	/* Return a result. If there's a full match, use this. If not, use a
	 * partial one if available. By definition (above), the full match must
	 * be longer than the partial one, and so correct.
	 */

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


/**
 * Process a string object, copying bytes from read to write until a valid
 * terminator is found or the end of the line is reached. The two pointers
 * are updated on return.
 *
 * \param **read	Pointer to the current read pointer.
 * \param **write	Pointer to the current write pointer.
 * \param *dump		Pointer to buffer to take string contents, or NULL.
 * \return		True on success; False on error.
 */

static bool parse_process_string(char **read, char **write, char *dump)
{
	bool string_closed = false;

	if (read == NULL || write == NULL || *read == NULL || *write == NULL)
		return false;

	*(*write)++ = *(*read)++;

	while (**read != '\n' && !string_closed) {
		if (**read == '\"' && (**read + 1) != '\"')
			string_closed = true;
		else if (**read == '\"' && (**read + 1) == '\"')
			*(*write)++ = *(*read)++;

		if (dump != NULL && !string_closed)
			*dump++ = **read;

		*(*write)++ = *(*read)++;
	}

	if (dump != NULL)
		*dump = '\0';

	if (!string_closed) {
		printf("Missing string terminator");
		return false;
	}
	
	return true;
}


/**
 * Process a numeric constant, copying bytes from read to write until a valid
 * terminator is found or the end of the line is reached. The two pointers
 * are updated on return.
 *
 * \param **read	Pointer to the current read pointer.
 * \param **write	Pointer to the current write pointer.
 */

static void parse_process_numeric_constant(char **read, char **write)
{
	switch (**read) {
	case '&':
		do {
			*(*write)++ = *(*read)++;
		} while ((**read >= '0' && **read <= '9') || (**read >= 'a' && **read <= 'f') || (**read >= 'A' && **read <= 'F'));
		break;
	case '%':
		do {
			*(*write)++ = *(*read)++;
		} while (**read == '0' || **read == '1');
		break;
	default:
		while (**read >= '0' && **read <= '9')
			*(*write)++ = *(*read)++;
		if (**read == '.')
			*(*write)++ = *(*read)++;
		while (**read >= '0' && **read <= '9')
			*(*write)++ = *(*read)++;
		if ((**read == 'e' || **read == 'E') && ((*(*read + 1) >= '0' && *(*read + 1) <= '9') || *(*read + 1) == '+' || *(*read + 1) == '-')) {
			*(*write)++ = *(*read)++;
			do {
				*(*write)++ = *(*read)++;
			} while (**read >= '0' && **read <= '9');
		}
		break;
	}
}


/**
 * Process a line number constant, taking bytes from read to write until a valid
 * terminator is found or the end of the line is reached. The number is then
 * converted into inline line number format, and the two pointers are updated on
 * return.
 *
 * \param **read	Pointer to the current read pointer.
 * \param **write	Pointer to the current write pointer.
 * \param *extra_spaces	Pointer to integer tracking the difference in line length
 *			between binary and displayed constant.
 */

static void parse_process_binary_constant(char **read, char **write, int *extra_spaces)
{
	char		number[256], *ptr;
	unsigned	line = 0;

	ptr = number;

	while (**read >= '0' && **read <= '9')
		*ptr++ = *(*read)++;
	*ptr = '\0';

	// \TODO -- This should error if not able to fit into two bytes.

	line = atoi(number) & 0xffff;

	*(*write)++ = 0x8d;
	*(*write)++ = (((line & 0xc0) >> 2) | ((line & 0xc000) >> 12)) ^ 0x54;
	*(*write)++ = (line & 0x3f) | 0x40;
	*(*write)++ = ((line & 0x3f00) >> 8) | 0x40;

	*extra_spaces += strlen(number) - 4;
}


/**
 * Process an FN or PROC name, copying bytes from read to write until a valid
 * terminator is found or the end of the line is reached. The two pointers
 * are updated on return.
 *
 * \param **read	Pointer to the current read pointer.
 * \param **write	Pointer to the current write pointer.
 */

static void parse_process_fnproc(char **read, char **write)
{
	while ((**read >= 'a' && **read <= 'z') || (**read >= 'A' && **read <= 'Z') || (**read >= '0' && **read <= '9') || **read == '_' || **read == '`')
		*(*write)++ = *(*read)++;
}


/**
 * Process a variable name, copying bytes from read to write until a valid
 * terminator is found or the end of the line is reached. The two pointers
 * are updated on return.
 *
 * \param **read	Pointer to the current read pointer.
 * \param **write	Pointer to the current write pointer.
 */

static void parse_process_variable(char **read, char **write)
{
	while ((**read >= 'a' && **read <= 'z') || (**read >= 'A' && **read <= 'Z') || (**read >= '0' && **read <= '9') || **read == '_' || **read == '`')
		*(*write)++ = *(*read)++;
	if (**read == '%' || **read == '$')
		*(*write)++ = *(*read)++;
}


/**
 * Process a "run to end" object, such as REM or *, copying bytes from read to
 * write until the end of the line is reached. The two pointers are updated on
 * return.
 *
 * \param **read	Pointer to the current read pointer.
 * \param **write	Pointer to the current write pointer.
 */

static void parse_process_to_line_end(char **read, char **write)
{
	while (**read != '\n' && **read != '\r' && **read != '\0')
		*(*write)++ = *(*read)++;
}
