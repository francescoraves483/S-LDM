/*
*	Options C module
*	
*	C module for easy and efficient command-line options management
*	The code is derived from LaTe, out project for an advanced latency measurement tool: https://github.com/francescoraves483/LaMP_LaTe
*
*	Copyright (C) 2019-2021 Francesco Raviglione (CNIT - Politecnico di Torino), Marco Malinverno (CNIT - Politecnico di Torino)
*	
*	This program is free software; you can redistribute it and/or
*	modify it under the terms of the GNU General Public License
*	as published by the Free Software Foundation; either version 2
*	of the License, or (at your option) any later version.
*	
*	This program is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*	
*	You should have received a copy of the GNU General Public License
*	along with this program; if not, write to the Free Software
*	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef OPTIONS_STRINGS_H_INCLUDED
#define OPTIONS_STRINGS_H_INCLUDED

#define MAX_OPTIONS_STRING_LEN 512 // Maximum number of characters in a 'options_string' buffer
	
typedef struct _options_string {
	char *buf;
	int len;
} options_string;

options_string options_string_declare(void);
void options_string_init(options_string *allocated_options_string);
void options_string_free(options_string allocated_options_string);
int options_string_push(options_string *new_options_string, const char *char_buff);
char *options_string_pop(options_string allocated_options_string);
int options_string_len(options_string allocated_options_string);

#endif