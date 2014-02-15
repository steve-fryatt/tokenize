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
#define MAX_LOCATION_TEXT 256

bool tokenize_run_job(char *output_file, struct parse_options *options);
bool tokenize_parse_file(FILE *in, FILE *out, unsigned *line_number, struct parse_options *options);

int main(int argc, char *argv[])
{
	bool			param_error = false;
	struct args_option	*options;
	struct args_data	*option_data;
	char *output_file	= NULL;
	struct parse_options	parse_options;

	parse_options.tab_indent = 8;
	parse_options.line_increment = 10;
	parse_options.link_libraries = false;
	parse_options.verbose_output = false;
	parse_options.crunch_body_rems = false;
	parse_options.crunch_rems = false;
	parse_options.crunch_empty = false;
	parse_options.crunch_indent = false;
	parse_options.crunch_whitespace = false;
	parse_options.crunch_all_whitespace = false;

	options = args_process_line(argc, argv, "path/KM,source/AM,out/AK,increment/IK,link/KS,tab/IK,crunch/K,verbose/S");
	if (options == NULL) {
		fprintf(stderr, "Usage: tokenize -out <output> <source1> [<source2> ...]\n");
		return 1;
	}

	while (options != NULL) {
		if (strcmp(options->name, "crunch") == 0) {
			if (options->data != NULL) {
				char *mode = options->data->value.string;
				
				while (mode != NULL && *mode != '\0') {
					switch (*mode++) {
					case 'e':
						parse_options.crunch_empty = true;
						break;
					case 'i':
						parse_options.crunch_indent = true;
						break;
					case 'R':
						parse_options.crunch_rems = true;
					case 'r':
						parse_options.crunch_body_rems = true;
						break;
					case 'W':
						parse_options.crunch_all_whitespace = true;
					case 'w':
						parse_options.crunch_whitespace = true;
						break;
					}
				}
			}
		} else if (strcmp(options->name, "increment") == 0) {
			if (options->data != NULL)
				parse_options.line_increment = options->data->value.integer;
		} else if (strcmp(options->name, "link") == 0) {
			if (options->data != NULL && options->data->value.boolean == true)
				parse_options.link_libraries = true;
		} else if (strcmp(options->name, "verbose") == 0) {
			if (options->data != NULL && options->data->value.boolean == true)
				parse_options.verbose_output = true;
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
				parse_options.tab_indent = options->data->value.integer;
		}
	
		options = options->next;
	}

	if (param_error) {
		fprintf(stderr, "Usage: tokenize -out <output> <source1> [<source2> ...]\n");
		return 1;
	}

	if (parse_options.verbose_output) {
		printf("Tokenize %s - %s\n", BUILD_VERSION, BUILD_DATE);
		printf("Copyright Stephen Fryatt, %s\n", BUILD_DATE + 7);
	}

	if (!tokenize_run_job(output_file, &parse_options))
		return 1;

	return 0;
}


/**
 * Run a tokenisation job, writing data to the specified output file. Input
 * files are taken from the library module, so as to handle any linked libraries
 * found during parsing.
 *
 * \param *output_file	Pointer to the name of the file to write to.
 * \param *options	Pointer to the tokenisation options.
 * \return		True on success; false on failure.
 */

bool tokenize_run_job(char *output_file, struct parse_options *options)
{
	FILE		*in, *out;
	unsigned	line_number = 0;
	bool		success = true;

	if (output_file == NULL || options == NULL)
		return false;

	if (options->verbose_output)
		printf("Creating tokenized file '%s'\n", output_file);

	out = fopen(output_file, "w");
	if (out == NULL)
		return false;

	while ((success == true) && ((in = library_get_file()) != NULL)) {
		success = tokenize_parse_file(in, out, &line_number, options);
		fclose(in);
	}

	fputc(0x0d, out);
	fputc(0xff, out);

	fclose(out);
	
	return true;
}


/**
 * Tokenise the contents of a file, sending the results to the output.
 *
 * \param *in		The handle of the file to be tokenised.
 * \param *out		The handle of the file to write the output to.
 * \param *line_number	Pointer to a variable holding the current line number.
 * \param *options	Pointer to the tokenisation options.
 * \return		True on success; false if an error occurred.
 */

bool tokenize_parse_file(FILE *in, FILE *out, unsigned *line_number, struct parse_options *options)
{
	char		line[MAX_INPUT_LINE_LENGTH], location[MAX_LOCATION_TEXT], *tokenised, *file;
	bool		assembler = false;
	unsigned	input_line = 0;

	if (in == NULL || out == NULL || line_number == NULL || options == NULL)
		return false;

	file = library_get_filename();
	if (file == NULL)
		file = "unknown file";

	if (options->verbose_output)
		printf("Processing source file '%s'\n", file);

	while (fgets(line, MAX_INPUT_LINE_LENGTH, in) != NULL) {
		snprintf(location, MAX_LOCATION_TEXT, " at line %u of '%s'", ++input_line, file);

		tokenised = parse_process_line(line, options, &assembler, line_number, location);
		if (tokenised != NULL) {
			/* The line tokeniser requests a line be deleted (ie. not
			 * written to the output) by setting the leading \r to be
			 * \0 instead (setting the line pointer to NULL signifies
			 * an error).
			 */

			if (*tokenised != '\0')
				fwrite(tokenised, sizeof(char), *(tokenised + 3), out);
		} else {
			return false;
		}
	}
	
	return true;
}

