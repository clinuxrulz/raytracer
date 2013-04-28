/*
 * text.c
 *
 *  Created on: 26/04/2013
 *      Author: Clinton
 */

#include <malloc.h>
#include <memory.h>
#include <string.h>
#include "text.h"

struct _Text {
	char* data;
	int ref_count;
};

Text* text_empty() {
	Text* r = malloc(sizeof(Text));
	r->data = 0;
	r->ref_count = 1;
	return r;
}

Text* text(const char* str) {
	Text* r = text_empty();
	r->data = malloc(strlen(str)+1);
	strcpy(r->data, str);
	return r;
}

Text* text_append(const Text* a, const Text* b) {
	Text* r = text_empty();
	r->data = malloc(strlen(a->data) + strlen(b->data) + 1);
	strcpy(r->data, a->data);
	strcat(r->data, b->data);
	return r;
}

int text_length(const Text* text) {
	return strlen(text->data);
}

void text_to_string(const Text* txt, char* out) {
	strcpy(out, txt->data);
}

void text_ref(Text* text) {
	++text->ref_count;
}

void text_unref(Text* text) {
	if (--text->ref_count == 0) {
		if (text->data) {
			free(text->data);
		}
		free(text);
	}
}
