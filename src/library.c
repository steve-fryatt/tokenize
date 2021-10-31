/* Copyright 2014, Stephen Fryatt (info@stevefryatt.org.uk)
 *
 * This file is part of Tokenize:
 *
 *   http://www.stevefryatt.org.uk/risc-os/
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

#include "msg.h"
#include "string.h"

#define LIBRARY_MAX_FILENAME 256

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

static char			library_filename_buffer[LIBRARY_MAX_FILENAME];	/**< Buffer holding the last filename.	*/
static char			*library_filename = NULL;			/**< Pointer to the last filename.	*/


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
	if (path == NULL) {
		free(name);
		return;
	}

	*path++ = '\0';

	library_add_path(name, path);
	free(name);
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
	char			*copy = NULL;
	struct library_file	*new = NULL;
#ifdef LINUX
	char			*tail = NULL, *temp;
	struct library_path	*paths = library_path_head;
#endif

	copy = strdup(file);
	if (copy == NULL)
		return;

#ifdef LINUX
	tail = strchr(copy, ':');
	if (tail != NULL) {
		*tail++ = '\0';

		while (paths != NULL && string_nocase_strcmp(paths->name, copy) != 0)
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
#endif

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
 * \return		True if a filename was returned; else false.
 */

FILE *library_get_file(void)
{
	struct library_file	*old = NULL;
	FILE			*file = NULL;

	while (library_file_head != NULL && file == NULL) {
		file = fopen(library_file_head->file, "r");

		if (file == NULL) {
			msg_report(MSG_OPEN_FAIL, library_file_head->file);
			return NULL;
		}

		strncpy(library_filename_buffer, library_file_head->file, LIBRARY_MAX_FILENAME);
		library_filename = library_filename_buffer;

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


/**
 * Get the name of the last file to be opened by the library.
 *
 * \return		Pointer to the filename, or NULL if none.
 */

char *library_get_filename(void)
{
	return library_filename;
}
