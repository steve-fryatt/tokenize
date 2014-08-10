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
 * Assembler instruction decoder, implementation.
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
};

static char *asm_conditionals[] = {"AL", "CC", "CS", "EQ", "GE", "GT", "HI", "HS", "LE", "LO", "LS", "LT", "MI", "NE", "NV", "PL", "VC", "VS", NULL};
static char *asm_loadmsuffix[] = {"DA", "DB", "EA", "ED", "FA", "FD", "IA", "IB", NULL};
static char *asm_loadsuffix[] = {"B", NULL};

static struct asm_mnemonic_definition asm_mnemonics[] = {
	/* &0 */

	{"ADC",		KWD_NO_MATCH,	asm_conditionals,	NULL},
	{"ADD",		KWD_NO_MATCH,	asm_conditionals,	NULL},
	{"AND",		KWD_AND,	asm_conditionals,	NULL},
	{"BIC",		KWD_NO_MATCH,	asm_conditionals,	NULL},
	{"EOR",		KWD_EOR,	asm_conditionals,	NULL},
	{"ORR",		KWD_NO_MATCH,	asm_conditionals,	NULL},
	{"RSB",		KWD_NO_MATCH,	asm_conditionals,	NULL},
	{"RSC",		KWD_NO_MATCH,	asm_conditionals,	NULL},
	{"SBC",		KWD_NO_MATCH,	asm_conditionals,	NULL},
	{"SUB",		KWD_NO_MATCH,	asm_conditionals,	NULL},

	/* &1 */

	{"CMN",		KWD_NO_MATCH,	asm_conditionals,	NULL},
	{"CMP",		KWD_NO_MATCH,	asm_conditionals,	NULL},
	{"TEQ",		KWD_NO_MATCH,	asm_conditionals,	NULL},
	{"TST",		KWD_NO_MATCH,	asm_conditionals,	NULL},

	/* &2 */

	{"MOV",		KWD_NO_MATCH,	asm_conditionals,	NULL},
	{"MVN",		KWD_NO_MATCH,	asm_conditionals,	NULL},

	/* &3 */

	{"MUL",		KWD_NO_MATCH,	asm_conditionals,	NULL},
	{"MLA",		KWD_NO_MATCH,	asm_conditionals,	NULL},
	{"SML",		KWD_NO_MATCH,	asm_conditionals,	NULL},
	{"SMU",		KWD_NO_MATCH,	asm_conditionals,	NULL},
	{"UML",		KWD_NO_MATCH,	asm_conditionals,	NULL},
	{"UMU",		KWD_NO_MATCH,	asm_conditionals,	NULL},

	/* &4 */

	{"LDR",		KWD_NO_MATCH,	asm_conditionals,	asm_loadsuffix},

	/* &5 */

	{"STR",		KWD_NO_MATCH,	asm_conditionals,	asm_loadsuffix},

	/* &6 */

	{"LDM",		KWD_NO_MATCH,	asm_conditionals,	asm_loadmsuffix},

	/* &7 */

	{"STM",		KWD_NO_MATCH,	asm_conditionals,	asm_loadmsuffix},

	/* &8 */

	{"SWI",		KWD_NO_MATCH,	asm_conditionals,	NULL},

	/* &9 */

	{"CDP",		KWD_NO_MATCH,	NULL,			NULL},
	{"LDC",		KWD_NO_MATCH,	NULL,			NULL},
	{"MCR",		KWD_NO_MATCH,	NULL,			NULL},
	{"MRC",		KWD_NO_MATCH,	NULL,			NULL},
	{"MRR",		KWD_NO_MATCH,	NULL,			NULL},
	{"STC",		KWD_NO_MATCH,	NULL,			NULL},

	/* &A */

	{"MRS",		KWD_NO_MATCH,	NULL,			NULL},
	{"MSR",		KWD_NO_MATCH,	NULL,			NULL},
	{"SWP",		KWD_NO_MATCH,	NULL,			NULL},

	/* &B */

