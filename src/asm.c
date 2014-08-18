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

/**
 * A list of indexes for the known assembler mnemonics in the definition
 * list. The order of this list *MUST* match the order of the definitions
 * below.
 */

enum asm_mnemonic {
	MNM_NO_MATCH = -1,

/* &0 */

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

/* &1 */

	MNM_CMN,
	MNM_CMP,
	MNM_TEQ,
	MNM_TST,

/* &2 */

	MNM_MOV,
	MNM_MVN,

/* &3 */

	MNM_MUL,
	MNM_MLA,
	MNM_SMLAL,
	MNM_SMUAL,
	MNM_UMLAL,
	MNM_UMULL,
	MNM_SMULB,
	MNM_SMULTB,
	MNM_SMULWB,
	MNM_SMULWTB,
	MNM_SMLAB,
	MNM_SMLATB,
	MNM_SMLAWB,
	MNM_SMLAWTB,

/* &4 */

	MNM_LDR,
	MNM_LDRD,

/* &5 */

	MNM_STR,
	MNM_STRD,

/* &6 */

	MNM_LDM,

/* &7 */

	MNM_STM,

/* &8 */

	MNM_SWI,

/* &9 */

	MNM_CDP,
	MNM_CDP2,
	MNM_LDC,
	MNM_LDC2,
	MNM_MCR,
	MNM_MCR2,
	MNM_MRC,
	MNM_MRC2,
	MNM_MCRR,
	MNM_MRRC,
	MNM_STC,
	MNM_STC2,

/* &A */

	MNM_MRS,
	MNM_MSR,
	MNM_SWP,

/* &B */

	MNM_LDF,
	MNM_LFM,
	MNM_STF,
	MNM_SFM,

/* &C */

	MNM_CMF,
	MNM_CNF,
	MNM_FIX,
	MNM_FLT,
	MNM_RFC,
	MNM_RFS,
	MNM_WFC,
	MNM_WFS,

/* &D */

	MNM_ADF,
	MNM_DVF,
	MNM_FDV,
	MNM_FML,
	MNM_FRD,
	MNM_MUF,
	MNM_POL,
	MNM_POW,
	MNM_RDF,
	MNM_RMF,
	MNM_RPW,
	MNM_RSF,
	MNM_SUF,

/* &E */

	MNM_ABS,
	MNM_ACS,
	MNM_ASN,
	MNM_ATN,
	MNM_COS,
	MNM_EXP,
	MNM_LGN,
	MNM_LOF,
	MNM_MNF,
	MNM_MVF,
	MNM_NRM,
	MNM_RND,
	MNM_SIN,
	MNM_SQT,
	MNM_TAN,
	MNM_URD,

/* Other Stuff. */

	MNM_QADD,
	MNM_QSUB,
	MNM_QDADD,
	MNM_QDSUB,
	MNM_CLZ,
	MNM_BKPT,
	MNM_PLD,

/* Branches */

	MNM_BLX,
	MNM_BL,
	MNM_BX,
	MNM_B,

/* &F (Directives) */

	MNM_ADR,
	MNM_ALIGN,
	MNM_DCB,
	MNM_DCF,
	MNM_DCW,
	MNM_DCD,
	MNM_EQUB,
	MNM_EQUD,
	MNM_EQUF,
	MNM_EQUS,
	MNM_EQUW,
	MNM_NOP,
	MNM_OPT,

/* End Of List */

	MAX_MNEMONICS
};


/**
 * The mnemonic definition structure, holding details of a single mnemonic.
 */

struct asm_mnemonic_definition {
	char			*name;			/**< The name of the mnemonic.					*/
	enum parse_keyword	keyword;		/**< The keyword whose token can replace the text.		*/
	char			**conditionals;		/**< A list of possible conditional siffixes.			*/
	char			**suffixes;		/**< A list of possible suffixes to follow the conditional.	*/
	char			***parameters;		/**< A list of parameter lists.					*/
};

/**
 * Condition Codes.
 */

static char *asm_conditionals[] = {"AL", "CC", "CS", "EQ", "GE", "GT", "HI", "HS",
		"LE", "LO", "LS", "LT", "MI", "NE", "NV", "PL", "VC", "VS", NULL};

