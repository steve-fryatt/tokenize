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
	int		length;		/**< The length of the keyword.				*/
	int		abbrev;		/**< The minimum number of characters allowed.		*/
	unsigned	start;		/**< The token if at the start of a statement.		*/
	unsigned	elsewhere;	/**< The token if elsewhere in a statement.		*/
	bool		alone;		/**< true if the token can not be followed by a letter.	*/
};

/**
 * The table of known keywords and their tokens.  The keywords *must* be in
 * alphabetical order, to allow the search routine to operate.
 */

#define MAX_KEYWORDS 6

static struct keyword parse_keywords[] = {
	{ "ABS",	3,	3,	0x94,	0x94,	false	},
	{ "ACS",	3,	3,	0x95,	0x95,	false	},
	{ "ADVAL",	5,	2,	0x96,	0x96,	false	},
	{ "AND",	3,	1,	0x80,	0x80,	false	},

	{ "DEF",	3,	3,	0xdd,	0xdd,	false	},
	
	{ "REM",	3,	3,	0xf4,	0xf4,	false	},

	{ "[",		1,	1,	0x00,	0x00,	false	}
};

/**
 * Indexes into the keywords table for the various initial letters. -1
 * indicates that there are no keywords starting with that letter.
 */

static int parse_keyword_index[] = {
	0,	/**< A	*/
	-1,	/**< B	*/
	-1,	/**< C	*/
	4,	/**< D	*/
	-1,	/**< E	*/
	-1,	/**< F	*/
	-1,	/**< G	*/
	-1,	/**< H	*/
	-1,	/**< I	*/
	-1,	/**< J	*/
	-1,	/**< K	*/
	-1,	/**< L	*/
	-1,	/**< M	*/
	-1,	/**< N	*/
	-1,	/**< O	*/
	-1,	/**< P	*/
	-1,	/**< Q	*/
	5,	/**< R	*/
	-1,	/**< S	*/
	-1,	/**< T	*/
	-1,	/**< U	*/
	-1,	/**< V	*/
	-1,	/**< W	*/
	-1,	/**< X	*/
	-1,	/**< Y	*/
	-1	/**< Z	*/
};

static char parse_buffer[MAX_TOKENISED_LINE];


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
	int	first = -1;
	int	last = -1;
	int	full = -1;
	int	partial = -1;

	/* If the code doesn't start with an upper case letter, it's not a keyword */

	if (*start < 'A' || *start > 'Z')
		return -1;

	/* Find the first entry in the keyword table that will match. */

	first = parse_keyword_index[*start - 'A'];
	if (first == -1)
		return -1;

	while (parse_keywords[first].name[0] = *start) {
		for (i = 1; start[i] != '\n' && i < parse_keywords[first].length; i++) {
			if (i == parse_keywords[first].length - 1) {
				full = first;
				break;
			}
		
		}
	
	
		first++;
	}

	if (parse_keywords[first].name[0] > *start)
		return -1;

	return 0;
}


