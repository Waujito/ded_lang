#include <assert.h>
#include <ctype.h>
#include <string.h>

#include "lexer.h"

#define LEXER_INITIAL_BUFLEN (128)

#define LEXER_STATUS_GEN(status_) \
	((struct LexerStatus) {.status = status_, .text_position = -1})

LexerStatus lexer_ctor(struct lexer *lexer) {
	assert (lexer);

	DSError_t ret = DS_OK;

	if ((ret = pvector_init(&lexer->tokens, sizeof(struct lexer_token)))) {
		return LEXER_STATUS_GEN(LXST_INTERNAL_FAILURE);
	}

	lexer->words_buflen = 0;
	lexer->words_bufidx = 0;
	lexer->words_buf = NULL;

	return LEXER_STATUS_GEN(LXST_OK);
}

LexerStatus lexer_dtor(struct lexer *lexer) {
	assert (lexer);

	pvector_destroy(&lexer->tokens);

	free(lexer->words_buf);
	lexer->words_buf = NULL;

	lexer->words_buflen = 0;
	lexer->words_bufidx = 0;

	return LEXER_STATUS_GEN(LXST_OK);
}

LexerStatus lexer_clone(struct lexer *old, struct lexer *new) {
	assert (old);
	assert (new);

	char *wordsbuf_clone = calloc(old->words_buflen, 1);
	if (!wordsbuf_clone) {
		return LEXER_STATUS_GEN(LXST_ALLOCATION);
	}

	new->words_buflen	= old->words_buflen;
	new->words_bufidx	= old->words_bufidx;
	new->words_buf		= wordsbuf_clone;

	if (pvector_clone(&new->tokens, &old->tokens)) {
		free (wordsbuf_clone);
		return LEXER_STATUS_GEN(LXST_ALLOCATION);
	}

	return LEXER_STATUS_GEN(LXST_OK);
}

LexerStatus lexer_copy_token_word(
	struct lexer *lexer, const char *token_name, size_t token_size,
	char **token_copied) {

	assert (lexer);
	assert (token_name);
	assert (token_size > 0);
	assert (token_copied);
	
	if (lexer->words_bufidx + token_size + 1 > lexer->words_buflen) {
		size_t new_wordsbuf_sz = lexer->words_buflen * 2;

		if (new_wordsbuf_sz < LEXER_INITIAL_BUFLEN) {
			new_wordsbuf_sz = LEXER_INITIAL_BUFLEN;
		}
		if (lexer->words_bufidx + token_size + 1 > new_wordsbuf_sz) {
			new_wordsbuf_sz = lexer->words_bufidx + token_size + 1;
		}

		void *new_lexer_buf = realloc(lexer->words_buf, new_wordsbuf_sz);
		if (!new_lexer_buf) {
			return LEXER_STATUS_GEN(LXST_ALLOCATION);
		}

		lexer->words_buf = new_lexer_buf;
		lexer->words_buflen = new_wordsbuf_sz;
	}

	memcpy(lexer->words_buf + lexer->words_bufidx, token_name, token_size);
	lexer->words_buf[lexer->words_bufidx + token_size] = '\0';

	char *word_ptr = lexer->words_buf + lexer->words_bufidx;

	lexer->words_bufidx += token_size + 1;

	
	*token_copied = word_ptr;

	return LEXER_STATUS_GEN(LXST_OK);
}

static LexerStatus lexer_add_token(struct lexer *lexer, struct lexer_token *token) {
	assert (lexer);
	assert (token);

	DSError_t ret = DS_OK;
	if ((ret = pvector_push_back(&lexer->tokens, token))) {
		return LEXER_STATUS_GEN(LXST_ALLOCATION);
	}

	return LEXER_STATUS_GEN(LXST_OK);
}