/**
 * Mnemonic Suffixes.
 */

static char *asm_suffix_S[] = {"S", NULL};
static char *asm_suffix_SP[] = {"SP", "S", "P", NULL};
static char *asm_suffix_E[] = {"E", NULL};
static char *asm_suffix_B[] = {"B", NULL};
static char *asm_suffix_L[] = {"L", NULL};
static char *asm_suffix_ldm[] = {"DA", "DB", "EA", "ED", "FA", "FD", "IA", "IB", NULL};
static char *asm_suffix_ldr[] = {"BT", "SB", "SH", "B", "H", "T", NULL};
static char *asm_suffix_dcf[] = {"D", "E", "S", NULL};
static char *asm_suffix_fp_all[] = {"DM", "DP", "DZ", "EM", "EP", "EZ", "SM", "SP", "SZ", "D", "E", "S", NULL};
static char *asm_suffix_fp_single[] = {"SM", "SP", "SZ", "S", NULL};
static char *asm_suffix_fp_ldm[] = {"EA", "FD", NULL};

/**
 * Parameter Content Lists.
 */

/* No expected matches (ie. treat everything as a variable name). */

static char *asm_no_params[] = {NULL};

/* ARM Registers. */

static char *asm_registers[] = {"R15", "R14", "R13", "R12", "R11", "R10", "R9", "R8",
		"R7", "R6", "R5", "R4", "R3", "R2", "R1", "R0", "PC", "LR", "SP", NULL};

/* ARM Registers and Shifts. */

static char *asm_shifts[] = {"ASL", "ASR", "LSL", "LSR", "ROR", "RRX", "R15", "R14",
		"R13", "R12", "R11", "R10", "R9", "R8", "R7", "R6", "R5", "R4",
		"R3", "R2", "R1", "R0", "PC", "LR", "SP", NULL};

/* Co-Processor Registers. */

static char *asm_coproreg[] = {"C15", "C14", "C13", "C12", "C11", "C10", "C9", "C8",
		"C7", "C6", "C5", "C4", "C3", "C2", "C1", "C0", NULL};

/* Floating Point Registers. */

static char *asm_fpreg[] = {"F0", "F1", "F2", "F3", "F4", "F5", "F6", "F7", NULL};

/* Co-Processors. */

static char *asm_copros[] = {"CP0", "CP1", "CP2", "CP3", "CP4", "CP5", "CP6", "CP7",
		"CP8", "CP9", "CP10", "CP11", "CP12", "CP13", "CP14", "CP15", NULL};

/* Status Registers. */

static char *asm_statusmrs[] = {"CPSR", "SPSR", NULL};

/* Status Registers with Bitfields. */

static char *asm_statusmsr[] = {"CPSR_C", "CPSR_F", "CPSR_S", "CPSR_X",
		"SPSR_C", "SPSR_F", "SPSR_S", "SPSR_X", NULL};

/**
 * Parameter Lists.
 */

/* No parameters which look like variable names. */

static char **asm_param_none[] = {NULL};

/* One ARM register parameter*/

static char **asm_param_1[] = {asm_registers, NULL};

/* Two ARM register parameters. */

static char **asm_param_2_shift[] = {asm_registers, asm_registers, asm_shifts, NULL};

/* Three ARM register parameters. */

static char **asm_param_3[] = {asm_registers, asm_registers, asm_registers, NULL};
static char **asm_param_3_shift[] = {asm_registers, asm_registers, asm_registers, asm_shifts, NULL};

/* Four ARM register parameters. */

static char **asm_param_4[] = {asm_registers, asm_registers, asm_registers, asm_registers, NULL};

/* LDR and STR, where there can be different combinations of shifts.
 * These combinations will allow shifts through for the latter final
 * register location, as the first shift might not be present.
 */

static char **asm_param_ldr_3[] = {asm_registers, asm_registers, asm_registers, asm_shifts,
		asm_shifts, asm_shifts, NULL};
static char **asm_param_ldr_4[] = {asm_registers, asm_registers, asm_registers, asm_registers,
		asm_shifts, asm_shifts, asm_shifts, NULL};

/* 16 ARM register parameters (worst-case LDM and STM). */