	{"LDF",		KWD_NO_MATCH,	NULL,			NULL},
	{"LFM",		KWD_NO_MATCH,	NULL,			NULL},
	{"STF",		KWD_NO_MATCH,	NULL,			NULL},
	{"SFM",		KWD_NO_MATCH,	NULL,			NULL},

	/* &C */

	{"CMF",		KWD_NO_MATCH,	NULL,			NULL},
	{"CNF",		KWD_NO_MATCH,	NULL,			NULL},
	{"FIX",		KWD_NO_MATCH,	NULL,			NULL},
	{"FLT",		KWD_NO_MATCH,	NULL,			NULL},
	{"RFC",		KWD_NO_MATCH,	NULL,			NULL},
	{"RFS",		KWD_NO_MATCH,	NULL,			NULL},
	{"WFC",		KWD_NO_MATCH,	NULL,			NULL},
	{"WFS",		KWD_NO_MATCH,	NULL,			NULL},

	/* &D */

	{"ADF",		KWD_NO_MATCH,	NULL,			NULL},
	{"DVF",		KWD_NO_MATCH,	NULL,			NULL},
	{"FDV",		KWD_NO_MATCH,	NULL,			NULL},
	{"FRD",		KWD_NO_MATCH,	NULL,			NULL},
	{"FML",		KWD_NO_MATCH,	NULL,			NULL},
	{"MUF",		KWD_NO_MATCH,	NULL,			NULL},
	{"POL",		KWD_NO_MATCH,	NULL,			NULL},
	{"POW",		KWD_NO_MATCH,	NULL,			NULL},
	{"RDF",		KWD_NO_MATCH,	NULL,			NULL},
	{"RMF",		KWD_NO_MATCH,	NULL,			NULL},
	{"RPW",		KWD_NO_MATCH,	NULL,			NULL},
	{"RSF",		KWD_NO_MATCH,	NULL,			NULL},
	{"SUF",		KWD_NO_MATCH,	NULL,			NULL},

	/* &E */

	{"ABS",		KWD_ABS,	NULL,			NULL},
	{"ACS",		KWD_ACS,	NULL,			NULL},
	{"ASN",		KWD_ASN,	NULL,			NULL},
	{"ATN",		KWD_ATN,	NULL,			NULL},
	{"COS",		KWD_COS,	NULL,			NULL},
	{"EXP",		KWD_EXP,	NULL,			NULL},
	{"LGN",		KWD_NO_MATCH,	NULL,			NULL},
	{"LOG",		KWD_LOG,	NULL,			NULL},
	{"MNF",		KWD_NO_MATCH,	NULL,			NULL},
	{"MVF",		KWD_NO_MATCH,	NULL,			NULL},
	{"NRM",		KWD_NO_MATCH,	NULL,			NULL},
	{"RND",		KWD_RND,	NULL,			NULL},
	{"SIN",		KWD_SIN,	NULL,			NULL},
	{"SQT",		KWD_NO_MATCH,	NULL,			NULL},
	{"TAN",		KWD_TAN,	NULL,			NULL},
	{"URD",		KWD_NO_MATCH,	NULL,			NULL},

	/* &F */

	{"ADR",		KWD_NO_MATCH,	NULL,			NULL},
	{"ALIGN",	KWD_NO_MATCH,	NULL,			NULL},
	{"DCB",		KWD_NO_MATCH,	NULL,			NULL},
	{"DCF",		KWD_NO_MATCH,	NULL,			NULL},
	{"DCW",		KWD_NO_MATCH,	NULL,			NULL},
	{"DCD",		KWD_NO_MATCH,	NULL,			NULL},
	{"EQUB",	KWD_NO_MATCH,	NULL,			NULL},
	{"EQUD",	KWD_NO_MATCH,	NULL,			NULL},
	{"EQUS",	KWD_NO_MATCH,	NULL,			NULL},
	{"EQUW",	KWD_NO_MATCH,	NULL,			NULL}, /* Plus FP Ones? */
	{"NOP",		KWD_NO_MATCH,	NULL,			NULL},
	{"OPT",		KWD_NO_MATCH,	NULL,			NULL},