static LexerStatus lexer_parse_number(struct lexer *lexer,
			const char *text, const char **text_end_ptr,
			struct lexer_token *token) {
	assert (lexer);
	assert (text);
	assert (text_end_ptr);
	assert (token);

	const char *text_end = text;

	if (!isdigit(*text_end)) {
		return LEXER_STATUS_GEN(LXST_NOT_MATCHED_TOKEN);
	}

	char *strte = NULL;
	long long num = strtoll(text, &strte, 0);
	text_end = strte;

	*text_end_ptr = text_end;

	if (isalpha(*text_end)) {
		return LEXER_STATUS_GEN(LXST_VARIABLE_DIGIT_BEGINNING);
	}

	(*token).tok_type = LXTOK_NUMBER;
	(*token).lexer_number = num;

	return LEXER_STATUS_GEN(LXST_OK);
}

LexerStatus lexer_parse_var(struct lexer *lexer,
			const char *text, const char **text_end_ptr,
			struct lexer_token *token) {
	assert (lexer);
	assert (text);
	assert (text_end_ptr);
	assert (token);

	const char *text_end = text;

	if (!isalpha(*text)) {
		return LEXER_STATUS_GEN(LXST_NOT_MATCHED_TOKEN);
	}

	text_end++;

	while (isalpha(*text_end) || isdigit(*text_end)) {

		text_end++;
	}

	*text_end_ptr = text_end;

	char *copied_word = NULL;
	struct LexerStatus copy_status = lexer_copy_token_word(lexer, text,
				(size_t) (text_end - text), &copied_word);
	if (LEXER_STATUS(copy_status)) {
		return copy_status;
	}

	(*token).tok_type = LXTOK_VARIABLE;
	(*token).word = copied_word;


	return LEXER_STATUS_GEN(LXST_OK);
}

static LexerStatus lexer_parse_operator(struct lexer *lexer,
			const char *text, const char **text_end_ptr,
			struct lexer_token *token) {
	assert (lexer);
	assert (text);
	assert (text_end_ptr);
	assert (token);

	const char *text_cur_ptr = text;

	enum LexerTokenType token_type = 0;

	switch (*text) {
#define LXCASE_TOK_TYPE_(op_name, tok_type)	\
		case op_name:			\
			token_type = tok_type;	\
			break;			\

		LXCASE_TOK_TYPE_('{', LXTOK_BCURLY_OPEN);
		LXCASE_TOK_TYPE_('}', LXTOK_BCURLY_CLOSE);
		LXCASE_TOK_TYPE_('(', LXTOK_BROUND_OPEN);
		LXCASE_TOK_TYPE_(')', LXTOK_BROUND_CLOSE);
		LXCASE_TOK_TYPE_('+', LXTOK_PLUS);
		LXCASE_TOK_TYPE_('-', LXTOK_MINUS);
		LXCASE_TOK_TYPE_('*', LXTOK_MULTIPLY);
		LXCASE_TOK_TYPE_('/', LXTOK_DIVIDE);
		LXCASE_TOK_TYPE_('^', LXTOK_POW);	
		LXCASE_TOK_TYPE_(';', LXTOK_SEMICOLON);
		LXCASE_TOK_TYPE_(',', LXTOK_COMMA);
		LXCASE_TOK_TYPE_('>', LXTOK_GREATER_CMP);
		LXCASE_TOK_TYPE_('<', LXTOK_LESS_CMP);

		case '=':
			token_type = LXTOK_ASSIGN;
			if (*(text_cur_ptr + 1) == '=') {
				text_cur_ptr++;
				token_type = LXTOK_EQUALS_CMP;
			}
			break;

		case ':':
			if (*(text_cur_ptr + 1) == '=') {
				text_cur_ptr++;
				token_type = LXTOK_DECL_ASSIGN;
			} else {
				return LEXER_STATUS_GEN(LXST_NOT_MATCHED_TOKEN);
			}
			break;

#undef LXCASE_TOK_TYPE_
		default:
			return LEXER_STATUS_GEN(LXST_NOT_MATCHED_TOKEN);
	}

	text_cur_ptr++;

	(*token).word = NULL;
	(*token).tok_type = token_type;

	*text_end_ptr = text_cur_ptr;

	return LEXER_STATUS_GEN(LXST_OK);
}

