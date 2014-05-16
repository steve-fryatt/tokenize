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
 * \file swi.c
 *
 * SWI name to number conversion, implementation.
 */

#include <ctype.h>
#include <stdio.h>

/* Local source headers. */

#include "swi.h"

/* OSLib source headers. */

#ifdef RISCOS
#include "oslib/os.h"
#endif

static int	swi_os_lookup(char *name);

/**
 * Look up a SWI name, returning its number if a match is found.
 *
 * \param *name		The possible SWI name to look up.
 * \return		The SWI number, or -1 if not found.
 */

int swi_get_number_from_name(char *name)
{
	printf("Lookup SWI name: '%s'\n", name);

#ifdef RISCOS
	return swi_os_lookup(name);
#endif

	return -1;
}


#ifdef RISCOS
/**
 * Look up a SWI name natively on RISC OS, using OS_SWINumberFromString.
 *
 * \param *name		The possible SWI name to look up.
 * \return		The SWI number, or -1 if not found.
 */

static int swi_os_lookup(char *name)
{
	os_error	*error;
	int		number;

	error = xos_swi_number_from_string(name, &number);

	if (error != NULL)
		return -1;

	return number;
}
#endif

