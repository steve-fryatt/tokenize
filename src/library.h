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
 * \file library.h
 *
 * Library Tracking and Management Interface.
 */

#ifndef TOKENIZE_LIBRARY_H
#define TOKENIZE_LIBRARY_H

#include <stdio.h>


/**
 * Add a combined path definition to the list of library file paths. The
 * definition is in the format "name:path".
 *
 * \param *combined	The definition of the new path.
 */

void library_add_path_combined(char *combined);


/**
 * Add a path definition to the list of library file paths.
 *
 * \param *name		The name of the path.
 * \param *path		The file path.
 */

void library_add_path(char *name, char *path);


/**
 * Add a file to the list of files to be processed. The name is supplied raw, and
 * will be interpreted according to any library and system paths already defined
 *
 * \param *fild		The filename to be added to the library list.
 */

void library_add_file(char *file);


/**
 * Get the next file to be processed from the library list, as a complete
 * filename ready to be used by the file load routine.
 *
 * \param *buffer	Pointer to a buffer to take the filename.
 * \param len		The length of the supplied buffer.
 * \return		True if a filename was returned; else false.
 */

FILE *library_get_file(void);

#endif

