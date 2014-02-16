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
 * \file parse.h
 *
 * Basic line parser, interface.
 */

#ifndef TOKENIZE_PARSE_H
#define TOKENIZE_PARSE_H

#include <stdbool.h>

/**
 * Structure defining the parse options.
 */

struct parse_options {
	unsigned	line_increment;		/**< The line number increment for AUTO.			*/

	unsigned	tab_indent;		/**< The number of spaces to convert tabs into.			*/

	bool		link_libraries;		/**< True to link LIBRARY files; False to ignore.		*/

	bool		verbose_output;		/**< True to produce verbose output; false to be silent.	*/

	bool		crunch_body_rems;	/**< True to remove all body REM statements.			*/
	bool		crunch_rems;		/**< True to remove all REM statements.				*/
	bool		crunch_empty;		/**< True to remove all empty statements.			*/
	bool		crunch_indent;		/**< True to remove all leading indent spaces.			*/
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
	PARSE_ERROR_OPEN_STRING = 256,		/**< Error: unterminated string.			*/
	PARSE_ERROR_DELETED_STATEMENT = 257,	/**< Error: unclean deleted statement.			*/
	PARSE_ERROR_LINE_CONSTANT = 258		/**< Error: line number constant out of range.		*/
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
 * \param *location	Pointer to source file location information.
 * \return		Pointer to the tokenised line, or NULL on error.
 */

char *parse_process_line(char *line, struct parse_options *options, bool *assembler, unsigned *line_number, char *location);

#endif

