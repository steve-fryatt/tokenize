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

#include "args.h"
#include "library.h"
#include "parse.h"


#define MAX_INPUT_LINE_LENGTH 1024



bool tokenize_run_job(char *output_file, unsigned line_increment, unsigned tab_increment);
void tokenize_parse_file(FILE *in, FILE *out, unsigned *line_number, unsigned line_increment, unsigned tab_increment);

int main(int argc, char *argv[])
{
	bool			param_error = false;
	struct args_option	*options;
	struct args_data	*option_data;
	char *output_file	= NULL;

	unsigned		line_increment = 10;		/**< The default line number increment.		*/
	unsigned		tab_indent = 8;			/**< The default tab indent.			*/


	//stack_initialise(MAX_STACK_SIZE);

	printf("Tokenize %s - %s\n", BUILD_VERSION, BUILD_DATE);
	printf("Copyright Stephen Fryatt, %s\n", BUILD_DATE + 7);

	options = args_process_line(argc, argv, "path/KM,source/AM,out/AK,increment/IK,link/KS,tab/IK");
	if (options == NULL) {
		fprintf(stderr, "Usage: tokenize -out <output> <source1> [<source2> ...]\n");
		return 1;
	}

	while (options != NULL) {
		if (strcmp(options->name, "increment") == 0) {
			if (options->data != NULL)
				line_increment = options->data->value.integer;
		} else if (strcmp(options->name, "source") == 0) {
			if (options->data != NULL) {
				option_data = options->data;
				
				while (option_data != NULL) {
					if (option_data->value.string != NULL)
						library_add_file(option_data->value.string);
					option_data = option_data->next;
				}
			} else {
				param_error = true;
			}
		} else if (strcmp(options->name, "out") == 0) {
			if (options->data != NULL && options->data->value.string != NULL)
				output_file = options->data->value.string;
			else
				param_error = true;
		} else if (strcmp(options->name, "path") == 0) {
			if (options->data != NULL) {
				option_data = options->data;
				
				while (option_data != NULL) {
					if (option_data->value.string != NULL)
						library_add_path_combined(option_data->value.string);
					option_data = option_data->next;
				}
			} else {
				param_error = true;
			}
		} else if (strcmp(options->name, "tab") == 0) {
			if (options->data != NULL)
				tab_indent = options->data->value.integer;
		}
	
		options = options->next;
	}

	if (param_error) {
		fprintf(stderr, "Usage: tokenize -out <output> <source1> [<source2> ...]\n");
		return 1;
	}

	if (!tokenize_run_job(output_file, line_increment, tab_indent))
		return 1;

	return 0;
}

bool tokenize_run_job(char *output_file, unsigned line_increment, unsigned tab_indent)
{
	FILE		*in, *out;
	unsigned	line_number = 0;

	printf("Open for file '%s'\n", output_file);

	out = fopen(output_file, "w");
	if (out == NULL)
		return false;

	while ((in = library_get_file()) != NULL) {
		tokenize_parse_file(in, out, &line_number, line_increment, tab_indent);
		fclose(in);
	}

	fputc(0x0d, out);
	fputc(0xff, out);

	fclose(out);
	
	return true;
}


void tokenize_parse_file(FILE *in, FILE *out, unsigned *line_number, unsigned line_increment, unsigned tab_indent)
{
	char		line[MAX_INPUT_LINE_LENGTH], *tokenised;
	bool		assembler = false;

	if (in == NULL || out == NULL)
		return;

	while (fgets(line, MAX_INPUT_LINE_LENGTH, in) != NULL) {
		*line_number += line_increment;
		
		tokenised = parse_process_line(line, tab_indent, &assembler, line_number);
		if (tokenised != NULL)
			fwrite(tokenised, sizeof(char), *(tokenised + 3), out);
		else
			break;
	}
}

