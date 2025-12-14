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
};

struct lexer_token {
	char *word;

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

#ifdef __cplusplus
}
#endif

#endif /* LEXER_H */
