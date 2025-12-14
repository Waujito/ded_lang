#include <gtest/gtest.h>

#include "lexer/lexer.h"

TEST(Lexer, LexerOperates) {
	const char *lexerText = "mewo 2134 meo1234 m12m m 2 m m m m m 2";

	struct lexer lexer = {{0}};
	ASSERT_EQ(lexer_ctor(&lexer).status, LXST_OK);

	LexerStatus status = lexer_parse_text(&lexer, lexerText);
	ASSERT_EQ(LEXER_STATUS(status), LXST_OK);

	ASSERT_EQ(lexer.tokens.len, 12);

	struct lexer_token *ntoken = NULL;
	ASSERT_EQ(pvector_get(&lexer.tokens, 3, (void **)&ntoken), DS_OK);
	ASSERT_STREQ(ntoken->word, "m12m");

	lexer_dtor(&lexer);
}

TEST(Lexer, LexerVarDigitBeginning) {
	const char *lexerText = "mewo 2mwm";

	struct lexer lexer = {{0}};
	ASSERT_EQ(lexer_ctor(&lexer).status, LXST_OK);

	LexerStatus status = lexer_parse_text(&lexer, lexerText);
	ASSERT_EQ(LEXER_STATUS(status), LXST_VARIABLE_DIGIT_BEGINNING);
	ASSERT_EQ(status.text_position, 5);

	lexer_dtor(&lexer);
}

TEST(Lexer, LexerInlineComment) {
	const char *lexerText = "mewo 2134// uwuw owow uwero i12 1m\nmwee";

	struct lexer lexer = {{0}};
	ASSERT_EQ(lexer_ctor(&lexer).status, LXST_OK);

	LexerStatus status = lexer_parse_text(&lexer, lexerText);
	ASSERT_EQ(LEXER_STATUS(status), LXST_OK);

	ASSERT_EQ(lexer.tokens.len, 3);

	lexer_dtor(&lexer);
}

TEST(Lexer, LexerPlayground) {
	const char *lexerText = "mewo 2134 meo1234 m12m m 2 m m m m m 2+2;{1243};;{;}{//asdf  asd fs df 1mm 11m \nm1";

	struct lexer lexer = {{0}};
	ASSERT_EQ(lexer_ctor(&lexer).status, LXST_OK);

	LexerStatus status = lexer_parse_text(&lexer, lexerText);
	ASSERT_EQ(LEXER_STATUS(status), LXST_OK);

	printf("words_buflen: %zu tokens_len: %zu\n",
		lexer.words_buflen, lexer.tokens.len);

	for (size_t i = 0; i < lexer.tokens.len; i++) {
		struct lexer_token *token = NULL;
		ASSERT_EQ(pvector_get(&lexer.tokens, i, (void **)&token), DS_OK);

		printf("token: type: <%d>, text: <%s>, pos: <%zu>\n", token->tok_type, token->word, token->text_position);
	}

	lexer_dtor(&lexer);
}
