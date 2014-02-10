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
 * \file args.c
 *
 * Command Line Options, implementation.
 */

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Local source headers. */

#include "args.h"

#include "string.h"


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

struct args_option *args_process_line(int argc, char *argv[], char *definition)
{
	char			*defs, *token;
	int			i;
	struct args_option	*options = NULL, *tail = NULL, *new = NULL;

	if (argv == NULL || definition == NULL)
		return NULL;

	/* Take a copy of the definitions, as we'll break the string up and then
	 * use it for the name storage.
	 */ 

	defs = strdup(definition);
	if (defs == NULL)
		return NULL;

	/* Process the parameter options from the configuration string one by
	 * one, and build the necessary data structures ready to parse the
	 * command line. */

	token = strtok(defs, ",");

	while (token != NULL) {
		char *qualifiers = strchr(token, '/');
		
		if (qualifiers != NULL)
			*qualifiers++ = '\0';

		/* Create a new options block, link it to the end of the
		 * chain and initialise the contents.
		 */

		new = malloc(sizeof(struct args_option));
		if (new == NULL)
			return NULL;

		if (options == NULL)
			options = new;
		else if (tail != NULL)
			tail->next = new;

		new->name = token;
		new->required = false;
		new->multiple = false;
		new->type = ARGS_TYPE_STRING;
		new->data = NULL;
		new->next = NULL;

		tail = new;

		/* Process the qualifier flags from the end of the option. */

		while (qualifiers != NULL && *qualifiers != '\0') {
			switch (toupper(*qualifiers++)) {
			case 'A':	/* */
				break;
			case 'I':	/* Integer Type */
				if (new->type != ARGS_TYPE_STRING)
					return NULL;
				new->type = ARGS_TYPE_INT;
				break;
			case 'K':	/* Option Name is Required. */
				new->required = true;
				break;
			case 'M':	/* Can take multiple values. */
				new->multiple = true;
				break;
			case 'S':	/* Switch (Boolean) Type. */
				if (new->type != ARGS_TYPE_STRING)
					return NULL;
				new->type = ARGS_TYPE_BOOL;
				break;
			}
		}

		token = strtok(NULL, ",");
	}

	/* Now process the contents of argv[]. We assume that argv[0] is the
	 * command used to call the client, so start at argv[1].
	 */

	for (i = 1; i < argc; i++) {
		struct args_option	*search = options;
		char			*name = NULL;

		if (*argv[i] == '-') {
			/* The entry's an option name. */
		
			name = argv[i] + 1;

			while (search != NULL && string_nocase_strcmp(name, search->name) != 0)
				search = search->next;

			if (search == NULL) {
				fprintf(stderr, "Switch -%s not recognised.\n", name);
				return NULL;
			}

			if ((search->data != NULL) && !search->multiple) {
				fprintf(stderr, "Switch -%s can not appear multiple times.\n", name);
				return NULL;
			}

			if (search->type != ARGS_TYPE_BOOL) {
				if (i + 1 >= argc || *argv[i + 1] == '-') {
					fprintf(stderr, "Switch -%s requires a value.\n", name);
					return NULL;
				}
			
				i++;
			}
		} else {
			/* The entry's a free one, so match it to the first open
			 * option in the list.
			 */

			while (search != NULL && (search->type == ARGS_TYPE_BOOL || search->required ||
					(!search->multiple && (search->data != NULL))))
				search = search->next;

			if (search == NULL) {
				fprintf(stderr, "Option '%s' not recognised.\n", argv[i]);
				return NULL;
			}
		}

		/* A valid match was found, so add the data to the option. */

		if (search != NULL) {
			struct args_data *new = NULL, *end = NULL;

			/* Create a new data block and add it to the list. */

			new = malloc(sizeof(struct args_data));
			if (new == NULL)
				return NULL;

			switch (search->type) {
			case ARGS_TYPE_BOOL:
				new->value.boolean = true;
				break;
			case ARGS_TYPE_INT:
				new->value.integer = atoi(argv[i]);
				break;
			case ARGS_TYPE_STRING:
				new->value.string = argv[i];
				break;
			default:
				break;
			}
			new->next = NULL;

			if (search->data == NULL) {
				search->data = new;
			} else {
				for (end = search->data; end->next != NULL; end = end->next);
				end->next = new;
			}
		}
	}

	return options;
}