static char **asm_param_ldm[] = {asm_registers, asm_registers, asm_registers, asm_registers,
		asm_registers, asm_registers, asm_registers, asm_registers, asm_registers,
		asm_registers, asm_registers, asm_registers, asm_registers, asm_registers,
		asm_registers, asm_registers, asm_registers, NULL};

/* Move data between ARM registers and Status registers. */

static char **asm_param_mrs[] = {asm_registers, asm_statusmrs, NULL};
static char **asm_param_msr[] = {asm_statusmsr, asm_registers, NULL};

/* Move data between ARM and Co-Processor. */

static char **asm_param_cp_cdp[] = {asm_copros, asm_no_params, asm_coproreg,
		asm_coproreg, asm_coproreg, asm_no_params, NULL};
static char **asm_param_cp_mcr[] = {asm_copros, asm_no_params, asm_registers,
		asm_coproreg, asm_coproreg, NULL};
static char **asm_param_cp_mcrr[] = {asm_copros, asm_no_params, asm_registers,
		asm_registers, asm_coproreg, NULL};
static char **asm_param_cp_ldc[] = {asm_copros, asm_registers, NULL};

/* Floating Point operations. */

static char **asm_param_fp_2[] = {asm_fpreg, asm_fpreg, NULL};
static char **asm_param_fp_3[] = {asm_fpreg, asm_fpreg, asm_fpreg, NULL};
static char **asm_param_fp_fix[] = {asm_registers, asm_fpreg, NULL};
static char **asm_param_fp_flt[] = {asm_fpreg, asm_registers, NULL};
static char **asm_param_fp_ldf[] = {asm_fpreg, asm_registers, NULL};
static char **asm_param_fp_lfm[] = {asm_fpreg, asm_no_params, asm_registers, NULL};

/**
 * Assembler Mnemonic Definitions.
 */

static struct asm_mnemonic_definition asm_mnemonics[] = {
	/* &0 */

	{"ADC",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_S,		asm_param_3_shift},
	{"ADD",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_S,		asm_param_3_shift},
	{"AND",		KWD_AND,	asm_conditionals,	asm_suffix_S,		asm_param_3_shift},
	{"BIC",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_S,		asm_param_3_shift},
	{"EOR",		KWD_EOR,	asm_conditionals,	asm_suffix_S,		asm_param_3_shift},
	{"ORR",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_S,		asm_param_3_shift},
	{"RSB",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_S,		asm_param_3_shift},
	{"RSC",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_S,		asm_param_3_shift},
	{"SBC",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_S,		asm_param_3_shift},
	{"SUB",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_S,		asm_param_3_shift},

	/* &1 */

	{"CMN",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_SP,		asm_param_2_shift},
	{"CMP",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_SP,		asm_param_2_shift},
	{"TEQ",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_SP,		asm_param_2_shift},
	{"TST",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_SP,		asm_param_2_shift},

	/* &2 */

	{"MOV",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_S,		asm_param_2_shift},
	{"MVN",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_S,		asm_param_2_shift},

	/* &3 */

	{"MUL",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_S,		asm_param_3},
	{"MLA",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_S,		asm_param_4},
	{"SMLAL",	KWD_NO_MATCH,	asm_conditionals,	asm_suffix_S,		asm_param_4},
	{"SMUAL",	KWD_NO_MATCH,	asm_conditionals,	asm_suffix_S,		asm_param_4},
	{"UMLAL",	KWD_NO_MATCH,	asm_conditionals,	asm_suffix_S,		asm_param_4},
	{"UMULL",	KWD_NO_MATCH,	asm_conditionals,	asm_suffix_S,		asm_param_4},
	{"SMULB",	KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_param_3},
	{"SMULTB",	KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_param_3},
	{"SMULWB",	KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_param_3},
	{"SMULWTB",	KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_param_3},
	{"SMLAB",	KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_param_4},
	{"SMLATB",	KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_param_4},
	{"SMLAWB",	KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_param_4},
	{"SMLAWTB",	KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_param_4},

	/* &4 */

	{"LDR",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_ldr,		asm_param_ldr_3},
	{"LDRD",	KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_param_ldr_4},

	/* &5 */

	{"STR",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_ldr,		asm_param_ldr_3},
	{"STRD",	KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_param_ldr_4},

	/* &6 */

