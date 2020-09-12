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
 * \file args.h
 *
 * Command Line Option Interface.
 */

#ifndef TOKENIZE_ARGS_H
#define TOKENIZE_ARGS_H

#include <stdbool.h>

enum args_type {
	ARGS_TYPE_NONE = 0,
	ARGS_TYPE_STRING,
	ARGS_TYPE_INT,
	ARGS_TYPE_BOOL
};

union args_value {
	char	*string;			/**< The data in the form of a pointer to a string.	*/
	int	integer;			/**< The data in the form of an integer.		*/
	bool	boolean;			/**< The data in the form of a boolean value.		*/
};

struct args_data {
	union args_value	value;		/**< The data itself.					*/

	struct args_data	*next;		/**< Pointer to the next linked list entry.		*/
};

struct args_option {
	char			*name;		/**< The name of the option.				*/

	bool			required;	/**< True if the keyword must always included.		*/
	bool			multiple;	/**< True if the keyword can be used more than once.	*/

	enum args_type		type;		/**< The type of data used by the option.		*/
	struct args_data	*data;		/**< Pointer to the data for the option, if any.	*/

	struct args_option	*next;		/**< Pointer to next linked list entry.			*/
};

/**
 * Process a program's command line options, returning a pointer to the first
 * item in a linked list of data or NULL if an error prevented it from
 * completing.
 *
 * \param argc			The argc value passed into main().
 * \param *argv[]		The argv value passed into main().
 * \param *definition		A list of option definitions to be processed.
 * \return			Pointer to the linked list of data, or NULL.
 */

struct args_option *args_process_line(int argc, char *argv[], char *definition);

#endif

