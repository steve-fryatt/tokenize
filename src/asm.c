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
 * \file asm.c
 *
 * Assembler instruction identifier, implementation.
 */

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Local source headers. */

#include "asm.h"

#include "parse.h"

enum asm_mnemonic {
	MNM_NO_MATCH = -1,
	MNM_ADC,
	MNM_ADD,
	MNM_AND,
	MNM_BIC,
	MNM_EOR,
	MNM_ORR,
	MNM_RSB,
	MNM_RSC,
	MNM_SBC,
	MNM_SUB,
	MNM_CMN,
	MNM_CMP,
	MNM_TEQ,
	MNM_TST,
	MNM_MOV,
	MNM_MVN
};

struct asm_mnemonic_definition {
	char			*name;			/**< The name of the mnemonic.					*/
	enum parse_keyword	keyword;		/**< The keyword whose token can replace the text.		*/
	char			**conditionals;		/**< A list of possible conditional siffixes.			*/
	char			**suffixes;		/**< A list of possible suffixes to follow the conditional.	*/
	char			***parameters;		/**< A list of parameter lists.					*/
};

static char *asm_conditionals[] = {"AL", "CC", "CS", "EQ", "GE", "GT", "HI", "HS", "LE", "LO", "LS", "LT", "MI", "NE", "NV", "PL", "VC", "VS", NULL};
static char *asm_shifts[] = {"ASL", "ASR", "LSL", "LSR", "ROR", "RRX", "R15", "R14", "R13", "R12", "R11", "R10", "R9", "R8", "R7", "R6", "R5", "R4", "R3", "R2", "R1", "R0", "PC", "LR", "SP", NULL};
static char *asm_registers[] = {"R15", "R14", "R13", "R12", "R11", "R10", "R9", "R8", "R7", "R6", "R5", "R4", "R3", "R2", "R1", "R0", "PC", "LR", "SP", NULL};
static char *asm_copros[] = {"CP0", "CP1", "CP2", "CP3", "CP4", "CP5", "CP6", "CP7", "CP8", "CP9", "CP10", "CP11", "CP12", "CP13", "CP14", "CP15", NULL};
static char *asm_coproreg[] = {"C15", "C14", "C13", "C12", "C11", "C10", "C9", "C8", "C7", "C6", "C5", "C4", "C3", "C2", "C1", "C0", NULL};
static char *asm_movsuffix[] = {"S", NULL};
static char *asm_cmpsuffix[] = {"SP", "S", "P", NULL};
static char *asm_loadmsuffix[] = {"DA", "DB", "EA", "ED", "FA", "FD", "IA", "IB", NULL};
static char *asm_loadsuffix[] = {"BT", "SB", "SH", "B", "H", "T", NULL};

static char **asm_none[] = {NULL};
static char **asm_two[] = {asm_registers, asm_shifts, NULL};
static char **asm_three[] = {asm_registers, asm_registers, asm_shifts, NULL};
static char **asm_four[] = {asm_registers, asm_registers, asm_registers, asm_shifts, NULL};
static char **asm_loadm[] = {asm_registers, asm_registers, asm_registers, asm_registers,
		asm_registers, asm_registers, asm_registers, asm_registers, asm_registers,
		asm_registers, asm_registers, asm_registers, asm_registers, asm_registers,
		asm_registers, asm_registers, asm_registers, NULL};
static char **asm_multhree[] = {asm_registers, asm_registers, asm_registers, NULL};
static char **asm_mulfour[] = {asm_registers, asm_registers, asm_registers, asm_registers, NULL};
static char **asm_param_copro[] = {asm_copros, asm_none, asm_coproreg, asm_coproreg, asm_coproreg, asm_none, NULL};

static struct asm_mnemonic_definition asm_mnemonics[] = {
	/* &0 */

