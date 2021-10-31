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
 * \file swi.h
 *
 * SWI name to number conversion Interface.
 */

#ifndef TOKENIZE_SWI_H
#define TOKENIZE_SWI_H

#include <stdbool.h>

/**
 * Look up a SWI name, returning its number if a match is found.
 *
 * \param *name		The possible SWI name to look up.
 * \return		The SWI number, or -1 if not found.
 */

long swi_get_number_from_name(char *name);


/**
 * Add the contents of a header file to the list of known
 * SWI names and numbers.
 *
 * \param *file		The file to be added.
 * \return		True if successful; False on error.
 */

bool swi_add_header_file(char *file);

#endif