	{"LDM",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_ldm,		asm_param_ldm},

	/* &7 */

	{"STM",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_ldm,		asm_param_ldm},

	/* &8 */

	{"SWI",		KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_param_none},

	/* &9 */

	{"CDP",		KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_param_cp_cdp},
	{"CDP2",	KWD_NO_MATCH,	NULL		,	NULL,			asm_param_cp_cdp},
	{"LDC",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_L,		asm_param_cp_ldc},
	{"LDC2",	KWD_NO_MATCH,	NULL,			asm_suffix_L,		asm_param_cp_ldc},
	{"MCR",		KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_param_cp_mcr},
	{"MCR2",	KWD_NO_MATCH,	NULL,			NULL,			asm_param_cp_mcr},
	{"MRC",		KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_param_cp_mcr},
	{"MRC2",	KWD_NO_MATCH,	NULL,			NULL,			asm_param_cp_mcr},
	{"MCRR",	KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_param_cp_mcrr},
	{"MRRC",	KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_param_cp_mcrr},
	{"STC",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_L,		asm_param_cp_ldc},
	{"STC2",	KWD_NO_MATCH,	NULL,			asm_suffix_L,		asm_param_cp_ldc},

	/* &A */

	{"MRS",		KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_param_mrs},
	{"MSR",		KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_param_msr},
	{"SWP",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_B,		asm_param_3},

	/* &B */

	{"LDF",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_fp_all,	asm_param_fp_ldf},
	{"LFM",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_fp_ldm,	asm_param_fp_lfm},
	{"STF",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_fp_all,	asm_param_fp_ldf},
	{"SFM",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_fp_ldm,	asm_param_fp_lfm},

	/* &C */

	{"CMF",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_E,		asm_param_fp_2},
	{"CNF",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_E,		asm_param_fp_2},
	{"FIX",		KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_param_fp_fix},
	{"FLT",		KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_param_fp_flt},
	{"RFC",		KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_param_1},
	{"RFS",		KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_param_1},
	{"WFC",		KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_param_1},
	{"WFS",		KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_param_1},

	/* &D */

	{"ADF",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_fp_all,	asm_param_fp_3},
	{"DVF",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_fp_all,	asm_param_fp_3},
	{"FDV",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_fp_single,	asm_param_fp_3},
	{"FML",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_fp_single,	asm_param_fp_3},
	{"FRD",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_fp_single,	asm_param_fp_3},
	{"MUF",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_fp_all,	asm_param_fp_3},
	{"POL",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_fp_all,	asm_param_fp_3},
	{"POW",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_fp_all,	asm_param_fp_3},
	{"RDF",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_fp_all,	asm_param_fp_3},
	{"RMF",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_fp_all,	asm_param_fp_3},
	{"RPW",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_fp_all,	asm_param_fp_3},
	{"RSF",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_fp_all,	asm_param_fp_3},
	{"SUF",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_fp_all,	asm_param_fp_3},

	/* &E */

	{"ABS",		KWD_ABS,	asm_conditionals,	asm_suffix_fp_all,	asm_param_fp_2},
	{"ACS",		KWD_ACS,	asm_conditionals,	asm_suffix_fp_all,	asm_param_fp_2},
	{"ASN",		KWD_ASN,	asm_conditionals,	asm_suffix_fp_all,	asm_param_fp_2},
	{"ATN",		KWD_ATN,	asm_conditionals,	asm_suffix_fp_all,	asm_param_fp_2},
	{"COS",		KWD_COS,	asm_conditionals,	asm_suffix_fp_all,	asm_param_fp_2},
	{"EXP",		KWD_EXP,	asm_conditionals,	asm_suffix_fp_all,	asm_param_fp_2},
	{"LGN",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_fp_all,	asm_param_fp_2},
	{"LOG",		KWD_LOG,	asm_conditionals,	asm_suffix_fp_all,	asm_param_fp_2},
	{"MNF",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_fp_all,	asm_param_fp_2},
	{"MVF",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_fp_all,	asm_param_fp_2},
	{"NRM",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_fp_all,	asm_param_fp_2},
	{"RND",		KWD_RND,	asm_conditionals,	asm_suffix_fp_all,	asm_param_fp_2},
	{"SIN",		KWD_SIN,	asm_conditionals,	asm_suffix_fp_all,	asm_param_fp_2},
	{"SQT",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_fp_all,	asm_param_fp_2},
	{"TAN",		KWD_TAN,	asm_conditionals,	asm_suffix_fp_all,	asm_param_fp_2},
	{"URD",		KWD_NO_MATCH,	asm_conditionals,	asm_suffix_fp_all,	asm_param_fp_2},

