/*
 * text.h
 *
 *  Created on: 26/04/2013
 *      Author: Clinton
 */

#ifndef TEXT_H_
#define TEXT_H_

typedef struct _Text Text;

Text* text_empty();
Text* text(const char* str);
Text* text_append(const Text* a, const Text* b);
int text_length(const Text* a);
void text_to_string(const Text* txt, char* out);
void text_ref(Text* text);
void text_unref(Text* text);

#endif /* TEXT_H_ */
