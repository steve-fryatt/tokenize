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

enum parse_keyword {
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
	KWD_CHR_D,
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
	KWD_GET_D,
	KWD_GOSUB,
	KWD_GOTO,
	KWD_HELP,
	KWD_HIMEM,
	KWD_IF,
	KWD_INKEY,
	KWD_INKEY_D,
	KWD_INPUT,
	KWD_INSTALL,
	KWD_INSTR,
	KWD_INT,
	KWD_LEFT_D,
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
	KWD_MID_D,
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
	KWD_RIGHT_D,
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
	KWD_STR_D,
	KWD_STRING_D,
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

struct parse_keyword_definition {
	char		*name;		/**< The name of the keyword.				*/
	int		abbrev;		/**< The minimum number of characters allowed.		*/
	unsigned	start;		/**< The token if at the start of a statement.		*/
	unsigned	elsewhere;	/**< The token if elsewhere in a statement.		*/
	bool		var_start;	/**< True if the keyword can start a variable name.	*/
};


/**
 * The table of known keywords and their tokens.  The keywords *must* be in
 * alphabetical order, to allow the search routine to operate, and this should
 * also match the entries in enum parse_keyword.
 */

static struct parse_keyword_definition parse_keywords[] = {
	{"ABS",		3,	0x94,	0x94,	false	},
	{"ACS",		3,	0x95,	0x95,	false	},
	{"ADVAL",	2,	0x96,	0x96,	false	},
	{"AND",		1,	0x80,	0x80,	false	},
	{"APPEND",	2,	0x8ec7,	0x8ec7,	false	},
	{"ASC",		3,	0x97,	0x97,	false	},
	{"ASN",		3,	0x98,	0x98,	false	},
	{"ATN",		3,	0x99,	0x99,	false	},
	{"AUTO",	2,	0x8fc7,	0x8fc7,	false	},
	{"BEAT",	4,	0x8fc6,	0x8fc6,	false	},
	{"BEATS",	3,	0x9ec8,	0x9ec8,	false	},
	{"BGET",	1,	0x9a,	0x9a,	true	},
	{"BPUT",	2,	0xd5,	0xd5,	true	},
	{"CALL",	2,	0xd6,	0xd6,	false	},
	{"CASE",	4,	0x8ec8,	0x8ec8,	false	},
	{"CHAIN",	2,	0xd7,	0xd7,	false	},
	{"CHR$",	4,	0xbd,	0xbd,	false	},
	{"CIRCLE",	2,	0x8fc8,	0x8fc8,	false	},
	{"CLEAR",	2,	0xd8,	0xd8,	true	},
	{"CLG",		3,	0xda,	0xda,	true	},
	{"CLOSE",	3,	0xd9,	0xd9,	true	},
	{"CLS",		3,	0xdb,	0xdb,	true	},
	{"COLOR",	1,	0xfb,	0xfb,	false	},
	{"COLOUR",	1,	0xfb,	0xfb,	false	},
	{"COS",		3,	0x9b,	0x9b,	false	},
	{"COUNT",	3,	0x9c,	0x96,	true	},
	{"CRUNCH",	2,	0x90c7,	0x90c7,	false	},
	{"DATA",	1,	0xdc,	0xdc,	false	},
	{"DEF",		3,	0xdd,	0xdd,	false	},
	{"DEG",		3,	0x9d,	0x9d,	false	},
	{"DELETE",	3,	0x91c7,	0x91c7,	false	},
	{"DIM",		3,	0xde,	0xde,	false	},
	{"DIV",		3,	0x81,	0x81,	false	},
	{"DRAW",	2,	0xdf,	0xdf,	false	},
	{"EDIT",	2,	0x92c7,	0x92c7,	false	},
	{"ELLIPSE",	3,	0x9dc8,	0x9dc8,	false	},
	{"ELSE",	2,	0xcc,	0x8b,	false	},
	{"END",		3,	0xe0,	0xe0,	true	},
	{"ENDCASE",	4,	0xcb,	0xcb,	true	},
	{"ENDIF",	5,	0xcd,	0xcd,	true	},
	{"ENDPROC",	1,	0xe1,	0xe1,	true	},
	{"ENDWHILE",	4,	0xce,	0xce,	true	},
	{"ENVELOPE",	3,	0xe2,	0xe2,	false	},
	{"EOF",		3,	0xc5,	0xc5,	true	},
	{"EOR",		3,	0x82,	0x82,	false	},
	{"ERL",		3,	0x9e,	0x9e,	true	},
	{"ERR",		3,	0x9f,	0x9f,	true	},
	{"ERROR",	3,	0x85,	0x85,	false	},
	{"EVAL",	2,	0xa0,	0xa0,	false	},
	{"EXP",		3,	0xa1,	0xa1,	false	},
	{"EXT",		3,	0xa2,	0xa2,	true	},
	{"FALSE",	2,	0xa3,	0xa3,	true	},
	{"FILL",	2,	0x90c8,	0x90c8,	false	},
	{"FN",		2,	0xa4,	0xa4,	false	},
	{"FOR",		1,	0xe3,	0xe3,	false	},
	{"GCOL",	2,	0xe6,	0xe6,	false	},
	{"GET",		3,	0xa5,	0xa5,	false	},
	{"GET$",	2,	0xbe,	0xbe,	false	},
	{"GOSUB",	3,	0xe4,	0xe4,	false	},
	{"GOTO",	1,	0xe5,	0xe5,	false	},
	{"HELP",	2,	0x93c7,	0x93c7,	true	},
	{"HIMEM",	1,	0xd3,	0x93,	true	},
	{"IF",		2,	0xe7,	0xe7,	false	},
	{"INKEY",	5,	0xa6,	0xa6,	false	},
	{"INKEY$",	3,	0xbf,	0xbf,	false	},
	{"INPUT",	1,	0xe8,	0xe8,	false	},
	{"INSTALL",	5,	0x9ac8,	0x9ac8,	false	},
	{"INSTR(",	3,	0xa7,	0xa7,	false	},
	{"INT",		3,	0xa8,	0xa8,	false	},
	{"LEFT$(",	2,	0xc0,	0xc0,	false	},
	{"LEN",		3,	0xa9,	0xa9,	false	},
	{"LET",		3,	0xe9,	0xe9,	false	},
	{"LIBRARY",	3,	0x9bc8,	0x9bc8,	false	},
	{"LINE",	4,	0x86,	0x86,	false	},
	{"LIST",	1,	0x94c7,	0x94c7,	false	},
	{"LN",		2,	0xaa,	0xaa,	false	},
	{"LOAD",	2,	0x95c7,	0x95c7,	false	},
	{"LOCAL",	3,	0xea,	0xea,	false	},
	{"LOG",		3,	0xab,	0xab,	false	},
	{"LOMEM",	3,	0xd2,	0x92,	true	},
	{"LVAR",	2,	0x96c7,	0x96c7,	true	},
	{"MID$(",	1,	0xc1,	0xc1,	false	},
	{"MOD",		3,	0x83,	0x83,	false	},
	{"MODE",	2,	0xeb,	0xeb,	false	},
	{"MOUSE",	3,	0x97c8,	0x97c8,	false	},
	{"MOVE",	4,	0xec,	0xec,	false	},
	{"NEW",		3,	0x97c7,	0x97c7,	true	},
	{"NEXT",	1,	0xed,	0xed,	false	},
	{"NOT",		3,	0xac,	0xac,	false	},
	{"OF",		2,	0xca,	0xca,	false	},
	{"OFF",		3,	0x87,	0x87,	false	},
	{"OLD",		1,	0x98c7,	0x98c7,	true	},
	{"ON",		2,	0xee,	0xee,	false	},
	{"OPENIN",	2,	0x8e,	0x8e,	false	},
	{"OPENOUT",	5,	0xae,	0xae,	false	},
	{"OPENUP",	6,	0xad,	0xad,	false	},
	{"OR",		2,	0x84,	0x84,	false	},
	{"ORIGIN",	2,	0x91c8,	0x91c8,	false	},
	{"OSCLI",	2,	0xff,	0xff,	false	},
	{"OTHERWISE",	2,	0x7f,	0x7f,	false	},
	{"OVERLAY",	2,	0xa3c8,	0xa3c8,	false	},
	{"PAGE",	2,	0xd0,	0x90,	true	},
	{"PI",		2,	0xaf,	0xaf,	true	},
	{"PLOT",	2,	0xf0,	0xf0,	false	},
	{"POINT",	5,	0x92c8,	0x92c8,	false	},
	{"POINT(",	2,	0xb0,	0xb0,	false	},
	{"POS",		3,	0xb1,	0xb1,	true	},
	{"PRINT",	1,	0xf1,	0xf1,	false	},
	{"PROC",	4,	0xf2,	0xf2,	false	},
	{"PTR",		3,	0xcf,	0x8f,	true	},
	{"QUIT",	1,	0x98c8,	0x98c8,	false	},
	{"RAD",		3,	0xb2,	0xb2,	false	},
	{"READ",	4,	0xf3,	0xf3,	false	},
	{"RECTANGLE",	3,	0x93c8,	0x93c8,	false	},
	{"REM",		3,	0xf4,	0xf4,	false	},
	{"RENUMBER",	3,	0x99c7,	0x99c7,	false	},
	{"REPEAT",	3,	0xf5,	0xf5,	false	},
	{"REPORT",	4,	0xf6,	0xf6,	true	},
	{"RESTORE",	3,	0xf7,	0xf7,	false	},
	{"RETURN",	1,	0xf8,	0xf8,	true	},
	{"RIGHT$(",	2,	0xc2,	0xc2,	false	},
	{"RND",		3,	0xb3,	0xb3,	true	},
	{"RUN",		3,	0xf9,	0xf9,	true	},
	{"SAVE",	2,	0x9ac7,	0x9ac7,	false	},
	{"SGN",		3,	0xb4,	0xb4,	false	},
	{"SIN",		3,	0xb5,	0xb5,	false	},
	{"SOUND",	2,	0xd4,	0xd4,	false	},
	{"SPC",		3,	0x89,	0x89,	false	},
	{"SQR",		3,	0xb6,	0xb6,	false	},
	{"STEP",	1,	0x88,	0x88,	false	},
	{"STEREO",	4,	0xa2c8,	0xa2c8,	false	},
	{"STOP",	4,	0xfa,	0xfa,	true	},
	{"STR$",	4,	0xc3,	0xc3,	false	},
	{"STRING$(",	4,	0xc4,	0xc4,	false	},
	{"SUM",		3,	0x8ec6,	0x8ec6,	false	},
	{"SWAP",	2,	0x94c8,	0x94c8,	false	},
	{"SYS",		3,	0x99c8,	0x99c8,	false	},
	{"TAB(",	4,	0x8a,	0x8a,	false	},
	{"TAN",		1,	0xb7,	0xb7,	false	},
	{"TEMPO",	2,	0x9fc8,	0x9fc8,	false	},
	{"TEXTLOAD",	5,	0x9bc7,	0x9bc7,	false	},
	{"TEXTSAVE",	5,	0x9cc7,	0x9cc7,	false	},
	{"THEN",	2,	0x8c,	0x8c,	false	},
	{"TIME",	2,	0xd1,	0x91,	true	},
	{"TINT",	4,	0x9cc8,	0x9cc8,	false	},
	{"TO",		2,	0xb8,	0xb8,	false	},
	{"TRACE",	2,	0xfc,	0xfc,	false	},
	{"TRUE",	4,	0xb9,	0xb9,	true	},
	{"TWIN",	4,	0x9dc7,	0x9dc7,	true	},
	{"TWINO",	2,	0x9ec7,	0x9ec7,	false	},
	{"UNTIL",	1,	0xfd,	0xfd,	false	},
	{"USR",		3,	0xba,	0xba,	false	},
	{"VAL",		3,	0xbb,	0xbb,	false	},
	{"VDU",		1,	0xef,	0xef,	false	},
	{"VOICE",	5,	0xa1c8,	0xa1c8,	false	},
	{"VOICES",	2,	0xa0c8,	0xa0c8,	false	},
	{"VPOS",	2,	0xbc,	0xbc,	true	},
	{"WAIT",	2,	0x96c8,	0x96c8,	true	},
	{"WHEN",	4,	0xc9,	0xc9,	false	},
	{"WHILE",	1,	0x95c8,	0x95c8,	false	},
	{"WIDTH",	2,	0xfe,	0xfe,	false	},
	{"[",		1,	0x00,	0x00,	false	}	/* TERM	*/
};


/**
 * Indexes into the keywords table for the various initial letters.
 * KWD_NO_MATCH indicates that there are no keywords starting with that letter.
 */

static enum parse_keyword parse_keyword_index[] = {
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
	KWD_LEFT_D,	/**< L	*/
	KWD_MID_D,	/**< M	*/
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
static enum parse_keyword parse_match_token(char **buffer);
static bool parse_process_string(char **read, char **write, char *dump);
static void parse_process_numeric_constant(char **read, char **write);
static bool parse_process_binary_constant(char **read, char **write, int *extra_spaces);
static void parse_process_fnproc(char **read, char **write);
static void parse_process_variable(char **read, char **write);
static void parse_process_whitespace(char **read, char **write, char *start_pos, int extra_spaces, struct parse_options *options);
static void parse_process_to_line_end(char **read, char **write);
static bool parse_is_name_body(char c);

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

char *parse_process_line(char *line, struct parse_options *options, bool *assembler, int *line_number, char *location)
{
	char			*read = line, *write = parse_buffer;
	int			read_number = -1;
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

		if (read_number < 0 || read_number > PARSE_MAX_LINE_NUMBER) {
			fprintf(stderr, "Error: Line number %u out of range%s\n", read_number, location);
			return NULL;
		} else if (read_number <= *line_number) {
			fprintf(stderr, "Error: Line number %u out of sequence%s\n", read_number, location);
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
	*write++ = 0;		/* Line number high byte placeholder.	*/
	*write++ = 0;		/* Line number low byte placeholder.	*/
	*write++ = 0;		/* Line length placeholder.		*/

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
		if (read_number != -1) {
			*line_number = read_number;
		} else if (*line_number == -1) {
			*line_number = options->line_start;
		} else {
			*line_number += options->line_increment;
			if (*line_number > PARSE_MAX_LINE_NUMBER) {
				fprintf(stderr, "Error: AUTO line number too large%s\n", location);
				return NULL;
			}
		}

		*(parse_buffer + 1) = (*line_number & 0xff00) >> 8;
		*(parse_buffer + 2) = (*line_number & 0x00ff);
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

	bool			statement_start = true;		/**< True while we're at the start of a statement.		*/
	bool			constant_due = false;		/**< True if a line number constant could be coming up.		*/
	bool			library_path_due = false;	/**< True if we're expecting a library path.			*/
	bool			clean_to_end = false;		/**< True if no non-whitespace has been found since set.	*/

	int			extra_spaces = 0;		/**< Extra spaces taken up by expended keywords.		*/
	enum parse_keyword	token = KWD_NO_MATCH;		/**< Storage for any keyword tokens that we look up.		*/

	char			*start_pos = *write;		/**< A pointer to the start of the statement.			*/

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

static enum parse_keyword parse_match_token(char **buffer)
{
	char			*start = *buffer;
	enum parse_keyword	keyword = KWD_NO_MATCH;
	int			result = 0;
	enum parse_keyword	full = KWD_NO_MATCH;
	char			*full_end = NULL;
	enum parse_keyword	partial = -1;
	char			*partial_end = NULL;

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
			partial_end = test + 1; /* Skip the . as well. */
		} else if (*match == '\0' && (!parse_keywords[keyword].var_start || !parse_is_name_body(*test))) {
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
	while (parse_is_name_body(**read))
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
	while (parse_is_name_body(**read))
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


/**
 * Test a character to see if it is a valid name body character (alphanumeric,
 * _ and `), returning true or false.
 *
 * \param c		The character to test.
 * \return		true if the character is valid; else false.
 */

static bool parse_is_name_body(char c)
{
	return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '`') ? true : false;
}