	/* Other Stuff */

	{"QADD",	KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_param_3},
	{"QSUB",	KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_param_3},
	{"QDADD",	KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_param_3},
	{"QDSUB",	KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_param_3},
	{"CLZ",		KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_param_none},
	{"BKPT",	KWD_NO_MATCH,	NULL,			NULL,			asm_param_none},
	{"PLD",		KWD_NO_MATCH,	NULL,			NULL,			asm_param_2_shift},


	/* Branches */

	{"BLX",		KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_param_1},
	{"BL",		KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_param_none},
	{"BX",		KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_param_none},
	{"B",		KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_param_none},

	/* &F (Directives) */

	{"ADR",		KWD_NO_MATCH,	asm_conditionals,	NULL,			asm_param_1},
	{"ALIGN",	KWD_NO_MATCH,	NULL,			NULL,			asm_param_none},
	{"DCB",		KWD_NO_MATCH,	NULL,			NULL,			asm_param_none},
	{"DCF",		KWD_NO_MATCH,	NULL,			asm_suffix_dcf,		asm_param_none},
	{"DCW",		KWD_NO_MATCH,	NULL,			NULL,			asm_param_none},
	{"DCD",		KWD_NO_MATCH,	NULL,			NULL,			asm_param_none},
	{"EQUB",	KWD_NO_MATCH,	NULL,			NULL,			asm_param_none},
	{"EQUD",	KWD_NO_MATCH,	NULL,			NULL,			asm_param_none},
	{"EQUF",	KWD_NO_MATCH,	NULL,			asm_suffix_dcf,		asm_param_none},
	{"EQUS",	KWD_NO_MATCH,	NULL,			NULL,			asm_param_none},
	{"EQUW",	KWD_NO_MATCH,	NULL,			NULL,			asm_param_none},
	{"NOP",		KWD_NO_MATCH,	NULL,			NULL,			asm_param_none},
	{"OPT",		KWD_NO_MATCH,	NULL,			NULL,			asm_param_none},

	/* End */

