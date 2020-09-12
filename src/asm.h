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
 * \file asm.h
 *
 * Assembler instruction identifier, interface.
 */

#ifndef TOKENIZE_ASM_H
#define TOKENIZE_ASM_H

#include <stdbool.h>

#include "parse.h"

/**
 * Start a new assembler statement, resetting all of the tracking information.
 */

void asm_new_statement(void);


/**
 * Process a tokenised keyword within the assembler, looking for keywords that
 * can form part of an assembler instruction.
 *
 * \param keyword	The keyword to be processed.
 */

void asm_process_keyword(enum parse_keyword keyword);


/**
 * Process a zero-terminated string of text in an assembler section that the
 * tokeniser thinks is a variable name. If any of it looks like something that
 * could be part of an assembler instruction, move the pointer on to point to
 * the first unrecognised byte.
 *
 * \param **text	Pointer to a pointer to the possible variable name in
 *			the tokeniser output buffer; this is updated on exit
 *			to point to the first unrecognised character.
 */

void asm_process_variable(char **text);


/**
 * Process a comma in an assembler statement. This moves on a parameter
 * in the parameter list if there's a command active.
 */

void asm_process_comma(void);

#endif

