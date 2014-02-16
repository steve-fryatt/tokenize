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

#define TOKEN_CONST 0x8d

/**
 * List of keyword array indexes. This must match the entries in the
 * parse_keywords[] array defined further down the file.
 */

enum keyword {
	KWD_NO_MATCH = -1,	/**< Indicates that no keyword macth is available.		*/
	KWD_ABS = 0,		/**< Keywords start at array index 0 and go alphabetically.	*/
	KWD_ACS,
	KWD_ADVAL,
	KWD_AND,
	KWD_APPEND,
	KWD_ASC,
	KWD_ASN,
	KWD_ATN,
	KWD_AUTO,
	KWD_BEAT,
	KWD_BEATS,
	KWD_BGET,
	KWD_BPUT,
	KWD_CALL,
	KWD_CASE,
	KWD_CHAIN,
	KWD_CHR$,
	KWD_CIRCLE,
	KWD_CLEAR,
	KWD_CLG,
	KWD_CLOSE,
	KWD_CLS,
	KWD_COLOR,
	KWD_COLOUR,
	KWD_COS,
	KWD_COUNT,
	KWD_CRUNCH,
	KWD_DATA,
	KWD_DEF,
	KWD_DEG,
	KWD_DELETE,
	KWD_DIM,
	KWD_DIV,
	KWD_DRAW,
	KWD_EDIT,
	KWD_ELLIPSE,
	KWD_ELSE,
	KWD_END,
	KWD_ENDCASE,
	KWD_ENDIF,
	KWD_ENDPROC,
	KWD_ENDWHILE,
	KWD_ENVELOPE,
	KWD_EOF,
	KWD_EOR,
	KWD_ERL,
	KWD_ERR,
	KWD_ERROR,
	KWD_EVAL,
	KWD_EXP,
	KWD_EXT,
	KWD_FALSE,
	KWD_FILL,
	KWD_FN,
	KWD_FOR,
	KWD_GCOL,
	KWD_GET,
	KWD_GET$,
	KWD_GOSUB,
	KWD_GOTO,
	KWD_HELP,
	KWD_HIMEM,
	KWD_IF,
	KWD_INKEY,
	KWD_INKEY$,
	KWD_INPUT,
	KWD_INSTALL,
	KWD_INSTR,
	KWD_INT,
	KWD_LEFT$,
	KWD_LEN,
	KWD_LET,
	KWD_LIBRARY,
	KWD_LINE,
	KWD_LIST,
	KWD_LN,
	KWD_LOAD,
	KWD_LOCAL,
	KWD_LOG,
	KWD_LOMEM,
	KWD_LVAR,
	KWD_MID$,
	KWD_MOD,
	KWD_MODE,
	KWD_MOUSE,
	KWD_MOVE,
	KWD_NEW,
	KWD_NEXT,
	KWD_NOT,
	KWD_OF,
	KWD_OFF,
	KWD_OLD,
	KWD_ON,
	KWD_OPENIN,
	KWD_OPENOUT,
	KWD_OPENUP,
	KWD_OR,
	KWD_ORIGIN,
	KWD_OSCLI,
	KWD_OTHERWISE,
	KWD_OVERLAY,
	KWD_PAGE,
	KWD_PI,
	KWD_PLOT,
	KWD_POINT,
	KWD_POINT2,
	KWD_POS,
	KWD_PRINT,
	KWD_PROC,
	KWD_PTR,
	KWD_QUIT,
	KWD_RAD,
	KWD_READ,
	KWD_RECTANGLE,
	KWD_REM,
	KWD_RENUMBER,
	KWD_REPEAT,
	KWD_REPORT,
	KWD_RESTORE,
	KWD_RETURN,
	KWD_RIGHT$,
	KWD_RND,
	KWD_RUN,
	KWD_SAVE,
	KWD_SGN,
	KWD_SIN,
	KWD_SOUND,
	KWD_SPC,
	KWD_SQR,
	KWD_STEP,
	KWD_STEREO,
	KWD_STOP,
	KWD_STR$,
	KWD_STRING$,
	KWD_SUM,
	KWD_SWAP,
	KWD_SYS,
	KWD_TAB,
	KWD_TAN,
	KWD_TEMPO,
	KWD_TEXTLOAD,
	KWD_TEXTSAVE,
	KWD_THEN,
	KWD_TIME,
	KWD_TINT,
	KWD_TO,
	KWD_TRACE,
	KWD_TRUE,
	KWD_TWIN,
	KWD_TWINO,
	KWD_UNTIL,
	KWD_USR,
	KWD_VAL,
	KWD_VDU,
	KWD_VOICE,
	KWD_VOICES,
	KWD_VPOS,
	KWD_WAIT,
	KWD_WHEN,
	KWD_WHILE,
	KWD_WIDTH,
	MAX_KEYWORDS		/**< The maximum number of keywords available.			*/
};


