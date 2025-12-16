#include <gtest/gtest.h>

#include "expression.h"

TEST(Parser, ParserDumps) {
	const char *rawText = ";;;;;2+2;1+21;asdf;a:=b;a=b;a-b;a=a+b;";//"2+2^5^3*2/1+2-2;2;2;2;2;";

	struct expression expr = {0};
	ASSERT_EQ(expression_parse_str(rawText, &expr), S_OK);

	FILE *dump_file = fopen("tree_dump.htm", "w");
	tree_dump(&expr.tree, (struct tree_dump_params) {
		.out_stream = dump_file,
		.drawing_filename = "uwu.png",
		.idx = 0,
		.serializer = expression_serializer,
	});
	fclose(dump_file);

	expression_dtor(&expr);
}

