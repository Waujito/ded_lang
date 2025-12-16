#ifndef LEXER_H
#define LEXER_H

#include "types.h"
#include "pvector.h"

#ifdef __cplusplus
extern "C" {
#endif


enum LexerTokenType {
	LXTOK_NUMBER,		// 1234
	LXTOK_VARIABLE,		// asdfASDF1234
	LXTOK_BCURLY_OPEN,	// {
	LXTOK_BCURLY_CLOSE,	// }
	LXTOK_BROUND_OPEN,	// (
	LXTOK_BROUND_CLOSE,	// )
	LXTOK_PLUS,		// +
	LXTOK_MINUS,		// -
	LXTOK_MULTIPLY,		// *
	LXTOK_DIVIDE,		// /
	LXTOK_POW,		// ^
	LXTOK_ASSIGN,		// =
	LXTOK_EQUALS_CMP,	// ==
	LXTOK_SEMICOLON,	// ;
	LXTOK_COMMA,		// ,
	LXTOK_GREATER_CMP,	// >
	LXTOK_LESS_CMP,		// <
	LXTOK_DECL_ASSIGN,	// :=
};

struct lexer_token {
	union {
		char *word;
		int64_t lexer_number;
	};

	enum LexerTokenType tok_type;
	size_t text_position;
};

struct lexer {
	struct pvector tokens;

	char *words_buf;
	size_t words_bufidx;
	size_t words_buflen;
};

enum LexerStatusType {
	LXST_OK = 0,
	LXST_INTERNAL_FAILURE = 1,
	LXST_ALLOCATION = 2,
	LXST_NOT_MATCHED_TOKEN = 3,
	LXST_VARIABLE_DIGIT_BEGINNING = 4,
	LXST_OK_NO_TOKEN = 5,
};

typedef struct LexerStatus {
	enum LexerStatusType status;
	ssize_t text_position;
} LexerStatus;

#define LEXER_STATUS(status_) ((status_).status)

LexerStatus lexer_ctor(struct lexer *lexer);

LexerStatus lexer_dtor(struct lexer *lexer);

LexerStatus lexer_parse_text(struct lexer *lexer, const char *text);


LexerStatus lexer_clone(struct lexer *old, struct lexer *new);

struct lexer_token *lexer_get_token(struct lexer *lexer, size_t tok_idx);

LexerStatus lexer_copy_token_word(
	struct lexer *lexer, const char *token_name, size_t token_size,
	char **token_copied);

LexerStatus lexer_parse_var(struct lexer *lexer,
			const char *text, const char **text_end_ptr,
			struct lexer_token *token);

#ifdef __cplusplus
}
#endif

#endif /* LEXER_H */
