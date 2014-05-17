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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Local source headers. */

#include "swi.h"

/* OSLib source headers. */

#ifdef RISCOS
#include "oslib/os.h"
#endif


/**
 * The maximum length of a line in the definitions header file.
 */

#define SWI_MAX_LINE_LENGTH 1024

#define SWI_X_BIT 0x20000
#define SWI_USED_BITS 0xfffff

/**
 * Structure to hold a SWI definition.
 */

struct swi {
	char			*name;		/**< The SWI name following the _		*/
	long			number;		/**< The absolute SWI number.			*/

	struct swi		*next;		/**< The next SWI in the group, or NULL.	*/
};


/**
 * Structure to hold a SWI chunk definition.
 */

struct swi_chunk {
	char			*name;		/**< The chunk name before the _		*/
	long			base;		/**< The base address if the SWI chunk.		*/

	struct swi		*swis;		/**< The list of SWIs in this chunk.		*/

	struct swi_chunk	*next;		/**< The next chunk in the list, or NULL.	*/
};


/**
 * List of SWI chunks that we know about.
 */

static struct swi_chunk		*swi_chunk_list = NULL;


/**
 * Function Definitions
 */

#ifdef RISCOS
static long		swi_os_lookup(char *name);
#endif
static bool		swi_add_definition(char *chunk_name, char *swi_name, long number);
static struct swi_chunk	*swi_find_chunk(char *name);
static struct swi	*swi_find_swi(struct swi_chunk *chunk, char *name);
static char		*swi_find_next_text_item(char **line, char *line_end);


/**
 * Look up a SWI name, returning its number if a match is found. On Linux
 * we do this using our own tables of SWI names; on RISC OS, we do it using
 * OS_SWINumberFromString.
 *
 * \param *name		The possible SWI name to look up.
 * \return		The SWI number, or -1 if not found.
 */

long swi_get_number_from_name(char *name)
{
	char			*copy, *chunk_name, *swi_name;
	struct swi_chunk	*chunk;
	struct swi		*swi;
	bool			xswi = false;


#ifdef RISCOS
	/* On RISC OS, if we have no SWI definitions of our own, we defer
	 * the the OS to do the lookup for us. If we do have our own
	 * definitions, then we assume that the user wishes to use these
	 * instead of the OS.
	 */

	if (swi_chunk_list == NULL)
		return swi_os_lookup(name);
#endif

	copy = strdup(name);
	if (copy == NULL)
		return -1;

	chunk_name = strtok(copy, "_");
	swi_name = strtok(NULL, "_");

	/* If this is an X SWI, look up the non-X version and add in
	 * the X-bit when we return the SWI number.
	 */

	if (chunk_name != NULL && *chunk_name == 'X') {
		chunk_name++;
		xswi = true;
	}

	chunk = swi_find_chunk(chunk_name);
	swi = swi_find_swi(chunk, swi_name);

	free(copy);

	if (swi == NULL)
		return -1;

	return (xswi == true) ? (swi->number | SWI_X_BIT) : swi->number;
}


#ifdef RISCOS
/**
 * Look up a SWI name natively on RISC OS, using OS_SWINumberFromString.
 *
 * \param *name		The possible SWI name to look up.
 * \return		The SWI number, or -1 if not found.
 */

static long swi_os_lookup(char *name)
{
	os_error	*error;
	long		number;

	error = xos_swi_number_from_string(name, (int *) &number);

	if (error != NULL)
		return -1;

	return number;
}
#endif


/**
 * Add the contents of a header file to the list of known
 * SWI names and numbers.
 *
 * \param *file		The file to be added.
 * \return		True if successful; False on error.
 */

bool swi_add_header_file(char *file)
{
	FILE	*header;
	char	line[SWI_MAX_LINE_LENGTH], *line_end;
	char	*p, *define, *swi, *number, *block, *name;
	long	swi_number;


	if (file == NULL)
		return false;

	header = fopen(file, "r");
	if (header == NULL)
		return false;

	line_end = line + SWI_MAX_LINE_LENGTH;

	while (fgets(line, SWI_MAX_LINE_LENGTH, header) != NULL) {
		p = line;

		/* Find the #define; if this isn't one, jump to the next
		 * line.
		 */

		define = swi_find_next_text_item(&p, line_end);

		if (define == NULL || strcmp(define, "#define") != 0)
			continue;

		/* Find the possible SWI name. */

		swi = swi_find_next_text_item(&p, line_end);

		if (swi == NULL)
			continue;

		/* Find the possible SWI number. */

		number = swi_find_next_text_item(&p, line_end);

		if (number == NULL || *number == '\0')
			continue;

		/* Break the SWI name into block and name. */

		block = strtok(swi, "_");
		name = strtok(NULL, "_");

		if (block == NULL || name == NULL)
			continue;

		swi_number = strtol(number, &p, 0);

		if (p == NULL || *p != '\0')
			continue;

		/* RISC OS SWIs should only have the bits 0-19 set. If only the
		 * error bit is set, this isn't a valid SWI either.
		 */

		if (swi_number > SWI_USED_BITS || swi_number == SWI_X_BIT)
			continue;

		if (!swi_add_definition(block, name, swi_number))
			return false;
	}

	fclose(header);

	return true;
}


