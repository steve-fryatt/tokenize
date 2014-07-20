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
#include <stdio.h>

/* Local source headers. */

#include "variable.h"

#include "msg.h"

enum variable_mode {
	VARIABLE_UNSET = 0,			/**< Variable has not yet been set up.				*/
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

	unsigned		assignments;	/**< The number of times the variable has been assigned.	*/
	unsigned		reads;		/**< The number of times the variable has been read.		*/

	struct variable_entry	*next;		/**< Pointer to the next variable in the chain, or NULL.	*/
};

#define VARIABLE_INDEXES 128
static struct variable_entry	*variable_list[VARIABLE_INDEXES];

static void variable_substitute_constant(struct variable_entry *variable, char *name, char **write);
static struct variable_entry *variable_create(char *name);
static enum variable_type variable_find_type(char *name);
static struct variable_entry *variable_find(char *name);
static int variable_find_index(char *name);


/**
 * Initialise the variable module.
 */

void variable_initialise(void)
{
	int	i;

	for (i = 0; i < VARIABLE_INDEXES; i++)
		variable_list[i] = NULL;
}


/**
 * Generate a report on the variable information.
 */

void variable_report(void)
{
	int			index;
	struct variable_entry	*list;

	for (index = 0; index < VARIABLE_INDEXES; index++) {
		if (variable_list[index] == NULL)
			continue;

		list = variable_list[index];

		while (list != NULL) {
			if (list->name == NULL)
				continue;

			if (list->assignments == 0 && list->reads > 0)
				msg_report(MSG_VAR_MISSING_DEF, list->name);

			list = list->next;
		}
	}
}


/**
 * Add a constant definition in the form of a single name=value string, such as
 * would be obtained from the command-line.
 *
 * \param *constant		Pointer to the defintion string.
 * \return			True on success; else false;
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
 * \param *name			Pointer to the variable's name.
 * \param *value		Pointer to the variable's value.
 * \return			True if successful; else false.
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

	variable = variable_create(name);
	if (variable == NULL)
		return false;

	variable->mode = VARIABLE_CONSTANT;

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
 * \param *name			Pointer to the start of the variable name in the output
 *				buffer.
 * \param **write		Pointer to the output buffer write pointer, which will
 *				be updated on exit.
 * \param statement_left	True if this is an assignment; False for a read.
 * \return			True if the variable is being assigned to, else false.
 */

bool variable_process(char *name, char **write, bool statement_left)
{
	struct variable_entry	*variable;

	/* Look the variable name up in the index. */

	variable = variable_find(name);
	if (variable == NULL)
		variable = variable_create(name);

	if (variable == NULL)
		return false;

	/* If the variable was found or created... */
 
	if (statement_left)
		variable->assignments++;
	else
		variable->reads++;

	switch (variable->mode) {
	case VARIABLE_CONSTANT:
		if (statement_left)
			return true;

		variable_substitute_constant(variable, name, write);
		break;

	default:
		break;
	}

	return false;
}


/**
 * Write a variable's value out into a buffer, starting at the specified point
 * and updating the line pointer when done.
 *
 * \param *variable		Pointer to the variable to be substituted.
 * \param *name			Pointer to the start location in the write buffer.
 * \param **write		Pointer to the buffer position pointer, to be updated
 *				after the substitution is completed.
 */

static void variable_substitute_constant(struct variable_entry *variable, char *name, char **write)
{
	int	written;
	char	*read;

	if (variable == NULL)
		return;

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


/**
 * Create a new variable, returning a pointer to its data block.
 *
 * \param *name			Pointer to the name to use for the new variable.
 * \return			Pointer to the newly created block, or NULL on failure.
 */

static struct variable_entry *variable_create(char *name)
{
	struct variable_entry	*variable;
	int			index;

	if (name == NULL)
		return NULL;

	variable = malloc(sizeof(struct variable_entry));
	if (variable == NULL) {
		msg_report(MSG_VAR_NOMEM, name);
		return NULL;
	}

	variable->name = strdup(name);
	variable->type = variable_find_type(variable->name);
	switch (variable->type) {
	case VARIABLE_INTEGER:
		variable->value.integer = 0;
		break;
	case VARIABLE_REAL:
		variable->value.real = 0.0;
		break;
	case VARIABLE_STRING:
	default:
		variable->value.string = NULL;
		break;
	}

	variable->mode = VARIABLE_UNSET;
	variable->assignments = 0;
	variable->reads = 0;

	index = variable_find_index(name);
	variable->next = variable_list[index];
	variable_list[index] = variable;

	return variable;
}


/**
 * Find the type of a variable from its name, by looking at the last character
 * and testing it against the permissable characters. VARIABLE_UNKNOWN is
 * returned if the character isn't valid.
 *
 * \param *name			The name to test.
 * \return			The type indicated by the name.
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
 * \param *name			Pointer to the variable's name.
 * \return			Pointer to the variable's record, or NULL if not found.
 */

static struct variable_entry *variable_find(char *name)
{
	struct variable_entry	*list;
	int			index;

	if (name == NULL)
		return NULL;

	index = variable_find_index(name);
	list = variable_list[index];

	while (list != NULL && list->name != NULL && strcmp(list->name, name) != 0)
		list = list->next;

	if (list == NULL || list->name == NULL)
		return NULL;

	return list;
}


/**
 * Return the index for a given variable name.
 *
 * \param *name			Pointer to the name to index.
 * \return			The index from the name.
 */

static int variable_find_index(char *name)
{
	int	index;

	if (name == NULL)
		return 0;

	index = (int) *name;
	if (index < 0 || index >= VARIABLE_INDEXES)
		return 0;

	return index;
}

