#include <stdio.h>
#include "expression.h"
#include "simplifier.h"

int main(int argc, char *argv[]) {
	if (argc != 3) {
		eprintf("Usage: %s <input_file> <output_file>\n", argv[0]);
		return 1;
	}

	const char *input_file = argv[1];
	const char *output_file = argv[2];

	struct expression input_expr = {0};
	struct expression simplified_expr = {0};

	if (expression_load(&input_expr, input_file)) {
		log_error("Failed to load expression from file %s", input_file);
		return 1;
	}

	if (expression_simplify(&input_expr, &simplified_expr)) {
		log_error("Failed to simplify expression");
		expression_dtor(&input_expr);
		return 1;
	}

	if (expression_store(&simplified_expr, output_file)) {
		log_error("Failed to store simplified expression to file %s", output_file);
		expression_dtor(&input_expr);
		expression_dtor(&simplified_expr);
		return 1;
	}

	printf("Read tree from '%s', simplified it, and wrote to '%s'\n", input_file, output_file);

	FILE *dump_file = fopen("middleend_dump.htm", "w");
	tree_dump(&input_expr.tree, (struct tree_dump_params) {
		.out_stream = dump_file,
		.drawing_filename = "uwu.png",
		.idx = 0,
		.serializer = expression_serializer,
	});
	tree_dump(&simplified_expr.tree, (struct tree_dump_params) {
		.out_stream = dump_file,
		.drawing_filename = "uwu2.png",
		.idx = 1,
		.serializer = expression_serializer,
	});
	fclose(dump_file);

	expression_dtor(&input_expr);
	expression_dtor(&simplified_expr);

	return 0;
}
