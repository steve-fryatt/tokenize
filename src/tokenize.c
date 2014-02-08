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

/* Tokenize
 *
 * Generate tokenized BBC BASIC files from ASCII text.
 *
 * Syntax: Tokenize [<options>]
 *
 * Options -v  - Produce verbose output
 */

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Local source headers. */

#include "parse.h"


#define MAX_INPUT_LINE_LENGTH 1024

static int verbose_output = 0;
static int embed_dialogue_names = 0;


void tokenize_parse_file(char *in, char *out);


int main(int argc, char *argv[])
{
	int	param;
	bool	param_error = false;

	//stack_initialise(MAX_STACK_SIZE);

	printf("Tokenize %s - %s\n", BUILD_VERSION, BUILD_DATE);
	printf("Copyright Stephen Fryatt, %s\n", BUILD_DATE + 7);

	args_process_line(argc, argv, "source/AM,out/AK,link/KS,path/K,test");

	if (argc < 3)
		param_error = true;

	if (!param_error) {
		for (param = 3; param < argc; param++) {
			if (strcmp(argv[param], "-d") == 0)
				embed_dialogue_names = 1;
			else if (strcmp(argv[param], "-v") == 0)
				verbose_output = 1;
			else
				param_error = true;
		}
	}

	if (param_error) {
		printf("Usage: tokenize <sourcefile> <output> [-v]\n");
		return 1;
	}
	
	tokenize_parse_file(argv[1], argv[2]);
/*
	printf("Starting to parse menu definition file...\n");
	if (parse_process_file(argv[1], verbose_output)) {
		printf("Errors in source file: terminating.\n");
		return 1;
	}

	printf("Collating menu data...\n");
	data_collate_structures(embed_dialogue_names, verbose_output);

	if (verbose_output) {
		printf("Printing structure report...\n");
		data_print_structure_report();
	}

	printf("Writing menu file...\n");
	data_write_standard_menu_file(argv[2]);
*/
	return 0;
}

void tokenize_parse_file(char *in, char *out)
{
	FILE		*fin, *fout;
	char		line[MAX_INPUT_LINE_LENGTH], *tokenised;
	bool		assembler = false;
	unsigned	line_number = 0, increment = 10, indent = 8;

	if (in == NULL || out == NULL)
		return;

	fin = fopen(in, "r");
	if (fin == NULL)
		return;
	
	fout = fopen(out, "w");
	if (fout == NULL) {
		fclose(fin);
		return;
	}

	while (fgets(line, MAX_INPUT_LINE_LENGTH, fin) != NULL) {
		line_number += increment;
		
		tokenised = parse_process_line(line, indent, &assembler, &line_number);
		if (tokenised != NULL)
			fwrite(tokenised, sizeof(char), *(tokenised + 3), fout);
		else
			break;
	}

	fputc(0x0d, fout);
	fputc(0xff, fout);

	fclose(fin);
	fclose(fout);
}