	/* Branches (must come at end due to ambiguity. */

	{"BLX",		KWD_NO_MATCH,	asm_conditionals,	NULL},
	{"BL",		KWD_NO_MATCH,	asm_conditionals,	NULL},
	{"BX",		KWD_NO_MATCH,	asm_conditionals,	NULL},
	{"B",		KWD_NO_MATCH,	asm_conditionals,	NULL},

	/* End */

	{NULL,		KWD_NO_MATCH,	NULL}
};

enum asm_state {
	ASM_AT_START,			/**< So far, nothing has been passed to us for this statement.		*/
	ASM_FOUND_LABEL,		/**< We've found a label declaration.					*/
	ASM_FOUND_TOKEN,
	ASM_FOUND_MNEMONIC,
	ASM_EXTRA_MNEMONIC,
	ASM_FOUND_OR,
	ASM_FOUND_MOVE
};

static enum asm_state		asm_current_state;
static enum asm_mnemonic	asm_current_mnemonic;


static char*	asm_match_list(char **list, char *text);



void asm_new_statement(void)
{
	asm_current_state = ASM_AT_START;
	asm_current_mnemonic = MNM_NO_MATCH;
}


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

	printf("Found keyword as token: %s\n", asm_mnemonics[entry].name);
}


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
			asm_current_state = ASM_FOUND_MNEMONIC;
		else
			;/* \TODO -- Move on to parameters! */
	} else if ((asm_current_state == ASM_FOUND_OR) && (*(*text - 1) == ((char) parse_get_token(KWD_OR)))) {
		printf("Special case for OR\n");
	
		if (toupper(**text) == 'R') {
			printf("Found keyword as OR + R: %s in %s\n", asm_mnemonics[MNM_ORR].name, *text);
			*text += 1;
			asm_current_state = ASM_FOUND_MNEMONIC;
			asm_current_mnemonic = MNM_ORR;
		}
	} else if (asm_current_state == ASM_AT_START || asm_current_state == ASM_FOUND_LABEL) {
		int			i;
		enum asm_mnemonic	entry = 0;
		bool			found = false;

		while (asm_mnemonics[entry].name != NULL && !found) {
			for (i = 0; asm_mnemonics[entry].name[i] != '\0' && (*text)[i] != '\0' && asm_mnemonics[entry].name[i] == toupper((*text)[i]); i++);

			if (asm_mnemonics[entry].name[i] == '\0')
				found = true;
			else
				entry++;
		}

		if (found == true) {
			printf("Found keyword as text: %s in %s\n", asm_mnemonics[entry].name, *text);
			*text += strlen(asm_mnemonics[entry].name);
			asm_current_state = ASM_FOUND_MNEMONIC;
			asm_current_mnemonic = entry;
		}
	}
	
	if (asm_current_state == ASM_FOUND_MNEMONIC && asm_current_mnemonic != MNM_NO_MATCH &&
			**text != '\0' && asm_mnemonics[asm_current_mnemonic].conditionals != NULL) {
		char	*conditional = NULL;

		if ((conditional = asm_match_list(asm_mnemonics[asm_current_mnemonic].conditionals, *text)) != NULL) {
			printf("Found conditional as text: %s in %s\n", conditional, *text);
			*text += strlen(conditional);
			asm_current_state = ASM_FOUND_MNEMONIC;
		}
	}
	
	if (asm_current_state == ASM_FOUND_MNEMONIC && asm_current_mnemonic != MNM_NO_MATCH &&
			**text != '\0' && asm_mnemonics[asm_current_mnemonic].suffixes != NULL) {
		char	*suffix = NULL;

		if ((suffix = asm_match_list(asm_mnemonics[asm_current_mnemonic].suffixes, *text)) != NULL) {
			printf("Found suffix as text: %s in %s\n", suffix, *text);
			*text += strlen(suffix);
			asm_current_state = ASM_FOUND_MNEMONIC;
		}
	}
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

void asm_process_comma(void)
{

}

