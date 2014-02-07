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
 * \file parse.h
 *
 * Basic line parser, interface.
 */

#ifndef TOKENIZE_PARSE_H
#define TOKENIZE_PARSE_H

#include <stdbool.h>

#define KWD_ABS         0
#define KWD_ACS         1
#define KWD_ADVAL       2
#define KWD_AND         3
#define KWD_APPEND      4
#define KWD_ASC         5
#define KWD_ASN         6
#define KWD_ATN         7
#define KWD_AUTO        8
#define KWD_BEAT        9
#define KWD_BEATS      10
#define KWD_BGET       11
#define KWD_BPUT       12
#define KWD_CALL       13
#define KWD_CASE       14
#define KWD_CHAIN      15
#define KWD_CHR$       16
#define KWD_CIRCLE     17
#define KWD_CLEAR      18
#define KWD_CLG        19
#define KWD_CLOSE      20
#define KWD_CLS        21
#define KWD_COLOR      22
#define KWD_COLOUR     23
#define KWD_COS        24
#define KWD_COUNT      25
#define KWD_CRUNCH     26
#define KWD_DATA       27
#define KWD_DEF        28
#define KWD_DEG        29
#define KWD_DELETE     30
#define KWD_DIM        31
#define KWD_DIV        32
#define KWD_DRAW       33
#define KWD_EDIT       34
#define KWD_ELLIPSE    35
#define KWD_ELSE       36
#define KWD_END        37
#define KWD_ENDCASE    38
#define KWD_ENDIF      39
#define KWD_ENDPROC    40
#define KWD_ENDWHILE   41
#define KWD_EOF        42
#define KWD_EOR        43
#define KWD_ERL        44
#define KWD_ERR        45
#define KWD_ERROR      46
#define KWD_EVAL       47
#define KWD_EXP        48
#define KWD_EXT        49
#define KWD_FALSE      50
#define KWD_FILL       51
#define KWD_FN         52
#define KWD_FOR        53
#define KWD_GCOL       54
#define KWD_GET        55
#define KWD_GET$       56
#define KWD_GOSUB      57
#define KWD_GOTO       58
#define KWD_HELP       59
#define KWD_HIMEM      60
#define KWD_IF         61
#define KWD_INKEY      62
#define KWD_INKEY$     63
#define KWD_INPUT      64
#define KWD_INSTALL    65
#define KWD_INSTR      66
#define KWD_INT        67
#define KWD_LEFT$      68
#define KWD_LEN        69
#define KWD_LET        70
#define KWD_LIBRARY    71
#define KWD_LINE       72
#define KWD_LIST       73
#define KWD_LN         74
#define KWD_LOAD       75
#define KWD_LOCAL      76
#define KWD_LOG        77
#define KWD_LOMEM      78
#define KWD_LVAR       79
#define KWD_MID$       80
#define KWD_MOD        81
#define KWD_MODE       82
#define KWD_MOUSE      83
#define KWD_MOVE       84
#define KWD_NEW        85
#define KWD_NEXT       86
#define KWD_NOT        87
#define KWD_OF         88
#define KWD_OFF        89
#define KWD_OLD        90
#define KWD_ON         91
#define KWD_OPENIN     92
#define KWD_OPENOUT    93
#define KWD_OPENUP     94
#define KWD_OR         95
#define KWD_ORIGIN     96
#define KWD_OSCLI      97
#define KWD_OTHERWISE  98
#define KWD_OVERLAY    99
#define KWD_PAGE      100
#define KWD_PI        101
#define KWD_PLOT      102
#define KWD_POINT     103
#define KWD_POINT2    104
#define KWD_POS       105
#define KWD_PRINT     106
#define KWD_PROC      107
#define KWD_PTR       108
#define KWD_QUIT      109
#define KWD_READ      110
#define KWD_RECTANGLE 111
#define KWD_REM       112
#define KWD_RENUMBER  113
#define KWD_REPEAT    114
#define KWD_REPORT    115
#define KWD_RESTORE   116
#define KWD_RETURN    117
#define KWD_RIGHT$    118
#define KWD_RND       119
#define KWD_RUN       120
#define KWD_SAVE      121
#define KWD_SGN       122
#define KWD_SIN       123
#define KWD_SOUND     124
#define KWD_SPC       125
#define KWD_SQR       126
#define KWD_STEP      127
#define KWD_STEREO    128
#define KWD_STOP      129
#define KWD_STR$      130
#define KWD_STRING$   131
#define KWD_SUM       132
#define KWD_SWAP      133
#define KWD_SYS       134
#define KWD_TAB       135
#define KWD_TAN       136
#define KWD_TEMPO     137
#define KWD_TEXTLOAD  138
#define KWD_TEXTSAVE  139
#define KWD_THEN      140
#define KWD_TIME      141
#define KWD_TINT      142
#define KWD_TO        143
#define KWD_TRACE     144
#define KWD_TRUE      145
#define KWD_TWIN      146
#define KWD_TWINO     147
#define KWD_UNTIL     148
#define KWD_USR       149
#define KWD_VAL       150
#define KWD_VDU       151
#define KWD_VOICE     152
#define KWD_VOICES    153
#define KWD_VPOS      154
#define KWD_WAIT      155
#define KWD_WHEN      156
#define KWD_WHILE     157
#define KWD_WIDTH     158


enum parse_error {
	PARSE_NO_ERROR = 0,		/**< No Error.				*/
	PARSE_MISMATCHED_QUOTES		/**< Mismatched string delimiter.	*/
};

/**
 * Parse a line of BASIC, returning a pointer to the tokenised form which will
 * remain valid until the function is called again.
 *
 * \param *line		Pointer to the line to process,
 * \param *assembler	Pointer to a boolean which is TRUE if we are in an
 *			assember section and FALSE otherwise; updated on exit.
 * \param *line_number	Pointer to a variable to hold the proposed next line
 *			number; updated on exit if a number was found.
 * \return		Pointer to the tokenised line, or NULL on error.
 */

char *parse_process_line(char *line, bool *assembler, unsigned *line_number);

#endif

