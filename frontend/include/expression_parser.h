#ifndef EXPRESSION_PARSER_H
#define EXPRESSION_PARSER_H

#include "expression.h"
#include "lexer.h"

#ifdef __cplusplus
extern "C" {
#endif

int expression_parse_lexer(struct expression *expr, struct lexer *lexer);
int expression_parse_str(const char *str, struct expression *expr);
int expression_parse_file(const char *filename, struct expression *expr);

#ifdef __cplusplus
}
#endif

#endif /* EXPRESSION_PARSER_H */
