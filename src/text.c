/*
 * text.c
 *
 *  Created on: 26/04/2013
 *      Author: Clinton
 */

#include <stdio.h>
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

Text* text_from_int(int x) {
	Text* r = text_empty();
	r->data = malloc(30);
	sprintf(r->data, "%d", x);
	return r;
}

Text* text_from_float(float x) {
	Text* r = text_empty();
	r->data = malloc(30);
	sprintf(r->data, "%f", x);
	return r;
}

Text* text_append(const Text* a, const Text* b) {
	Text* r = text_empty();
	r->data = malloc(strlen(a->data) + strlen(b->data) + 1);
	strcpy(r->data, a->data);
	strcat(r->data, b->data);
	text_unref((Text*)a);
	text_unref((Text*)b);
	return r;
}

Text* text_append_many(const Text** texts, int count) {
	if (count == 0) { return text_empty(); }
	int total_size = 0;
	for (int i = 0; i < count; ++i) {
		total_size += text_length(texts[i]);
	}
	Text* r = text_empty();
	r->data = malloc(total_size + 1);
	strcpy(r->data, texts[0]->data);
	for (int i = 1; i < count; ++i) {
		strcat(r->data, texts[i]->data);
	}
	for (int i = 0; i < count; ++i) {
		text_unref((Text*)texts[i]);
	}
	return r;
}

Text* text_indent_lines(const Text* text, const Text* indent) {
	int lineCount = 1;
	for (int i = 0; i < strlen(text->data); ++i) {
		if (text->data[i] == '\n') { ++lineCount; }
	}
	const Text* lines[lineCount];
	const char* atText = text->data;
	for (int i = 0; i < lineCount; ++i) {
		const char* atNext = strstr(atText, "\n");
		if (atNext == 0) {
			atNext = &atText[strlen(atText)];
		} else {
			++atNext;
		}
		Text* t = text_empty();
		t->data = malloc(atNext - atText + 1);
		strncpy(t->data, atText, atNext - atText);
		t->data[atNext - atText] = 0;
		text_ref((Text*)indent);
		lines[i] = text_append(indent, t);
		atText = atNext;
	}
	text_unref((Text*)text);
	text_unref((Text*)indent);
	return text_append_many(lines, lineCount);
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