	{"ADC",		KWD_NO_MATCH,	asm_conditionals,	asm_movsuffix,		asm_three},
	{"ADD",		KWD_NO_MATCH,	asm_conditionals,	asm_movsuffix,		asm_three},
	{"AND",		KWD_AND,	asm_conditionals,	asm_movsuffix,		asm_three},
	{"BIC",		KWD_NO_MATCH,	asm_conditionals,	asm_movsuffix,		asm_three},
	{"EOR",		KWD_EOR,	asm_conditionals,	asm_movsuffix,		asm_three},
	{"ORR",		KWD_NO_MATCH,	asm_conditionals,	asm_movsuffix,		asm_three},
	{"RSB",		KWD_NO_MATCH,	asm_conditionals,	asm_movsuffix,		asm_three},
	{"RSC",		KWD_NO_MATCH,	asm_conditionals,	asm_movsuffix,		asm_three},
	{"SBC",		KWD_NO_MATCH,	asm_conditionals,	asm_movsuffix,		asm_three},
	{"SUB",		KWD_NO_MATCH,	asm_conditionals,	asm_movsuffix,		asm_three},

	/* &1 */

	{"CMN",		KWD_NO_MATCH,	asm_conditionals,	asm_cmpsuffix,		asm_two},
	{"CMP",		KWD_NO_MATCH,	asm_conditionals,	asm_cmpsuffix,		asm_two},
	{"TEQ",		KWD_NO_MATCH,	asm_conditionals,	asm_cmpsuffix,		asm_two},
	{"TST",		KWD_NO_MATCH,	asm_conditionals,	asm_cmpsuffix,		asm_two},

	/* &2 */

	{"MOV",		KWD_NO_MATCH,	asm_conditionals,	asm_movsuffix,		asm_two},
	{"MVN",		KWD_NO_MATCH,	asm_conditionals,	asm_movsuffix,		asm_two},

	/* &3 */

	{"MUL",		KWD_NO_MATCH,	asm_conditionals,	asm_movsuffix,		asm_multhree},
	{"MLA",		KWD_NO_MATCH,	asm_conditionals,	asm_movsuffix,		asm_mulfour},
	{"SMLAL",	KWD_NO_MATCH,	asm_conditionals,	asm_movsuffix,		asm_mulfour},
	{"SMUAL",	KWD_NO_MATCH,	asm_conditionals,	asm_movsuffix,		asm_mulfour},
	{"UMLAL",	KWD_NO_MATCH,	asm_conditionals,	asm_movsuffix,		asm_mulfour},
	{"UMULL",	KWD_NO_MATCH,	asm_conditionals,	asm_movsuffix,		asm_mulfour},
	{"SMULB",	KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_multhree},
	{"SMULTB",	KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_multhree},
	{"SMULWB",	KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_multhree},
	{"SMULWTB",	KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_multhree},
	{"SMLAB",	KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_mulfour},
	{"SMLATB",	KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_mulfour},
	{"SMLAWB",	KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_mulfour},
	{"SMLAWTB",	KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_mulfour},

	/* &4 */

	{"LDR",		KWD_NO_MATCH,	asm_conditionals,	asm_loadsuffix,		asm_three},
	{"LDRD",	KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_four},

	/* &5 */

	{"STR",		KWD_NO_MATCH,	asm_conditionals,	asm_loadsuffix,		asm_three},
	{"STRD",	KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_four},

	/* &6 */

	{"LDM",		KWD_NO_MATCH,	asm_conditionals,	asm_loadmsuffix,	asm_loadm},

	/* &7 */

	{"STM",		KWD_NO_MATCH,	asm_conditionals,	asm_loadmsuffix,	asm_loadm},

	/* &8 */

	{"SWI",		KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_none},

	/* &9 */

	{"CDP",		KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_param_copro},
	{"CDP2",	KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_param_copro}, /* Done this far! */
	{"LDC",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},
	{"MCR",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},
	{"MRC",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},
	{"MRR",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},
	{"STC",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},

	/* &A */