/**
 * Individual keyword definition.
 */

struct keyword_definition {
	char		*name;		/**< The name of the keyword.				*/
	int		abbrev;		/**< The minimum number of characters allowed.		*/
	unsigned	start;		/**< The token if at the start of a statement.		*/
	unsigned	elsewhere;	/**< The token if elsewhere in a statement.		*/
};


/**
 * The table of known keywords and their tokens.  The keywords *must* be in
 * alphabetical order, to allow the search routine to operate, and this should
 * also match the entries in enum keyword.
 */

static struct keyword_definition parse_keywords[] = {
	{"ABS",		3,	0x94,	0x94	},
	{"ACS",		3,	0x95,	0x95	},
	{"ADVAL",	2,	0x96,	0x96	},
	{"AND",		1,	0x80,	0x80	},
	{"APPEND",	2,	0x8ec7,	0x8ec7	},
	{"ASC",		3,	0x97,	0x97	},
	{"ASN",		3,	0x98,	0x98	},
	{"ATN",		3,	0x99,	0x99	},
	{"AUTO",	2,	0x8fc7,	0x8fc7	},
	{"BEAT",	4,	0x8fc6,	0x8fc6	},
	{"BEATS",	3,	0x9ec8,	0x9ec8	},
	{"BGET",	1,	0x9a,	0x9a	},
	{"BPUT",	2,	0xd5,	0xd5	},
	{"CALL",	2,	0xd6,	0xd6	},
	{"CASE",	4,	0x8ec8,	0x8ec8	},
	{"CHAIN",	2,	0xd7,	0xd7	},
	{"CHR$",	4,	0xbd,	0xbd	},
	{"CIRCLE",	2,	0x8fc8,	0x8fc8	},
	{"CLEAR",	2,	0xd8,	0xd8	},
	{"CLG",		3,	0xda,	0xda	},
	{"CLOSE",	3,	0xd9,	0xd9	},
	{"CLS",		3,	0xdb,	0xdb	},
	{"COLOR",	1,	0xfb,	0xfb	},
	{"COLOUR",	1,	0xfb,	0xfb	},
	{"COS",		3,	0x9b,	0x9b	},
	{"COUNT",	3,	0x9c,	0x96	},
	{"CRUNCH",	2,	0x90c7,	0x90c7	},
	{"DATA",	1,	0xdc,	0xdc	},
	{"DEF",		3,	0xdd,	0xdd	},
	{"DEG",		3,	0x9d,	0x9d	},
	{"DELETE",	3,	0x91c7,	0x91c7	},
	{"DIM",		3,	0xde,	0xde	},
	{"DIV",		3,	0x81,	0x81	},
	{"DRAW",	2,	0xdf,	0xdf	},
	{"EDIT",	2,	0x92c7,	0x92c7	},
	{"ELLIPSE",	3,	0x9dc8,	0x9dc8	},
	{"ELSE",	2,	0xcc,	0x8b	},
	{"END",		3,	0xe0,	0xe0	},
	{"ENDCASE",	4,	0xcb,	0xcb	},
	{"ENDIF",	5,	0xcd,	0xcd	},
	{"ENDPROC",	1,	0xe1,	0xe1	},
	{"ENDWHILE",	4,	0xce,	0xce	},
	{"ENVELOPE",	3,	0xe2,	0xe2	},
	{"EOF",		3,	0xc5,	0xc5	},
	{"EOR",		3,	0x82,	0x82	},
	{"ERL",		3,	0x9e,	0x9e	},
	{"ERR",		3,	0x9f,	0x9f	},
	{"ERROR",	3,	0x85,	0x85	},
	{"EVAL",	2,	0xa0,	0xa0	},
	{"EXP",		3,	0xa1,	0xa1	},
	{"EXT",		3,	0xa2,	0xa2	},
	{"FALSE",	2,	0xa3,	0xa3	},
	{"FILL",	2,	0x90c8,	0x90c8	},
	{"FN",		2,	0xa4,	0xa4	},
	{"FOR",		1,	0xe3,	0xe3	},
	{"GCOL",	2,	0xe6,	0xe6	},
	{"GET",		3,	0xa5,	0xa5	},
	{"GET$",	2,	0xbe,	0xbe	},
	{"GOSUB",	3,	0xe4,	0xe4	},
	{"GOTO",	1,	0xe5,	0xe5	},
	{"HELP",	2,	0x93c7,	0x93c7	},
	{"HIMEM",	1,	0xd3,	0x93	},
	{"IF",		2,	0xe7,	0xe7	},
	{"INKEY",	5,	0xa6,	0xa6	},
	{"INKEY$",	3,	0xbf,	0xbf	},
	{"INPUT",	1,	0xe8,	0xe8	},
	{"INSTALL",	5,	0x9ac8,	0x9ac8	},
	{"INSTR(",	3,	0xa7,	0xa7	},
	{"INT",		3,	0xa8,	0xa8	},
	{"LEFT$(",	2,	0xc0,	0xc0	},
	{"LEN",		3,	0xa9,	0xa9	},
	{"LET",		3,	0xe9,	0xe9	},
	{"LIBRARY",	3,	0x9bc8,	0x9bc8	},
	{"LINE",	4,	0x86,	0x86	},
	{"LIST",	1,	0x94c7,	0x94c7	},
	{"LN",		2,	0xaa,	0xaa	},
	{"LOAD",	2,	0x95c7,	0x95c7	},
	{"LOCAL",	3,	0xea,	0xea	},
	{"LOG",		3,	0xab,	0xab	},
	{"LOMEM",	3,	0xd2,	0x92	},
	{"LVAR",	2,	0x96c7,	0x96c7	},
	{"MID$(",	1,	0xc1,	0xc1	},
	{"MOD",		3,	0x83,	0x83	},
	{"MODE",	2,	0xeb,	0xeb	},
	{"MOUSE",	3,	0x97c8,	0x97c8	},
	{"MOVE",	4,	0xec,	0xec	},
	{"NEW",		3,	0x97c7,	0x97c7	},
	{"NEXT",	1,	0xed,	0xed	},
	{"NOT",		3,	0xac,	0xac	},
	{"OF",		2,	0xca,	0xca	},
	{"OFF",		3,	0x87,	0x87	},
	{"OLD",		1,	0x98c7,	0x98c7	},
	{"ON",		2,	0xee,	0xee	},
	{"OPENIN",	2,	0x8e,	0x8e	},
	{"OPENOUT",	5,	0xae,	0xae	},
	{"OPENUP",	6,	0xad,	0xad	},
	{"OR",		2,	0x84,	0x84	},
	{"ORIGIN",	2,	0x91c8,	0x91c8	},
	{"OSCLI",	2,	0xff,	0xff	},
	{"OTHERWISE",	2,	0x7f,	0x7f	},
	{"OVERLAY",	2,	0xa3c8,	0xa3c8	},
	{"PAGE",	2,	0xd0,	0x90	},
	{"PI",		2,	0xaf,	0xaf	},
	{"PLOT",	2,	0xf0,	0xf0	},
	{"POINT",	5,	0x92c8,	0x92c8	},
	{"POINT(",	2,	0xb0,	0xb0	},
	{"POS",		3,	0xb1,	0xb1	},
	{"PRINT",	1,	0xf1,	0xf1	},
	{"PROC",	4,	0xf2,	0xf2	},
	{"PTR",		3,	0xcf,	0x8f	},
	{"QUIT",	1,	0x98c8,	0x98c8	},
	{"RAD",		3,	0xb2,	0xb2	},
	{"READ",	4,	0xf3,	0xf3	},
	{"RECTANGLE",	3,	0x93c8,	0x93c8	},
	{"REM",		3,	0xf4,	0xf4	},
	{"RENUMBER",	3,	0x99c7,	0x99c7	},
	{"REPEAT",	3,	0xf5,	0xf5	},
	{"REPORT",	4,	0xf6,	0xf6	},
	{"RESTORE",	3,	0xf7,	0xf7	},
	{"RETURN",	1,	0xf8,	0xf8	},
	{"RIGHT$(",	2,	0xc2,	0xc2	},
	{"RND",		3,	0xb3,	0xb3	},
	{"RUN",		3,	0xf9,	0xf9	},
	{"SAVE",	2,	0x9ac7,	0x9ac7	},
	{"SGN",		3,	0xb4,	0xb4	},
	{"SIN",		3,	0xb5,	0xb5	},
	{"SOUND",	2,	0xd4,	0xd4	},
	{"SPC",		3,	0x89,	0x89	},
	{"SQR",		3,	0xb6,	0xb6	},
	{"STEP",	1,	0x88,	0x88	},
	{"STEREO",	4,	0xa2c8,	0xa2c8	},
	{"STOP",	4,	0xfa,	0xfa	},
	{"STR$",	4,	0xc3,	0xc3	},
	{"STRING$(",	4,	0xc4,	0xc4	},
	{"SUM",		3,	0x8ec6,	0x8ec6	},
	{"SWAP",	2,	0x94c8,	0x94c8	},
	{"SYS",		3,	0x99c8,	0x99c8	},
	{"TAB(",	4,	0x8a,	0x8a	},
	{"TAN",		1,	0xb7,	0xb7	},
	{"TEMPO",	2,	0x9fc8,	0x9fc8	},
	{"TEXTLOAD",	5,	0x9bc7,	0x9bc7	},
	{"TEXTSAVE",	5,	0x9cc7,	0x9cc7	},
	{"THEN",	2,	0x8c,	0x8c	},
	{"TIME",	2,	0xd1,	0x91	},
	{"TINT",	4,	0x9cc8,	0x9cc8	},
	{"TO",		2,	0xb8,	0xb8	},
	{"TRACE",	2,	0xfc,	0xfc	},
	{"TRUE",	4,	0xb9,	0xb9	},
	{"TWIN",	4,	0x9dc7,	0x9dc7	},
	{"TWINO",	2,	0x9ec7,	0x9ec7	},
	{"UNTIL",	1,	0xfd,	0xfd	},
	{"USR",		3,	0xba,	0xba	},
	{"VAL",		3,	0xbb,	0xbb	},
	{"VDU",		1,	0xef,	0xef	},
	{"VOICE",	5,	0xa1c8,	0xa1c8	},
	{"VOICES",	2,	0xa0c8,	0xa0c8	},
	{"VPOS",	2,	0xbc,	0xbc	},
	{"WAIT",	2,	0x96c8,	0x96c8	},
	{"WHEN",	4,	0xc9,	0xc9	},
	{"WHILE",	1,	0x95c8,	0x95c8	},
	{"WIDTH",	2,	0xfe,	0xfe	},
	{"[",		1,	0x00,	0x00	}	/* TERM	*/
};