	{NULL,		KWD_NO_MATCH,	NULL,			NULL,			NULL}
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
		return;
	} else if ((asm_current_state == ASM_AT_START || asm_current_state == ASM_FOUND_LABEL) && keyword == KWD_MOVE) {
		asm_current_state = ASM_FOUND_MOVE;
		return;
	}

	/* Otherwise, we're looking for whole tokenised keywords. Scan the list
	 * of mnemonics and see if we can find a match for this token.
	 */

	while (asm_mnemonics[entry].name != NULL && asm_mnemonics[entry].keyword != keyword)
		entry++;

	if (asm_mnemonics[entry].name == NULL)
		return;

	/* If the token matched, and we're not at the start of a statement, this
	 * is unexpected...
	 */

	if (asm_current_state != ASM_AT_START && asm_current_state != ASM_FOUND_LABEL) {
		asm_current_state = ASM_EXTRA_MNEMONIC;
		return;
	}

	/* Otherwise, we've found a mnemonic. Record that in case we get called
	 * with some variable text to check.
	 */

	asm_current_state = ASM_FOUND_TOKEN;
	asm_current_mnemonic = entry;
	asm_current_parameter = asm_mnemonics[entry].parameters;
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

	/* Otherwise, there are a range of ways that we can start a new assembler
	 * statement. Work through all of these in turn.
	 */

	if (asm_current_state == ASM_FOUND_TOKEN) {
		/* If we've found a valid tokenised mnemonic, then check to see
		 * if that was the last thing in the buffer (ie. if this new text
		 * is contiguous). If it was, then we need to check for condition
		 * codes; otherwise there's been a space and we need to look for
		 * parameters.
		 */ 

		if (*(*text - 1) == (char) parse_get_token(asm_mnemonics[asm_current_mnemonic].keyword))
			asm_current_state = ASM_TEST_CONDITIONAL;
		else
			asm_current_state = ASM_TEST_PARAMETERS;
	} else if ((asm_current_state == ASM_FOUND_OR) && (*(*text - 1) == ((char) parse_get_token(KWD_OR)))) {
		/* If we've found a tokenised OR as the last thing in the buffer,
		 * and the next character is an R, we've found an ORR mnemonic and
		 * need to move on to testing condition codes.
		 */

		if (toupper(**text) == 'R') {
			*text += 1;
			asm_current_state = ASM_TEST_CONDITIONAL;
			asm_current_mnemonic = MNM_ORR;
			asm_current_parameter = asm_mnemonics[MNM_ORR].parameters;
		}
	} else if ((asm_current_state == ASM_FOUND_MOVE) && (*(*text - 1) == ((char) parse_get_token(KWD_MOVE)))) {
		/* If we've found a tokenised MOVE as the last thing in the buffer,
		 * and the next character is a Q, we've found a MOVEQ mnemonic and
		 * need to move on to testing possible suffixes.
		 */

		if (toupper(**text) == 'Q') {
			*text += 1;
			asm_current_state = ASM_TEST_PARAMETERS;
			asm_current_mnemonic = MNM_MOV;
			asm_current_parameter = asm_mnemonics[MNM_MOV].parameters;
		}
	} else if (asm_current_state == ASM_AT_START || asm_current_state == ASM_FOUND_LABEL) {
		int			i, longest = 0;
		enum asm_mnemonic	entry = 0, found = MNM_NO_MATCH;

		/* Failing that, if we're at the start of a statement, test the
		 * text in the buffer to see if it forms one of the valid
		 * mnemonics. Test the text against the list of mnemonics, taking
		 * the longest match we can find (so that "BIC" mathes "BIC" and
		 * not "B", for example).
		 *
		 * If a match is found, we move on to looking for condition
		 * codes.
		 */

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
			*text += strlen(asm_mnemonics[found].name);
			asm_current_state = ASM_TEST_CONDITIONAL;
			asm_current_mnemonic = found;
			asm_current_parameter = asm_mnemonics[found].parameters;
		}
	}

	/* If we're ready to test a condition code, carry out that test now. */

	if (asm_current_state == ASM_TEST_CONDITIONAL && asm_current_mnemonic != MNM_NO_MATCH) {
		if (asm_mnemonics[asm_current_mnemonic].conditionals == NULL) {
			/* If the mnemonic doesn't take condition codes, move
			 * on to test any suffixes.
			 */

			asm_current_state = ASM_TEST_SUFFIX;
		} else if (**text == '\0') {
			/* If we've reached the end of the buffer, there can't be
			 * any suffixes so move straight on to parameters.
			 */

			asm_current_state = ASM_TEST_PARAMETERS;
		} else {
			char	*conditional = NULL;

			/* Otherwise, try to match the text against one of the
			 * list of condition codes. If we match, push the pointer
			 * on past the code; either way move on to test suffixes.
			 */

			if ((conditional = asm_match_list(asm_mnemonics[asm_current_mnemonic].conditionals, *text)) != NULL)
				*text += strlen(conditional);

			asm_current_state = ASM_TEST_SUFFIX;
		}
	}

	/* If we're ready to test suffixes, carry out the test now. */

	if (asm_current_state == ASM_TEST_SUFFIX && asm_current_mnemonic != MNM_NO_MATCH) {
		if (asm_mnemonics[asm_current_mnemonic].suffixes == NULL || **text == '\0') {
			/* If there are no suffixes defined for this mnemonic, move
			 * straight on to testing parameters.
			 */

			asm_current_state = ASM_TEST_PARAMETERS;
		} else {
			char	*suffix = NULL;

			/* Otherwise, try to match the text against one of the
			 * list of possible suffixes. If we match, push the pointer
			 * on past the text; either way, move on to test for
			 * the first parameter.
			 */

			if ((suffix = asm_match_list(asm_mnemonics[asm_current_mnemonic].suffixes, *text)) != NULL)
				*text += strlen(suffix);

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

		while ((param = asm_match_list(*asm_current_parameter, *text)) != NULL)
			*text += strlen(param);
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