	{"MRS",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},
	{"MSR",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},
	{"SWP",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},

	/* &B */

	{"LDF",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},
	{"LFM",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},
	{"STF",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},
	{"SFM",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},

	/* &C */

	{"CMF",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},
	{"CNF",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},
	{"FIX",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},
	{"FLT",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},
	{"RFC",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},
	{"RFS",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},
	{"WFC",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},
	{"WFS",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},

	/* &D */

	{"ADF",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},
	{"DVF",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},
	{"FDV",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},
	{"FRD",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},
	{"FML",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},
	{"MUF",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},
	{"POL",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},
	{"POW",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},
	{"RDF",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},
	{"RMF",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},
	{"RPW",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},
	{"RSF",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},
	{"SUF",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},

	/* &E */

	{"ABS",		KWD_ABS,	NULL,			NULL,			asm_none},
	{"ACS",		KWD_ACS,	NULL,			NULL,			asm_none},
	{"ASN",		KWD_ASN,	NULL,			NULL,			asm_none},
	{"ATN",		KWD_ATN,	NULL,			NULL,			asm_none},
	{"COS",		KWD_COS,	NULL,			NULL,			asm_none},
	{"EXP",		KWD_EXP,	NULL,			NULL,			asm_none},
	{"LGN",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},
	{"LOG",		KWD_LOG,	NULL,			NULL,			asm_none},
	{"MNF",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},
	{"MVF",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},
	{"NRM",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},
	{"RND",		KWD_RND,	NULL,			NULL,			asm_none},
	{"SIN",		KWD_SIN,	NULL,			NULL,			asm_none},
	{"SQT",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},
	{"TAN",		KWD_TAN,	NULL,			NULL,			asm_none},
	{"URD",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},

	/* &F */

	{"ADR",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},
	{"ALIGN",	KWD_NO_MATCH,	NULL,			NULL,			asm_none},
	{"DCB",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},
	{"DCF",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},
	{"DCW",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},
	{"DCD",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},
	{"EQUB",	KWD_NO_MATCH,	NULL,			NULL,			asm_none},
	{"EQUD",	KWD_NO_MATCH,	NULL,			NULL,			asm_none},
	{"EQUS",	KWD_NO_MATCH,	NULL,			NULL,			asm_none},
	{"EQUW",	KWD_NO_MATCH,	NULL,			NULL,			asm_none}, /* Plus FP Ones? */
	{"NOP",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},
	{"OPT",		KWD_NO_MATCH,	NULL,			NULL,			asm_none},

	/* Branches (must come at end due to ambiguity. */

	{"BLX",		KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_none},
	{"BL",		KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_none},
	{"BX",		KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_none},
	{"B",		KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_none},

	/* End */

	{NULL,		KWD_NO_MATCH,	NULL}
};

enum asm_state {
	ASM_AT_START,			/**< So far, nothing has been passed to us for this statement.		*/
	ASM_FOUND_LABEL,		/**< We've found a label declaration.					*/
	ASM_FOUND_TOKEN,		/**< We've found a token than looks like a valid instruction.		*/
	ASM_FOUND_OR,			/**< We've found an OR token, which is treated as part of ORR.		*/
	ASM_FOUND_MOVE,			/**< We've found a MOVE token, which is treated as part of MOVEQ.	*/
	ASM_TEST_CONDITIONAL,		/**< We've found an instruction, so look for any conditional code.	*/
	ASM_TEST_SUFFIX,		/**< We've found an instruction, so look for any valid suffix.		*/
	ASM_TEST_PARAMETERS,		/**< We've found the main instruction body, so now look for the params.	*/
	ASM_EXTRA_MNEMONIC
};

static enum asm_state		asm_current_state;
static char			***asm_current_parameter;
static enum asm_mnemonic	asm_current_mnemonic;


static char*	asm_match_list(char **list, char *text);


/**
 * Start a new assembler statement, resetting all of the tracking information.
 */

