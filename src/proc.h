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
 * \file proc.h
 *
 * Function and Procedure analysis Interface.
 */

#ifndef TOKENIZE_PROC_H
#define TOKENIZE_PROC_H

#include <stdbool.h>


/**
 * Initialise the procedure module.
 */

void proc_initialise(void);


/**
 * Generate a report on the list of functions and procedures, detaining
 * missing definitions, multiple definitions and optionally unused definitions.
 *
 * \param unused	True to report unused definitions; False to ignore.
 */

void proc_report(bool unused);


/**
 * Process a function or procedure in the parse buffer, adding it to the list
 * of known routines and recording the number of calls and defintions.
 *
 * \param *name		Pointer to the start of the routine name in the output
 *			buffer.
 * \param is_function	TRUE if the routine is an FN; FALSE if it is a PROC.
 * \param is_definition	TRUE if this is a DEF; FALSE for a call.
 */

void proc_process(char *name, bool is_function, bool is_definition);

#endif

