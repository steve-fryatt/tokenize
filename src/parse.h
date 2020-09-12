/* Copyright 2014, Stephen Fryatt (info@stevefryatt.org.uk)
 *
 * This file is part of Tokenize:
 *
 *   http://www.stevefryatt.org.uk/software/
 *
 * Licensed under the EUPL, Version 1.2 only (the "Licence");
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
 * \file parse.h
 *
 * Basic line parser, interface.
 */

#ifndef TOKENIZE_PARSE_H
#define TOKENIZE_PARSE_H

#include <stdbool.h>

#define PARSE_MAX_LINE_NUMBER 65279

/**
 * List of keyword array indexes. This must match the entries in the
 * parse_keywords[] array defined in parse.c.
 */

enum parse_keyword {
	KWD_NO_MATCH = -1,	/**< Indicates that no keyword match is available.		*/
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
 * Structure defining the parse options.
 */

struct parse_options {
	unsigned	line_start;		/**< The initial line number for AUTO.				*/
	unsigned	line_increment;		/**< The line number increment for AUTO.			*/

	unsigned	tab_indent;		/**< The number of spaces to convert tabs into.			*/

	bool		link_libraries;		/**< True to link LIBRARY files; False to ignore.		*/

	bool		convert_swis;		/**< True to convert SWI names into numbers.			*/

	bool		verbose_output;		/**< True to produce verbose output; false to be silent.	*/

	bool		crunch_body_rems;	/**< True to remove all body REM statements.			*/
	bool		crunch_rems;		/**< True to remove all REM statements.				*/
	bool		crunch_empty;		/**< True to remove all empty statements.			*/
	bool		crunch_empty_lines;	/**< True to remove completely empty lines.			*/
	bool		crunch_indent;		/**< True to remove all leading indent spaces.			*/
	bool		crunch_trailing;	/**< True to remove all trailing whitespace, TEXTLOAD-style.	*/
	bool		crunch_whitespace;	/**< True to reduce contiguous whitespace to a single space.	*/
	bool		crunch_all_whitespace;	/**< True to remove all whitespace.				*/
};

/**
 * Output status for statement parsing, indicating errors or the kind
 * of successful outcome that was reached.
 */

enum parse_status {
	PARSE_DELETED = 0,			/**< The statement was deleted by the parser.		*/
	PARSE_WHITESPACE = 1,			/**< The statement consisted entirely of whitespace.	*/
	PARSE_COMMENT = 2,			/**< The statement was a comment.			*/
	PARSE_COMPLETE = 3,			/**< The statement was none of the above.		*/
	PARSE_ERROR_DELETED_STATEMENT = 256,	/**< Error: unclean deleted statement.			*/
	PARSE_ERROR_LINE_CONSTANT = 257,	/**< Error: line number constant out of range.		*/
	PARSE_ERROR_TOO_LONG = 258		/**< Error: line too long.				*/
};

/**
 * Parse a line of BASIC, returning a pointer to the tokenised form which will
 * remain valid until the function is called again.
 *
 * \param *line		Pointer to the line to process.
 * \param *options	Parse options block to set the configuration
 * \param *assembler	Pointer to a boolean which is TRUE if we are in an
 *			assember section and FALSE otherwise; updated on exit.
 * \param *line_number	Pointer to a variable to hold the proposed next line
 *			number; updated on exit if a number was found.
 * \return		Pointer to the tokenised line, or NULL on error.
 */

char *parse_process_line(char *line, struct parse_options *options, bool *assembler, int *line_number);


/**
 * Return the "right" token for a keyword.
 *
 * \param keyword	The keyword to return a token for.
 * \return		The "right" token for the keyword.
 */

unsigned parse_get_token(enum parse_keyword keyword);

#endif

