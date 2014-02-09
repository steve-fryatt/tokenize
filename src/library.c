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
 * \file library.c
 *
 * Library Tracking and Management, implementation.
 */

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Local source headers. */

#include "library.h"

struct library_file {
	char			*file;		/**< Pointer to the name of the file.	*/

	struct library_file	*next;		/**< Pointer to the next file record.	*/
};

struct library_path {
	char			*name;		/**< Pointer to the name of the path.	*/
	char			*path;		/**< Pointer to the path location.	*/

	struct library_path	*next;		/**< Pointer to the next file record.	*/
};

static struct library_file	*library_file_tail = NULL;
static struct library_file	*library_file_head = NULL;

static struct library_path	*library_path_head = NULL;



/**
 * Add a combined path definition to the list of library file paths. The
 * definition is in the format "name:path".
 *
 * \param *combined	The definition of the new path.
 */

void library_add_path_combined(char *combined)
{
	char *name = NULL, *path = NULL;

	name = strdup(combined);
	if (name == NULL)
		return;
	
	path = strchr(name, ':');
	if (path == NULL)
		return;
	
	*path++ = '\0';

	library_add_path(name, path);
}


/**
 * Add a path definition to the list of library file paths.
 *
 * \param *name		The name of the path.
 * \param *path		The file path.
 */

void library_add_path(char *name, char *path)
{
	struct library_path	*new = NULL;

	if (name == NULL || path == NULL)
		return;

	new = malloc(sizeof(struct library_path));
	if (new == NULL)
		return;

	printf("Adding path '%s' to '%s'\n", name, path);

	new->name = strdup(name);
	new->path = strdup(path);

	new->next = library_path_head;
	library_path_head = new;
}


/**
 * Add a file to the list of files to be processed. The name is supplied raw, and
 * will be interpreted according to any library and system paths already defined
 *
 * \param *fild		The filename to be added to the library list.
 */

void library_add_file(char *file)
{
	struct library_path	*paths = library_path_head;
	struct library_file	*new = NULL;
	char			*copy = NULL, *tail = NULL, *temp;

	printf("Adding file: '%s'\n", file);

	copy = strdup(file);
	if (copy == NULL)
		return;

	tail = strchr(copy, ':');
	if (tail != NULL) {
		*tail++ = '\0';

		while (paths != NULL && strcmp(paths->name, copy) != 0)
			paths = paths->next;

		if (paths != NULL) {
			temp = copy;

			copy = malloc(sizeof(char) * (strlen(paths->path) + strlen(tail) + 1));
			if (copy == NULL) {
				free(temp);
				return;
			}

			strcpy(copy, paths->path);
			strcat(copy, tail);
			free(temp);
		}
	}

	new = malloc(sizeof(struct library_file));
	if (new == NULL)
		return;

	new->next = NULL;
	new->file = copy;
	if (new->file == NULL) {
		free(new);
		return;
	}

	if (library_file_tail == NULL) {
		library_file_head = new;
		library_file_tail = new;
	} else {
		library_file_tail->next = new;
		library_file_tail = new;
	}
}


/**
 * Get the next file to be processed from the library list, as a complete
 * filename ready to be used by the file load routine.
 *
 * \param *buffer	Pointer to a buffer to take the filename.
 * \param len		The length of the supplied buffer.
 * \return		True if a filename was returned; else false.
 */

FILE *library_get_file(void)
{
	struct library_file	*old = NULL;
	FILE			*file = NULL;

	while (library_file_head != NULL && file == NULL) {
		printf("Trying to open library file '%s'\n", library_file_head->file);

		file = fopen(library_file_head->file, "r");

		old = library_file_head;
		library_file_head = library_file_head->next;
		if (library_file_tail == old)
			library_file_tail = NULL;
		if (old->file != NULL)
			free(old->file);
		free(old);
	}

	return file;
}