static LexerStatus lexer_parse_comments(struct lexer *lexer,
			const char *text, const char **text_end_ptr,
			struct lexer_token *token) {
	assert (lexer);
	assert (text);
	assert (text_end_ptr);
	assert (token);

	const char *text_cur_ptr = text;

	if (*text_cur_ptr == '/' && *(text_cur_ptr + 1) == '/') {
		text_cur_ptr += 2;

		while (*text_cur_ptr != '\n' && *text_cur_ptr != '\0') {
			text_cur_ptr++;
		}

		*text_end_ptr = text_cur_ptr;

		return LEXER_STATUS_GEN(LXST_OK_NO_TOKEN);

	}

	return LEXER_STATUS_GEN(LXST_NOT_MATCHED_TOKEN);
}

static LexerStatus lexer_parse_token(struct lexer *lexer,
			const char *text, const char **text_end_ptr,
			struct lexer_token *token) {
	assert (lexer);
	assert (text);
	assert (text_end_ptr);
	assert (token);

	LexerStatus parser_status = {0};	

	parser_status = lexer_parse_comments(lexer, text, text_end_ptr, token);
	if (LEXER_STATUS(parser_status) != LXST_NOT_MATCHED_TOKEN) {
		return parser_status;
	}

	parser_status = lexer_parse_number(lexer, text, text_end_ptr, token);
	if (LEXER_STATUS(parser_status) != LXST_NOT_MATCHED_TOKEN) {
		return parser_status;
	}

	parser_status = lexer_parse_var(lexer, text, text_end_ptr, token);
	if (LEXER_STATUS(parser_status) != LXST_NOT_MATCHED_TOKEN) {
		return parser_status;
	}	

	parser_status = lexer_parse_operator(lexer, text, text_end_ptr, token);
	if (LEXER_STATUS(parser_status) != LXST_NOT_MATCHED_TOKEN) {
		return parser_status;
	}

	return LEXER_STATUS_GEN(LXST_NOT_MATCHED_TOKEN);
}

LexerStatus lexer_parse_text(struct lexer *lexer,
			const char *text) {
	assert (lexer);
	assert (text);

	const char *text_cur_ptr = text;

	for (;;) {
		while (isspace(*text_cur_ptr)) {
			text_cur_ptr++;
		}
		if (*text_cur_ptr == '\0') {
			break;
		}

		const char *text_end_ptr = NULL;
		struct lexer_token token = {0};

		token.text_position = (size_t) (text_cur_ptr - text);

		LexerStatus parser_status = {0};

		parser_status = lexer_parse_token(lexer, text_cur_ptr,
							&text_end_ptr, &token);
		parser_status.text_position = text_cur_ptr - text;
		if (	LEXER_STATUS(parser_status) != LXST_OK && 
			LEXER_STATUS(parser_status) != LXST_OK_NO_TOKEN) {
			return parser_status;
		}

		if (text_end_ptr == text_cur_ptr) {
			parser_status.status = LXST_NOT_MATCHED_TOKEN;
			return parser_status;
		}

		if (LEXER_STATUS(parser_status) != LXST_OK_NO_TOKEN) {
			parser_status = lexer_add_token(lexer, &token);
			if (LEXER_STATUS(parser_status)) {
				return parser_status;
			}
		}

		text_cur_ptr = text_end_ptr;
	}

	return LEXER_STATUS_GEN(LXST_OK);
}

struct lexer_token *lexer_get_token(struct lexer *lexer, size_t tok_idx) {
	assert (lexer);

	struct lexer_token *tok = NULL;
	if (pvector_get(&lexer->tokens, tok_idx, (void **)&tok)) {
		return NULL;
	}

	return tok;
}