/**
 * Indexes into the keywords table for the various initial letters.
 * KWD_NO_MATCH indicates that there are no keywords starting with that letter.
 */

static enum keyword parse_keyword_index[] = {
	KWD_ABS,	/**< A	*/
	KWD_BEAT,	/**< B	*/
	KWD_CALL,	/**< C	*/
	KWD_DATA,	/**< D	*/
	KWD_EDIT,	/**< E	*/
	KWD_FALSE,	/**< F	*/
	KWD_GCOL,	/**< G	*/
	KWD_HELP,	/**< H	*/
	KWD_IF,		/**< I	*/
	KWD_NO_MATCH,	/**< J	*/
	KWD_NO_MATCH,	/**< K	*/
	KWD_LEFT$,	/**< L	*/
	KWD_MID$,	/**< M	*/
	KWD_NEW,	/**< N	*/
	KWD_OF,		/**< O	*/
	KWD_PAGE,	/**< P	*/
	KWD_QUIT,	/**< Q	*/
	KWD_RAD,	/**< R	*/
	KWD_SAVE,	/**< S	*/
	KWD_TAB,	/**< T	*/
	KWD_UNTIL,	/**< U	*/
	KWD_VAL,	/**< V	*/
	KWD_WAIT,	/**< W	*/
	KWD_NO_MATCH,	/**< X	*/
	KWD_NO_MATCH,	/**< Y	*/
	KWD_NO_MATCH	/**< Z	*/
};

