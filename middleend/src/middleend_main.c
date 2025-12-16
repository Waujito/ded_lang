#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
		eprintf("Error: Failed to load expression from file '%s'\n", input_file);
		return 1;
	}

	if (expression_simplify(&input_expr, &simplified_expr)) {
		eprintf("Error: Failed to simplify expression\n");
		expression_dtor(&input_expr);
		return 1;
	}

	if (expression_store(&simplified_expr, output_file)) {
		eprintf("Error: Failed to store simplified expression to file '%s'\n", output_file);
		expression_dtor(&input_expr);
		expression_dtor(&simplified_expr);
		return 1;
	}

	printf("Successfully read tree from '%s', simplified it, and wrote to '%s'\n", input_file, output_file);

	expression_dtor(&input_expr);
	expression_dtor(&simplified_expr);

	return 0;
}
