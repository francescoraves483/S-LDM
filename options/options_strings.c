#include "options_strings.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

options_string options_string_declare(void) {
	options_string new_options_string;

	new_options_string.buf=NULL;
	new_options_string.len=-1;

	return new_options_string;
}

void options_string_free(options_string allocated_options_string) {
	if(allocated_options_string.buf!=NULL) {
		free(allocated_options_string.buf);
		allocated_options_string.len=0;
	}
}

int options_string_push(options_string *new_options_string, const char *char_buff) {
	int len;

	if(char_buff==NULL) {
		return 0;
	}

	if(new_options_string==NULL || new_options_string->buf!=NULL) {
		return 0;
	}

	len=strlen(char_buff)+1;

	if(len<=0 || len>MAX_OPTIONS_STRING_LEN) {
		return 0;
	}

	new_options_string->buf=(char *)malloc(len*sizeof(char));
	strncpy(new_options_string->buf,char_buff,len);
	new_options_string->len=len;

	return 1;
}

char *options_string_pop(options_string allocated_options_string) {
	return allocated_options_string.buf;
}

int options_string_len(options_string allocated_options_string) {
	return allocated_options_string.len;
}