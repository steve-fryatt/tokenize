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
 * \file msg.c
 *
 * Status Message, implementation.
 */

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

/* Local source headers. */

#include "msg.h"

#define MSG_MAX_LOCATION_TEXT 256
#define MSG_MAX_MESSAGE 256

enum msg_level {
	MSG_INFO,
	MSG_WARNING,
	MSG_ERROR
};

struct msg_data {
	enum msg_level	level;
	char		*text;
	bool		show_location;
};


/**
 * Error message definitions.
 *
 * NB: The order of these messages *must* match the order of the corresponding
 * entries in enum msg_type in msgs.h
 */

static struct msg_data msg_messages[] = {
	{MSG_ERROR,	"Unknown error",				true	},
	{MSG_ERROR,	"Failed to open source file '%s'",		false	},
	{MSG_ERROR,	"Constant variable %s already defined",		false	},
	{MSG_WARNING,	"Constant variable assignment to %s removed",	true	},
	{MSG_ERROR,	"No room to define variable %s",		false	},
	{MSG_WARNING,	"Variable %s used but not defined",		false	},
	{MSG_ERROR,	"No room to define function or procedure %s%s",	false	},
	{MSG_WARNING,	"No definition found for %s%s",			false	},
	{MSG_WARNING,	"%s%s defined more than once",			false	},
	{MSG_WARNING,	"%s%s is defined but not used",			false	},
	{MSG_ERROR,	"Line number %u out of range",			true	},
	{MSG_ERROR,	"AUTO line number too large",			true	},
	{MSG_WARNING,	"Line number %u out of sequence",		true	},
	{MSG_ERROR,	"Line too long",				true	},
	{MSG_ERROR,	"Invalid line number constant",			true	},
	{MSG_WARNING,	"Unterminated string",				true	},
	{MSG_ERROR,	"Misformed deleted statement",			true	},
	{MSG_INFO,	"Queue 'LIBRARY \"%s\"' for linking",		true	},
	{MSG_WARNING,	"Unisolated LIBRARY not linked",		true	},
	{MSG_WARNING,	"Variable LIBRARY not linked",			true	},
	{MSG_WARNING,	"SYS \"%s\" not found on lookup",		true	},
	{MSG_ERROR,	"Failed to load SWI file '%s'",			false	}
};

static char	msg_location[MSG_MAX_LOCATION_TEXT];

/**
 * Set to true if an error is reported.
 */

static bool	msg_error_reported = false;

/**
 * Set the location for future messages, in the form of a file and line number
 * relating to the source files.
 *
 * \param line		The number of the current line.
 * \param *file		Pointer to the name of the current file.
 */

void msg_set_location(unsigned line, char *file)
{
	snprintf(msg_location, MSG_MAX_LOCATION_TEXT, "at line %u of '%s'", line, (file != NULL) ? file : "");
	msg_location[MSG_MAX_LOCATION_TEXT - 1] = '\0';
}


/**
 * Generate a message to the user, based on a range of standard message tokens
 *
 * \param type		The message to be displayed.
 * \param ...		Additional printf parameters as required by the token.
 */

void msg_report(enum msg_type type, ...)
{
	char		message[MSG_MAX_MESSAGE], *level;
	va_list		ap;

	if (type < 0 || type >= MSG_MAX_MESSAGES)
		return;

	va_start(ap, type);
	vsnprintf(message, MSG_MAX_MESSAGE, msg_messages[type].text, ap);
	va_end(ap);

	message[MSG_MAX_MESSAGE - 1] = '\0';

	switch (msg_messages[type].level) {
	case MSG_INFO:
		level = "Info";
		break;
	case MSG_WARNING:
		level = "Warning";
		break;
	case MSG_ERROR:
		level = "Error";
		msg_error_reported = true;
		break;
	default:
		level = "Message:";
		break;
	}

	if (msg_messages[type].show_location)
		fprintf(stderr, "%s: %s %s\n", level, message, msg_location);
	else
		fprintf(stderr, "%s: %s\n", level, message);
}


/**
 * Indicate whether an error has been reported at any point.
 *
 * \return		True if an error has been reported; else false.
 */

bool msg_errors(void)
{
	return msg_error_reported;
}

