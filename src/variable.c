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
 * \file variable.c
 *
 * Variable analysis and substitution, implementation.
 */

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h> // \TODO -- Debug; remove!

/* Local source headers. */

#include "variable.h"

#include "msg.h"

enum variable_mode {
	VARIABLE_IGNORE = 0,			/**< Variable is to be ignored.					*/
	VARIABLE_CONSTANT			/**< Variable is a constant, and should be replaced by value.	*/
};

enum variable_type {
	VARIABLE_UNKNOWN = 0,			/**< The variable's data type isn't known.			*/
	VARIABLE_STRING,			/**< The variable is a string variable.				*/
	VARIABLE_INTEGER,			/**< The variable is an integer.				*/
	VARIABLE_REAL				/**< The variable is a real.					*/
};

union variable_value {
	char	*string;			/**< Pointer to a textual variable value.			*/
	int	integer;			/**< Integer variable value.					*/
	double	real;				/**< Real variable value.					*/
};

/**
 * Variable definition, forming one entry in a linked list.
 */

struct variable_entry {
	char			*name;		/**< Pointer to the variable's name, in full.			*/
	enum variable_type	type;		/**< The variable's data type.					*/
	union variable_value	value;		/**< The variable's value information.				*/

	enum variable_mode	mode;		/**< The variable's mode.					*/
	unsigned		count;		/**< The number of times the variable has been seen.		*/

	struct variable_entry	*next;		/**< Pointer to the next variable in the chain, or NULL.	*/
};


static struct variable_entry	*variable_list = NULL;


static enum variable_type variable_find_type(char *name);
static struct variable_entry *variable_find(char *name);


/**
 * Initialise the variable module.
 */

void variable_initialise(void)
{

}


/**
 * Add a constant definition in the form of a single name=value string, such as
 * would be obtained from the command-line.
 *
 * \param *constant	Pointer to the defintion string.
 * \return		True on success; else false;
 */

bool variable_add_constant_combined(char *constant)
{
	char	*name = NULL, *value = NULL;
	bool	result;

	name = strdup(constant);
	if (name == NULL)
		return false;

	value = strchr(name, '=');
	if (value == NULL) {
		free(name);
		return false;
	}

	*value++ = '\0';

	result = variable_add_constant(name, value);
	free(name);

	return result;
}


/**
 * Add a constant variable definition. These are pre-defined, and whenever one
 * is encountered in a program it will be replaced by its value.
 *
 * \param *name		Pointer to the variable's name.
 * \param *value	Pointer to the variable's value.
 * \return		True if successful; else false.
 */

bool variable_add_constant(char *name, char *value)
{
	struct variable_entry	*variable = NULL;

	if (name == NULL || value == NULL)
		return false;

	/* See if the constant already exists. If it does, it can't -- by
	 * definition -- be redefined a second time.
	 */

	variable = variable_find(name);
	if (variable != NULL) {
		msg_report(MSG_CONST_REDEF, name);
		return false;
	}

	/* If the variable doesn't exist, create a new record for it. */

	variable = malloc(sizeof(struct variable_entry));
	if (variable == NULL) {
		msg_report(MSG_VAR_NOMEM, name);
		return false;
	}

	variable->name = strdup(name);
	variable->type = variable_find_type(variable->name);
	switch (variable->type) {
	case VARIABLE_INTEGER:
		variable->value.integer = atoi(value);
		break;
	case VARIABLE_STRING:
		variable->value.string = strdup(value);
		break;
	case VARIABLE_REAL:
		variable->value.real = atof(value);
		break;
	default:
		variable->value.string = NULL;
		break;
	}

	variable->mode = VARIABLE_CONSTANT;
	variable->count = 0;

	variable->next = variable_list;
	variable_list = variable;

	return true;
}


/**
 * Process a variable in the parse buffer, looking names up and handing them
 * appropriately: for constants...
 *
 * - Variables on the left-hand side are assignments, and are flagged back to
 *   the caller to be removed.
 *
 * - Variables on the right-hand side are replaced by their constant value.
 *
 * \param *name		Pointer to the start of the variable name in the output
 *			buffer.
 * \param **write	Pointer to the output buffer write pointer, which will
 *			be updated on exit.
 * \return		True if the variable is being assigned to, else false.
 */

bool variable_process(char *name, char **write, bool statement_left)
{
	struct variable_entry	*variable;
	int			written;
	char			*read;

	/* Look the variable name up in the index. */

	variable = variable_find(name);
	if (variable == NULL)
		return false;

	/* If the variable was found... */

	if (statement_left) {
		/* The variable was on the left, and so was being assigned to.
		 * Therefore we return true to indicate that it should be removed.
		 */

		return true;
	} else {
		/* The variable was on the right, so we can replace it in the
		 * output with the constant that it contains.
		 */

		switch (variable->type) {
		case VARIABLE_INTEGER:
			written = sprintf(name, "%d", variable->value.integer);
			if (written > 0)
				*write = name + written;
			break;
		case VARIABLE_REAL:
			written = sprintf(name, "%f", variable->value.real);
			if (written > 0)
				*write = name + written;
			break;
		case VARIABLE_STRING:
			read = variable->value.string;
			*name++ = '\"';
			while (*read != '\0') {
				if (*read == '\"')
					*name++ = '\"';
				*name++ = *read++;
			}
			*name++ = '\"';
			*write = name;
			break;
		default:
			break;
		}
	}

	return false;
}


/**
 * Find the type of a variable from its name, by looking at the last character
 * and testing it against the permissable characters. VARIABLE_UNKNOWN is
 * returned if the character isn't valid.
 *
 * \param *name		The name to test.
 * \return		The type indicated by the name.
 */

static enum variable_type variable_find_type(char *name)
{
	char			end;

	if (name == NULL)
		return VARIABLE_UNKNOWN;

	/* Find the last character in the variable name. */

	end = *(name + strlen(name) - 1);
	switch (end) {
	case '%':
		return VARIABLE_INTEGER;
		break;
	case '$':
		return VARIABLE_STRING;
		break;
	default:
		if ((end >= 'A' && end <= 'Z') || (end >= 'a' && end <= 'z') || (end >= '0' && end <= '9') || end == '_' || end == '`')
			return VARIABLE_REAL;
		break;
	}

	return VARIABLE_UNKNOWN;
}


/**
 * Given a variable name, find its record if one exists.
 *
 * \param *name		Pointer to the variable's name.
 * \return		Pointer to the variable's record, or NULL if not found.
 */

static struct variable_entry *variable_find(char *name)
{
	struct variable_entry *list = variable_list;

	while (list != NULL && list->name != NULL && strcmp(list->name, name) != 0)
		list = list->next;

	if (list == NULL || list->name == NULL)
		return NULL;

	return list;
}