/**
 * Add a SWI definition to the list of known SWIs, creating the necessary
 * data blocks. X versions of SWIs are converted into their non-X variants.
 *
 * \param *chunk_name	The SWI's chunk name, up to the _ character.
 * \param *swi_name	The SWI's name, after the _ character.
 * \param number	The SWI's number.
 * \return		True if the addition was successful; False on error.
 */

static bool swi_add_definition(char *chunk_name, char *swi_name, long number)
{
	struct swi_chunk	*chunk;
	struct swi		*swi;

	if (chunk_name == NULL || swi_name == NULL)
		return false;

	/* If this is an X SWI (the name starts 'X' and the X-bit is set in
	 * the SWI number), convert it to a non-X variant as there's no point
	 * storing both.
	 */

	if (*chunk_name == 'X' && (number & SWI_X_BIT) == SWI_X_BIT) {
		chunk_name++;
		number &= ~SWI_X_BIT;
	}

	/* See if we already know about the SWI's chunk, and create a new
	 * record if we don't.
	 */

	chunk = swi_find_chunk(chunk_name);

	if (chunk == NULL) {
		chunk = malloc(sizeof(struct swi_chunk));
		
		if (chunk == NULL)
			return false;

		chunk->name = strdup(chunk_name);
		chunk->base = 0;
		chunk->swis = NULL;

		chunk->next = swi_chunk_list;
		swi_chunk_list = chunk;
	}

	/* See if we already know about the SWI within its chunk, and create
	 * a new record if we don't.
	 */

	swi = swi_find_swi(chunk, swi_name);

	if (swi == NULL) {
		swi = malloc(sizeof(struct swi));
		
		if (swi == NULL)
			return false;

		swi->name = strdup(swi_name);
		swi->number = number;

		swi->next = chunk->swis;
		chunk->swis = swi;
	}

	return true;
}


/**
 * Find a SWI chunk definition based on its name.
 *
 * \param *name		The name of the chunk to find, up to the _.
 * \return		Pointer to the SWI chunk data, or NULL if not found.
 */

static struct swi_chunk *swi_find_chunk(char *name)
{
	struct swi_chunk	*chunk = swi_chunk_list;

	if (name == NULL)
		return NULL;

	while (chunk != NULL && chunk->name != NULL && strcmp(chunk->name, name) != 0)
		chunk = chunk->next;

	return chunk;
}


/**
 * Find a SWI definition from within a chunk, based on its name.
 *
 * \param *chunk	Pointer to the SWI chunk to search.
 * \param *name		The name of the SWI to find, after the _.
 * \return		Pointer to the SWI data, or NULL if not found.
 */

static struct swi *swi_find_swi(struct swi_chunk *chunk, char *name)
{
	struct swi	*swi;

	if (chunk == NULL || name == NULL)
		return NULL;

	swi = chunk->swis;

	while (swi != NULL && swi->name != NULL && strcmp(swi->name, name) != 0)
		swi = swi->next;

	return swi;
}


/**
 * Locate and terminate the next contiguous block of text in a line.
 *
 * \param **line	Pointer to the line pointer; updated on exit to be
 *			after the new string terminator.
 * \param *line_end	Pointer to the end of the line buffer.
 * \return		Pointer to a text block, or NULL if not found.
 */

static char *swi_find_next_text_item(char **line, char *line_end)
{
	char	*position = NULL;
	
	while (*line < line_end && **line != '\0' && **line != '\r' && **line != '\n' && isspace(**line))
		(*line)++;
	
	if (*line >= line_end)
		return NULL;
	
	position = *line;
	
	while (*line < line_end && **line != '\0' && **line != '\r' && **line != '\n' && !isspace(**line))
		(*line)++;
	
	if (*line >= line_end)
		return NULL;

	*(*line)++ = '\0';

	return position;
}