void asm_new_statement(void)
{
	asm_current_state = ASM_AT_START;
	asm_current_mnemonic = MNM_NO_MATCH;
	asm_current_parameter = NULL;
}


/**
 * Process a tokenised keyword within the assembler, looking for keywords that
 * can form part of an assembler instruction.
 *
 * \param keyword	The keyword to be processed.
 */

void asm_process_keyword(enum parse_keyword keyword)
{
	enum asm_mnemonic	entry = 0;

	/* Process two special-case tokens which form part of assembler mnemonics
	 * and will require further special processing when we subsequently get
	 * sent a variable to check.
	 */

	if ((asm_current_state == ASM_AT_START || asm_current_state == ASM_FOUND_LABEL) && keyword == KWD_OR) {
		asm_current_state = ASM_FOUND_OR;
		printf("Found OR as token\n");
		return;
	} else if ((asm_current_state == ASM_AT_START || asm_current_state == ASM_FOUND_LABEL) && keyword == KWD_MOVE) {
		asm_current_state = ASM_FOUND_MOVE;
		printf("Found MOVE as token\n");
		return;
	}

	/* Otherwise, we're looking for whole tokenised keywords. */

	while (asm_mnemonics[entry].name != NULL && asm_mnemonics[entry].keyword != keyword)
		entry++;

	if (asm_mnemonics[entry].name == NULL)
		return;

	if (asm_current_state != ASM_AT_START && asm_current_state != ASM_FOUND_LABEL) {
		asm_current_state = ASM_EXTRA_MNEMONIC;
		printf("Found unexpected keyword as token: %s\n", asm_mnemonics[entry].name);
		return;
	}

	asm_current_state = ASM_FOUND_TOKEN;
	asm_current_mnemonic = entry;
	asm_current_parameter = asm_mnemonics[entry].parameters;

	printf("Found keyword as token: %s\n", asm_mnemonics[entry].name);
}


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

