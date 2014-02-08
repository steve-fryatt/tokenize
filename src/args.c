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
	struct args_option	*options = NULL, *new = NULL;

	if (argv == NULL || definition == NULL)
		return NULL;

	/* Take a copy of the definitions, as we'll break the string up and then
	 * use it for the name storage.
	 */ 

	defs = strdup(definition);
	if (defs == NULL)
		return NULL;

	/* Process the parameter options one by one. */

	token = strtok(defs, ",");

	while (token != NULL) {
		char *qualifiers = strchr(token, '/');
		
		if (qualifiers != NULL)
			*qualifiers++ = '\0';

		new = malloc(sizeof(struct args_option));
		if (new == NULL)
			return NULL;

		if (options == NULL)
			options = new;
		else
			options->next = new;

		new->name = token;
		new->required = false;
		new->multiple = false;
		new->isswitch = false;
		new->found = false;
		new->next = NULL;

		while (qualifiers != NULL && *qualifiers != '\0') {
			switch (toupper(*qualifiers++)) {
			case 'A':
				break;
			case 'K':
				new->required = true;
				break;
			case 'M':
				new->multiple = true;
				break;
			case 'S':
				new->isswitch = true;
				break;
			}
		}

		token = strtok(NULL, ",");
	}

	for (i = 1; i < argc; i++) {
		printf("Found option: '%s'\n", argv[i]);
	
	}

	return options;
}



