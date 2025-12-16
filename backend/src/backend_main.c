#include <stdio.h>
#include "expression.h"
#include "backend.h"

int main(int argc, const char *argv[]) {
	if (argc != 3) {
		eprintf("Usage: %s <input_file> <output_file>\n", argv[0]);
		return 1;
	}

	const char *input_file = argv[1];
	const char *output_file = argv[2];

	struct expression input_expr = {0};

	if (expression_load(&input_expr, input_file)) {
		log_error("Failed to load expression from file %s", input_file);
		return 1;
	}

	FILE *asm_file = fopen(output_file, "w");
	if (!asm_file) {
		log_error("Failed to load destination file %s", output_file);
		expression_dtor(&input_expr);
		return 1;
	}

	TranslatorStatus status = backend_translator(&input_expr, asm_file);
	if (TRANSLATOR_STATUS(status)) {
		log_error("Translation failed with status: %u", TRANSLATOR_STATUS(status));

		expression_dtor(&input_expr);
		fclose(asm_file);
		return 1;
	}

	expression_dtor(&input_expr);
	fclose(asm_file);
	return 0;
}