static char parse_buffer[MAX_TOKENISED_LINE];
static char library_path[MAX_TOKENISED_LINE];


static enum parse_status parse_process_statement(char **read, char **write, int *real_pos, struct parse_options *options, bool *assembler, bool line_start, char *location);
static enum keyword parse_match_token(char **buffer);
static bool parse_process_string(char **read, char **write, char *dump);
static void parse_process_numeric_constant(char **read, char **write);
static bool parse_process_binary_constant(char **read, char **write, int *extra_spaces);
static void parse_process_fnproc(char **read, char **write);
static void parse_process_variable(char **read, char **write);
static void parse_process_whitespace(char **read, char **write, char *start_pos, int extra_spaces, struct parse_options *options);
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
 * \param *location	Pointer to source file location information.
 * \return		Pointer to the tokenised line, or NULL on error.
 */

char *parse_process_line(char *line, struct parse_options *options, bool *assembler, unsigned *line_number, char *location)
{
	char			*read = line, *write = parse_buffer;
	unsigned		read_number = 0;
	int			leading_spaces = 0;
	enum parse_status	status = PARSE_COMPLETE;

	bool	line_start = true;		/**< True while we're at the start of a line.				*/
	int	real_pos = 0;			/**< The real position in the line, including expanded  keywords.	*/
	bool	all_deleted = true;		/**< True while all the statements on the line have been deleted.	*/

	if (location == NULL)
		return NULL;

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

		if (read_number > 0xffff) {
			fprintf(stderr, "Error: Line number %u too large at%s\n", read_number, location);
			return NULL;
		} else if (read_number > *line_number) {
			*line_number = read_number;
		} else if (read_number <= *line_number) {
			fprintf(stderr, "Error: Line number %u out of sequence at%s\n", read_number, location);
			return NULL;
		}
	} else {
		*line_number += options->line_increment;
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

	if (!options->crunch_indent) {
		real_pos = leading_spaces;

		for (; leading_spaces > 0; leading_spaces--)
			*write++ = ' ';
	}

	/* Process statements from the line, sending them to the output buffer. */

	while (*read != '\n') {
		status = parse_process_statement(&read, &write, &real_pos, options, assembler, line_start, location);

		if (status == PARSE_DELETED) {
			/* If the statement was deleted, remove any following separator. */

			if (*read == ':')
				read++;
		} else if (status == PARSE_WHITESPACE || status == PARSE_COMMENT || status == PARSE_COMPLETE) {
			/* If the statement was parsed OK, process it. */

			/* It's no longer true that all the statements on the line have
			 * been deleted, because this one hansn't.
			 */

			if (all_deleted == true)
				all_deleted = false;

			/* If there's a separator following, copy it into the
			 * output buffer unless we're removing empty statements
			 * and this is the end of the line. Any trailing colons
			 * with spaces after them will be deleted when the spaces
			 * get crunched out as an empty statement -- if there are
			 * no spaces, this won't happen.
			 */

			if (*read == ':' && (!options->crunch_empty || *(read + 1) != '\n')) {
				*write++ = *read++;
				real_pos++;
			}

			/* If this isn't a comment, and we're due to remove all of the
			 * comments after the first block, then update the crunch
			 * flags to make it so.
			 */

			if (options->crunch_body_rems == true && options->crunch_rems == false && status != PARSE_COMMENT)
				options->crunch_rems = true;
		} else {
			/* The statement tokeniser returned an error, so report
			 * it with a suitable message and quit.
			 */

			char	*error;

			switch (status) {
			case PARSE_ERROR_OPEN_STRING:
				error = "Unterminated string";
				break;
			case PARSE_ERROR_DELETED_STATEMENT:
				error = "Misformed deleted statement";
				break;
			case PARSE_ERROR_LINE_CONSTANT:
				error = "Invalid line number constant";
				break;
			default:
				error = "Unknown error";
				break;
			}

			fprintf(stderr, "Error: %s at%s\n", error, location);
			return NULL;
		}

		line_start = false;
	}

	/* If the last statement was deleted, clean back to remove any trailing
	 * whitespace and colons that could have been orphaned.
	 */

	if (all_deleted == false && status == PARSE_DELETED) {
		while (((write - parse_buffer) > 4) && (*(write - 1) == ' ' || *(write - 1) == ':'))
			write--;
	}

	/* If all the statements in the line were deleted, then it doesn't want
	 * to be written. We report this back by setting the first byte of the
	 * buffer to zero: since this is defined to always be \r, it clearly
	 * marks the line as valid but empty.
	 *
	 * Otherwise, write the line length and terminate the output buffer. */

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
 * \param *location	Pointer to a string giving the file location.
 * \return		The status of the parsed statement.
 */

static enum parse_status parse_process_statement(char **read, char **write, int *real_pos, struct parse_options *options, bool *assembler, bool line_start, char *location)
{
	enum parse_status	status = PARSE_WHITESPACE;

	bool		statement_start = true;		/**< True while we're at the start of a statement.		*/
	bool		constant_due = false;		/**< True if a line number constant could be coming up.		*/
	bool		library_path_due = false;	/**< True if we're expecting a library path.			*/
	bool		clean_to_end = false;		/**< True if no non-whitespace has been found since set.	*/

	int		extra_spaces = 0;		/**< Extra spaces taken up by expended keywords.		*/
	enum keyword	token = KWD_NO_MATCH;		/**< Storage for any keyword tokens that we look up.		*/

	char		*start_pos = *write;		/**< A pointer to the start of the statement.			*/

	while (**read != '\n' && **read != ':') {
		/* If the character isn't whitespace, then the line can't be
		 * entirely whitespace.
		 */

		if (status == PARSE_WHITESPACE && !isspace(**read))
			status = PARSE_COMPLETE;

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
				if (options->verbose_output)
					printf("Queue 'LIBRARY \"%s\"' for linking%s\n", library_path, location);
			}

			statement_start = false;
			line_start = false;
			constant_due = false;
			library_path_due = false;
		} else if (**read >= 'A' && **read <= 'Z' && (token = parse_match_token(read)) != KWD_NO_MATCH) {
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
				if (statement_start)
					library_path_due = true;
				else
					fprintf(stderr, "Warning: Unisolated LIBRARY not linked%s\n", location);
				break;
			default:
				break;
			}

			statement_start = false;
			line_start = false;
		} else if ((**read >= '0' && **read <= '9') && constant_due) {
			/* Handle binary line number constants. */
			if (!parse_process_binary_constant(read, write, &extra_spaces))
				return PARSE_ERROR_LINE_CONSTANT;

			statement_start = false;
			line_start = false;
			library_path_due = false;
			clean_to_end = false;
		} else if ((**read >= 'a' && **read <= 'z') || (**read >= 'A' && **read <= 'Z')) {
			/* Handle variable names */
			parse_process_variable(read, write);

			if (library_path_due && options->link_libraries)
				fprintf(stderr, "Warning: Variable LIBRARY not linked%s\n", location);

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
		} else if (isspace(**read)) {
			/* Handle whitespace. */

			parse_process_whitespace(read, write, start_pos, *real_pos + extra_spaces, options);
		} else {
			/* Handle eveything else. */

			statement_start = false;
			line_start = false;
			library_path_due = false;
			clean_to_end = false;

			/* Whitespace or commas are the only valid things separating
			 * keywords from following line number constants, and whitespace
			 * doesn't end up in this section.
			 */

			if (!(**read == ','))
				constant_due = false;

			/* Copy the character to the output. */

			*(*write)++ = *(*read)++;
		}
	}

	/* If the statement is only whitespace, and we're removing empty statements,
	 * flag the statement to be deleted.
	 */

	if (status == PARSE_WHITESPACE && options->crunch_empty) {
		clean_to_end = true;
		status = PARSE_DELETED;
	}

	/* Depending on whether we will be keeping the statement, either
	 * update the line pointer or rewind the write buffer.
	 */

	if (status == PARSE_DELETED) {
		*write = start_pos;
		if (!clean_to_end)
			status = PARSE_ERROR_DELETED_STATEMENT;
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

static enum keyword parse_match_token(char **buffer)
{
	char		*start = *buffer;
	enum keyword	keyword = KWD_NO_MATCH;
	int		result = 0;
	enum keyword	full = KWD_NO_MATCH;
	char		*full_end = NULL;
	enum keyword	partial = -1;
	char		*partial_end = NULL;

	/* If the code doesn't start with an upper case letter, it's not a keyword */

	if (buffer == NULL || *start < 'A' || *start > 'Z')
		return KWD_NO_MATCH;

	/* Find the first entry in the keyword table that will match. If there isn't
	 * one, then report a fail immediately.
	 */

	keyword = parse_keyword_index[*start - 'A'];
	if (keyword == KWD_NO_MATCH)
		return KWD_NO_MATCH;

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
	} while (result <= 0 && keyword < MAX_KEYWORDS);

	/* Return a result. If there's a full match, use this. If not, use a
	 * partial one if available. By definition (above), the full match must
	 * be longer than the partial one, and so correct.
	 */

	if (full != KWD_NO_MATCH) {
		*buffer = full_end;
		return full;
	}

	if (partial != KWD_NO_MATCH) {
		*buffer = partial_end;
		return partial;
	}

	return KWD_NO_MATCH;
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

	if (!string_closed)
		return false;

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

static bool parse_process_binary_constant(char **read, char **write, int *extra_spaces)
{
	char		number[256], *ptr;
	unsigned	line = 0;

	ptr = number;

	while (**read >= '0' && **read <= '9')
		*ptr++ = *(*read)++;
	*ptr = '\0';

	line = atoi(number);
	if (line > 0xffff)
		return false;

	*(*write)++ = TOKEN_CONST;
	*(*write)++ = (((line & 0xc0) >> 2) | ((line & 0xc000) >> 12)) ^ 0x54;
	*(*write)++ = (line & 0x3f) | 0x40;
	*(*write)++ = ((line & 0x3f00) >> 8) | 0x40;

	*extra_spaces += strlen(number) - 4;

	return true;
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
 * Process white space in a line, either expanding tabs, reducing it to a single
 * space or removing it completely depending on the configured options.
 *
 * \param **read	Pointer to the current read pointer.
 * \param **write	Pointer to the current write pointer.
 * \param *start_pos	Pointer to the start of the write buffer.
 * \param extra_spaces	The number of extra spaces taken up by token expansion.
 * \param *options	Pointer to the current options block.
 */

static void parse_process_whitespace(char **read, char **write, char *start_pos, int extra_spaces, struct parse_options *options)
{
	bool	first_space = true;
	int	insert;

	while (isspace(**read) && **read != '\n') {
		if (!(options->crunch_all_whitespace || (options->crunch_whitespace && !first_space))) {
			if (**read == '\t' && !options->crunch_whitespace) {
				insert = options->tab_indent - (((*write - start_pos) + extra_spaces) % options->tab_indent);
				if (insert == 0)
					insert = options->tab_indent;

				for (; insert > 0; insert--)
					*(*write)++ = ' ';
			} else if (!options->crunch_whitespace || first_space) {
				*(*write)++ = ' ';
			}
		}

		(*read)++;

		first_space = false;
	}
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
