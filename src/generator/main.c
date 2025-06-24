#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef struct String {
	char *data;
	size_t len;
	size_t cap;
} String;

static void stringAppendChar(String *s, char c) {
	if (s->len == s->cap) {
		s->cap += 100;
		s->data = realloc(s->data, s->cap);
	}

	s->data[s->len++] = c;
}

static void stringFree(String *s) {
	free(s->data);
	s->data = NULL;
	s->len = 0;
	s->cap = 0;
}

int main(int argc, char **argv) {
// Первая строка - название статьи. Разместить его в title и h1
	char *title = NULL;
	size_t title_len = 0;
	getline (&title, &title_len, stdin);
	if (NULL == title) {
		fputs("Error reading title", stderr);
		return 1;
	}
	title[strlen(title) - 1] = '\0';
	printf(
		"<!DOCTYPE html>"
		"<html>"
		"<head>"
		  "<title>%s</title>"
		  "<meta http-equiv='Content-Type' content='text/html; charset=UTF-8' />"
		  "<link rel='stylesheet' href='style.css'>"
		"</head>"
		"<body>"
		"<h1>%s</h1>", title, title);
	free(title);

// Внутри тегов ничего не меняем
// Если строка начинается не с <, добавляем перед ней <p>. Пустые строки заменяются на <p><br>
// <a>link</a> --> <a href=link>link</a> - чтобы не дублировать ссылку, а значит не допускать ошибки несовпадения
	enum {
		st_StartOfLine,
		st_MiddleOfLine,
		st_StartOfTag,
		st_MiddleOfTag,
		st_UnderTag,
		st_TagA
	} state = st_StartOfLine;
	int tagLevel = 0;
	bool a = false; // Признак, что обрабатывается тег <a>
	String link = {}; // Строка между <a> и </a>, она будет вставлена в href
	for (int c; (c = getc(stdin)) != EOF;) {
		switch (state) {
		case st_StartOfLine:
			if ('<' == c)
				state = st_StartOfTag;
			else {
				printf("<p>");
				if ('\n' == c)
					printf("<br>");
				else
					state = st_MiddleOfLine;
			}
			break;
		case st_MiddleOfLine:
			if ('\n' == c)
				state = st_StartOfLine;
			else if ('<' == c)
				state = st_StartOfTag;
			break;
		case st_StartOfTag:
			if ('/' != c) {
				tagLevel++;
				a = ('a' == c);
			} else {
				tagLevel--;
				if (tagLevel < 0) {
					fputs("Unexpected closing tag", stderr);
					return 1;
				}
			}

			state = st_MiddleOfTag;
			break;
		case st_MiddleOfTag:
			if ('>' != c)
				a = false;
			else {
				if (a) {
					state = st_TagA;
					continue;
				} else
					state = (0 == tagLevel) ? st_MiddleOfLine : st_UnderTag;
			}

			break;
		case st_UnderTag:
			if ('<' == c)
				state = st_StartOfTag;
			break;
		case st_TagA:
			if ('<' == c) { // Полагаем, что это закрывающий тег </a>. В идеале в этом надо убедиться
				a = false;
				stringAppendChar(&link, '\0');
				printf(" href='%s'>%s", link.data, link.data);
				stringFree(&link);
				state = st_StartOfTag;
			} else {
				stringAppendChar(&link, c);
				continue;
			}
		}

		putc(c, stdout);
	}

	puts(
		"<script type='text/javascript'>"
		"const copyButtonLabel = 'Копировать';"

		"document.querySelectorAll('pre').forEach(function (codeBlock) {"
			"var button = document.createElement('button');"
			//button.className = 'copy-code-button';
			"button.type = 'button';"
			"button.setAttribute('data-clipboard-text', codeBlock.innerText);"
			"button.innerText = copyButtonLabel;"
			// var pre = codeBlock.parentNode;
			// codeBlock.classList.add('prettyprint');
			// pre.parentNode.insertBefore(button, pre);
			// handle click event
			"button.addEventListener('click', async () => {"
				"await copyCode(codeBlock, button);"
			"});"
			"codeBlock.appendChild(button);"
		"});"

		"async function copyCode(block, button) {"
			"await navigator.clipboard.writeText(button.getAttribute('data-clipboard-text'));"

			// visual feedback that task is completed
			"button.innerText = 'Скопировано';"

			"setTimeout(() => {"
				"button.innerText = copyButtonLabel;"
			"}, 700);"
		"}"
		"</script>"
		"</body></html>");
	return 0;
}
