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
 * \file proc.c
 *
 * Function and Procedure analysis, implementation.
 */

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/* Local source headers. */

#include "proc.h"

#include "msg.h"

enum proc_type {
	PROC_UNKNOWN = 0,			/**< The routine's type isn't known.				*/
	PROC_FUNCTION,				/**< The routine is a function.					*/
	PROC_PROCEDURE				/**< The routine is a procedure.				*/
};

/**
 * Procedure definition, forming one entry in a linked list.
 */

struct proc_entry {
	char			*name;		/**< Pointer to the routine's name, in full.			*/
	enum proc_type		type;		/**< The routine's type (function or procedure).		*/

	unsigned		definitions;	/**< The number of times the routine has been defined.		*/
	unsigned		calls;		/**< The number of times the routine has been called.		*/

	struct proc_entry	*next;		/**< Pointer to the next routine in the chain, or NULL.		*/
};

#define PROC_INDEXES 128
static struct proc_entry	*proc_list[PROC_INDEXES];

static struct proc_entry *proc_create(enum proc_type type, char *name);
static struct proc_entry *proc_find(enum proc_type type, char *name);
static int proc_find_index(char *name);
static char *proc_prefix_name(enum proc_type type);


/**
 * Initialise the procedure module.
 */

void proc_initialise(void)
{
	int	i;

	for (i = 0; i < PROC_INDEXES; i++)
		proc_list[i] = NULL;
}


/**
 * Generate a report on the list of functions and procedures, detaining
 * missing definitions, multiple definitions and optionally unused definitions.
 *
 * \param unused	True to report unused definitions; False to ignore.
 */

void proc_report(bool unused)
{
	int			index;
	struct proc_entry	*list;

	for (index = 0; index < PROC_INDEXES; index++) {
		if (proc_list[index] == NULL)
			continue;

		list = proc_list[index];

		while (list != NULL) {
			if (list->name == NULL || list->type == PROC_UNKNOWN)
				continue;

			if (list->definitions == 0 && list->calls > 0)
				msg_report(MSG_PROC_MISSING_DEF, proc_prefix_name(list->type), list->name);

			if (list->definitions > 1)
				msg_report(MSG_PROC_MULTIPLE_DEF, proc_prefix_name(list->type), list->name);

			if (unused && list->definitions > 0 && list->calls == 0)
				msg_report(MSG_PROC_UNUSED, proc_prefix_name(list->type), list->name);

			list = list->next;
		}
	}
}


/**
 * Process a function or procedure in the parse buffer, adding it to the list
 * of known routines and recording the number of calls and defintions.
 *
 * \param *name		Pointer to the start of the routine name in the output
 *			buffer.
 * \param is_function	True if the routine is an FN; False if it is a PROC.
 * \param is_definition	True if this is a DEF; False for a call.
 */

void proc_process(char *name, bool is_function, bool is_definition)
{
	struct proc_entry	*routine;
	enum proc_type		type = (is_function) ? PROC_FUNCTION : PROC_PROCEDURE;

	/* Look the variable name up in the index. */

	routine = proc_find(type, name);
	if (routine == NULL)
		routine = proc_create(type, name);

	if (routine == NULL)
		return;

	/* If the routine was found or created... */
 
	if (is_definition)
		routine->definitions++;
	else
		routine->calls++;
}


/**
 * Create a new routine, returning a pointer to its data block.
 *
 * \param *type		The type of the new routine (FN or PROC).
 * \param *name		Pointer to the name to use for the new variable.
 * \return		Pointer to the newly created block, or NULL on failure.
 */

static struct proc_entry *proc_create(enum proc_type type, char *name)
{
	struct proc_entry	*routine;
	int			index;

	if (name == NULL)
		return NULL;

	routine = malloc(sizeof(struct proc_entry));
	if (routine == NULL) {
		msg_report(MSG_PROC_NOMEM, proc_prefix_name(type), name);
		return NULL;
	}

	routine->name = strdup(name);
	routine->type = type;

	routine->definitions = 0;
	routine->calls = 0;

	index = proc_find_index(name);
	routine->next = proc_list[index];
	proc_list[index] = routine;

	return routine;
}


/**
 * Given a function or procedure's type and name, find its record if one exists.
 *
 * \param type		The type of the routine (function or procedure).
 * \param *name		Pointer to the routine's name.
 * \return		Pointer to the routine's record, or NULL if not found.
 */

static struct proc_entry *proc_find(enum proc_type type, char *name)
{
	struct proc_entry	*list;
	int			index;

	if (name == NULL)
		return NULL;

	index = proc_find_index(name);
	list = proc_list[index];

	while (list != NULL && list->name != NULL && (list->type != type || strcmp(list->name, name) != 0))
		list = list->next;

	if (list == NULL || list->name == NULL)
		return NULL;

	return list;
}


/**
 * Return the index for a given function or procedure name.
 *
 * \param *name		Pointer to the name to index.
 * \return		The index from the name.
 */

static int proc_find_index(char *name)
{
	int	index;

	if (name == NULL)
		return 0;

	index = (int) *name;
	if (index < 0 || index >= PROC_INDEXES)
		return 0;

	return index;
}


/**
 * Return the token name for a given routine.
 *
 * \param type		The type of routine.
 * \return		Pointer to the name of the routine.
 */

static char *proc_prefix_name(enum proc_type type)
{
	switch (type) {
	case PROC_FUNCTION:
		return "FN";
	case PROC_PROCEDURE:
		return "PROC";
	default:
		return "";
	}
}

