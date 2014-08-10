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
 * \file asm.h
 *
 * Assembler instruction decoder, interface.
 */

#ifndef TOKENIZE_ASM_H
#define TOKENIZE_ASM_H

#include <stdbool.h>

#include "parse.h"

void asm_new_statement(void);

void asm_process_keyword(enum parse_keyword keyword);

void asm_process_variable(char **text);

void asm_process_comma(void);

#endif