void asm_process_variable(char **text)
{
	/* If this is the start of a statement, and the previous character
	 * is a dot, then what follows must be a variable name.
	 */

	if (asm_current_state == ASM_AT_START && *(*text - 1) == '.') {
		asm_current_state = ASM_FOUND_LABEL;
		return;
	}

	if (asm_current_state == ASM_FOUND_TOKEN) {
		if (*(*text - 1) == (char) parse_get_token(asm_mnemonics[asm_current_mnemonic].keyword))
			asm_current_state = ASM_TEST_CONDITIONAL;
		else
			asm_current_state = ASM_TEST_PARAMETERS;
	} else if ((asm_current_state == ASM_FOUND_OR) && (*(*text - 1) == ((char) parse_get_token(KWD_OR)))) {
		printf("Special case for OR\n");
	
		if (toupper(**text) == 'R') {
			printf("Found keyword as OR + R: %s in %s\n", asm_mnemonics[MNM_ORR].name, *text);
			*text += 1;
			asm_current_state = ASM_TEST_CONDITIONAL;
			asm_current_mnemonic = MNM_ORR;
			asm_current_parameter = asm_mnemonics[MNM_ORR].parameters;
		}
	} else if ((asm_current_state == ASM_FOUND_MOVE) && (*(*text - 1) == ((char) parse_get_token(KWD_MOVE)))) {
		printf("Special case for MOVE\n");
	
		if (toupper(**text) == 'Q') {
			printf("Found keyword as MOVE + Q: %s in %s\n", asm_mnemonics[MNM_ORR].name, *text);
			*text += 1;
			asm_current_state = ASM_TEST_PARAMETERS;
			asm_current_mnemonic = MNM_MOV;
			asm_current_parameter = asm_mnemonics[MNM_MOV].parameters;
		}
	} else if (asm_current_state == ASM_AT_START || asm_current_state == ASM_FOUND_LABEL) {
		int			i, longest = 0;
		enum asm_mnemonic	entry = 0, found = MNM_NO_MATCH;

		while (asm_mnemonics[entry].name != NULL) {
			for (i = 0; asm_mnemonics[entry].name[i] != '\0' && (*text)[i] != '\0' && asm_mnemonics[entry].name[i] == toupper((*text)[i]); i++);

			if (asm_mnemonics[entry].name[i] == '\0' && i > longest) {
				found = entry;
				longest = i;
			} else {
				entry++;
			}
		}

		if (found != MNM_NO_MATCH) {
			printf("Found keyword as text: %s in %s\n", asm_mnemonics[found].name, *text);
			*text += strlen(asm_mnemonics[found].name);
			asm_current_state = ASM_TEST_CONDITIONAL;
			asm_current_mnemonic = found;
			asm_current_parameter = asm_mnemonics[found].parameters;
		}
	}
	
	if (asm_current_state == ASM_TEST_CONDITIONAL && asm_current_mnemonic != MNM_NO_MATCH) {
		printf("Testing conditionals...\n");
		if (asm_mnemonics[asm_current_mnemonic].conditionals == NULL) {
			asm_current_state = ASM_TEST_SUFFIX;
		} else if (**text == '\0') {
			asm_current_state = ASM_TEST_PARAMETERS;
		} else {
			char	*conditional = NULL;

			if ((conditional = asm_match_list(asm_mnemonics[asm_current_mnemonic].conditionals, *text)) != NULL) {
				printf("Found conditional as text: %s in %s\n", conditional, *text);
				*text += strlen(conditional);
			}

			asm_current_state = ASM_TEST_SUFFIX;
		}
	}

	if (asm_current_state == ASM_TEST_SUFFIX && asm_current_mnemonic != MNM_NO_MATCH) {
		printf("Testing suffixes...\n");
		if (asm_mnemonics[asm_current_mnemonic].suffixes == NULL || **text == '\0') {
			asm_current_state = ASM_TEST_PARAMETERS;
		} else {
			char	*suffix = NULL;

			if ((suffix = asm_match_list(asm_mnemonics[asm_current_mnemonic].suffixes, *text)) != NULL) {
				printf("Found suffix as text: %s in %s\n", suffix, *text);
				*text += strlen(suffix);
			}

			asm_current_state = ASM_TEST_PARAMETERS;
		}
	}

	/* Process the the current parameter, using the appropriate list of
	 * valid assembler mnemonics. The parameter is stepped on each time
	 * a comma is seen by the parser.
	 *
	 * If a parameter list contains no items, that parameter can't contain
	 * anything that might be construed as a variable name by the tokeniser.
	 */

	if (asm_current_state == ASM_TEST_PARAMETERS && asm_current_mnemonic != MNM_NO_MATCH
			&& asm_current_parameter != NULL && *asm_current_parameter != NULL) {
		char	*param = NULL;

		while ((param = asm_match_list(*asm_current_parameter, *text)) != NULL) {
			printf("Found parameter as text: %s in %s\n", param, *text);
			*text += strlen(param);
		}
	}
}


/**
 * Process a comma in an assembler statement. This moves on a parameter
 * in the parameter list if there's a command active.
 */

void asm_process_comma(void)
{
	if (asm_current_parameter != NULL && *asm_current_parameter != NULL)
		asm_current_parameter++;
}


/**
 * Case insensitively match the first characters in a string with a NULL
 * terminated list of possible matches (in uppercase).
 *
 * \param **list	Array of pointers to strings to test against.
 * \param *text		The text to be matched.
 * \return		The matched array entry, or NULL if none found.
 */

static char *asm_match_list(char **list, char *text)
{
	int	entry = 0, i = 0;
	bool	found = false;

	while (list[entry] != NULL && !found) {
		for (i = 0; list[entry][i] != '\0' && text[i] != '\0' && list[entry][i] == toupper(text[i]); i++);

		if (list[entry][i] == '\0')
			break;

		entry++;
	}

	return list[entry];
}

