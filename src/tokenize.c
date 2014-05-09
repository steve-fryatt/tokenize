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
#include "variable.h"

/* OSLib source headers. */

#ifdef RISCOS
#include "oslib/osfile.h"
#endif

#include "msg.h"

#define MAX_INPUT_LINE_LENGTH 1024
#define MAX_LOCATION_TEXT 256

bool tokenize_run_job(char *output_file, struct parse_options *options);
bool tokenize_parse_file(FILE *in, FILE *out, int *line_number, struct parse_options *options);

int main(int argc, char *argv[])
{
	bool			param_error = false;
	bool			output_help = false;
	struct args_option	*options;
	struct args_data	*option_data;
	char			*output_file = NULL;
	struct parse_options	parse_options;

	/* Default processing options. */

	parse_options.tab_indent = 8;
	parse_options.line_start = 10;
	parse_options.line_increment = 10;
	parse_options.link_libraries = false;
	parse_options.verbose_output = false;
	parse_options.crunch_body_rems = false;
	parse_options.crunch_rems = false;
	parse_options.crunch_empty = false;
	parse_options.crunch_empty_lines = false;
	parse_options.crunch_indent = false;
	parse_options.crunch_trailing = false;
	parse_options.crunch_whitespace = false;
	parse_options.crunch_all_whitespace = false;

	/* Initialise the variable handler. */

	variable_initialise();

	/* Decode the command line options. */

	options = args_process_line(argc, argv, "path/KM,source/AM,out/AK,start/IK,increment/IK,define/KM,link/KS,tab/IK,crunch/K,verbose/S,help/S");
	if (options == NULL)
		param_error = true;

	while (options != NULL) {
		if (strcmp(options->name, "crunch") == 0) {
			if (options->data != NULL) {
				char *mode = options->data->value.string;

				while (mode != NULL && *mode != '\0') {
					switch (*mode++) {
					case 'E':
					case 'e':
						parse_options.crunch_empty = true;
						break;
					case 'I':
					case 'i':
						parse_options.crunch_indent = true;
						break;
					case 'L':
					case 'l':
						parse_options.crunch_empty_lines = true;
						break;
					case 'R':
						parse_options.crunch_rems = true;
					case 'r':
						parse_options.crunch_body_rems = true;
						break;
					case 'T':
					case 't':
						parse_options.crunch_trailing = true;
						break;
					case 'W':
						parse_options.crunch_all_whitespace = true;
					case 'w':
						parse_options.crunch_whitespace = true;
						break;
					}
				}
			}
		} else if (strcmp(options->name, "define") == 0) {
			if (options->data != NULL) {
				option_data = options->data;

				while (option_data != NULL) {
					if (option_data->value.string != NULL)
						variable_add_constant_combined(option_data->value.string);
					else
						param_error = NULL;
					option_data = option_data->next;
				}
			}
		} else if (strcmp(options->name, "help") == 0) {
			if (options->data != NULL && options->data->value.boolean == true)
				output_help = true;
		} else if (strcmp(options->name, "increment") == 0) {
			if (options->data != NULL) {
				parse_options.line_increment = options->data->value.integer;
				if (parse_options.line_increment < 1 || parse_options.line_increment > PARSE_MAX_LINE_NUMBER)
					param_error = true;
			}
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
		} else if (strcmp(options->name, "start") == 0) {
			if (options->data != NULL) {
				parse_options.line_start = options->data->value.integer;
				if (parse_options.line_start < 0 || parse_options.line_start > PARSE_MAX_LINE_NUMBER)
					param_error = true;
			}
		} else if (strcmp(options->name, "out") == 0) {
			if (options->data != NULL && options->data->value.string != NULL)
				output_file = options->data->value.string;
			else
				param_error = true;
		} else if (strcmp(options->name, "path") == 0) {
			if (options->data != NULL) {
#ifdef LINUX
				/* The path parameter is valid on non-RISC OS systems,
				 * as we don't have native system variables to fall
				 * back on.
				 */

				option_data = options->data;

				while (option_data != NULL) {
					if (option_data->value.string != NULL)
						library_add_path_combined(option_data->value.string);
					else
						param_error = NULL;
					option_data = option_data->next;
				}
#endif
#ifdef RISCOS
				/* On RISC OS, there's no point setting paths as
				 * it is better to use real system variables.
				 */

				param_error = true;
#endif
			}
		} else if (strcmp(options->name, "tab") == 0) {
			if (options->data != NULL)
				parse_options.tab_indent = options->data->value.integer;
		}

		options = options->next;
	}

	/* Generate any necessary verbose or help output. If param_error is true,
	 * then we need to give some usage guidance and exit with an error.
	 */

	if (param_error || output_help || parse_options.verbose_output) {
		printf("Tokenize %s - %s\n", BUILD_VERSION, BUILD_DATE);
		printf("Copyright Stephen Fryatt, %s\n", BUILD_DATE + 7);
	}

	if (param_error || output_help) {
		printf("ARM BASIC V Tokenizer -- Usage:\n");
		printf("tokenize <infile> [<infile> ...] -out <outfile> [<options>]\n\n");

		printf(" -crunch [EILRTW]       Control application of output CRUNCHing.\n");
		printf("                    E|e - Remove empty statements.\n");
		printf("                    I|i - Remove opening indents.\n");
		printf("                    L|l - Remove empty lines (implied by E).\n");
		printf("                    R|r - Remove all|non-opening comments.\n");
		printf("                    T|t - Remove trailing whitespace (implied by W).\n");
		printf("                    W|w - Remove|reduce in-line whitespace.\n");
		printf(" -define <name>=<value> Define constant variables.\n");
		printf(" -help                  Produce this help information.\n");
		printf(" -increment <n>         Set the AUTO line number increment to <n>.\n");
		printf(" -link                  Link files from LIBRARY statements.\n");
		printf(" -out <file>            Write tokenised basic to file <out>.\n");
#ifdef LINUX
		printf(" -path <name>:<path>    Set path variable <name> to <path>.\n");
#endif
		printf(" -start <n>             Set the AUTO line number start to <n>.\n");
		printf(" -tab <n>               Set the tab column with to <n> spaces.\n");
		printf(" -verbose               Generate verbose process information.\n");

		return (output_help) ? 0 : 1;
	}

	/* Run the tokenisation. */

	if (!tokenize_run_job(output_file, &parse_options) || msg_errors())
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
	int		line_number = -1;
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

#if RISCOS
	osfile_set_type(output_file, osfile_TYPE_BASIC);
#endif

	return success;
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

bool tokenize_parse_file(FILE *in, FILE *out, int *line_number, struct parse_options *options)
{
	char		line[MAX_INPUT_LINE_LENGTH], *tokenised, *file;
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
		msg_set_location(++input_line, file);

		tokenised = parse_process_line(line, options, &assembler, line_number);
		if (tokenised != NULL) {
			/* The line tokeniser requests a line be deleted (ie. not
			 * written to the output) by setting the leading \r to be
			 * \0 instead (setting the line pointer to NULL signifies
			 * an error).
			 */

			if (*tokenised != '\0')
				fwrite(tokenised, sizeof(char), *((unsigned char *) tokenised + 3), out);
		} else {
			return false;
		}
	}

	return true;
}

