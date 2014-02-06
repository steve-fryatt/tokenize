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

enum parse_error {
	PARSE_NO_ERROR = 0,		/**< No Error.				*/
	PARSE_MISMATCHED_QUOTES		/**< Mismatched string delimiter.	*/
};

/**
 * Parse a line of BASIC, returning a pointer to the tokenised form which will
 * remain valid until the function is called again.
 *
 * \param *line		Pointer to the line to process,
 * \param *assembler	Pointer to a boolean which is TRUE if we are in an
 *			assember section and FALSE otherwise; updated on exit.
 * \param *line_number	Pointer to a variable to hold the proposed next line
 *			number; updated on exit if a number was found.
 * \return		Pointer to the tokenised line, or NULL on error.
 */

char *parse_process_line(char *line, bool *assembler, unsigned *line_number);

#endif

