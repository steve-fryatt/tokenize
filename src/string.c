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
 * \file string.c
 *
 * String Utilities, implementation.
 */

#include <ctype.h>

/* Local source headers. */

#include "string.h"


/* Perform a strcmp() case-insensitively on two strings, returning
 * a value less than, equal to or greater than zero depending on
 * their relative values.
 *
 * This is an external interface, documented in string.h
 */

int string_nocase_strcmp(char *s1, char *s2)
{
	while (*s1 != '\0' && *s2 != '\0' && (toupper(*s1) - toupper(*s2)) == 0) {
		s1++;
		s2++;
	}

	return (toupper(*s1) - toupper(*s2));
}

