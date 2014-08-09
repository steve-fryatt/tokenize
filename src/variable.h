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
 * \file variable.h
 *
 * Variable analysis and substitution Interface.
 */

#ifndef TOKENIZE_VARIABLE_H
#define TOKENIZE_VARIABLE_H

#include <stdbool.h>


/**
 * Initialise the variable module.
 */

void variable_initialise(void);


/**
 * Generate a report on the variable information.
 */

void variable_report(void);


/**
 * Add a constant definition in the form of a single name=value string, such as
 * would be obtained from the command-line.
 *
 * \param *constant		Pointer to the defintion string.
 * \return			True on success; else false;
 */

bool variable_add_constant_combined(char *constant);


/**
 * Add a constant variable definition. These are pre-defined, and whenever one
 * is encountered in a program it will be replaced by its value.
 *
 * \param *name			Pointer to the variable's name.
 * \param *value		Pointer to the variable's value.
 * \return			True if successful; else false.
 */

bool variable_add_constant(char *name, char *value);


/**
 * Process a variable in the parse buffer, looking names up and handing them
 * appropriately: for constants...
 *
 * - Variables on the left-hand side are assignments, and are flagged back to
 *   the caller to be removed.
 *
 * - Variables on the right-hand side are replaced by their constant value.
 *
 * \param *name			Pointer to the start of the variable name in the output
 *				buffer.
 * \param **write		Pointer to the output buffer write pointer, which will
 *				be updated on exit.
 * \param is_array		True if the variable is an array; else False.
 * \param statement_left	True if this is an assignment; False for a read.
 * \return			True if the variable is being assigned to, else false.
 */

bool variable_process(char *name, char **write, bool is_array, bool statement_left);

#endif

