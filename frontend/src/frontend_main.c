#include <string.h>
#include "types.h"
#include "expression.h"
#include "expression_parser.h"

int main(int argc, const char *argv[]) {
	if (argc < 2) {
		log_error("Frontend command syntax: %s [filename]+", argv[0]);
	}

	for (int i = 1; i < argc; i++) {
		const char *in_file = argv[i];
		struct expression expr = {0};

		if (expression_parse_file(in_file, &expr)) {
			log_error("Cannot parse file %s", in_file);
			continue;
		}

		size_t in_filename_len = strlen(in_file);
		char *out_filename = calloc(in_filename_len + 10, 1);
		if (!out_filename) {
			expression_dtor(&expr);
			log_error("allocation error");
			continue;
		}
		memcpy(out_filename, in_file, in_filename_len);
		strcpy(out_filename + in_filename_len, ".ast");

		if (expression_store(&expr, out_filename)) {
			expression_dtor(&expr);
			free(out_filename);
			log_error("Tree store error");
			continue;
		}

		expression_dtor(&expr);
		free(out_filename);
	}

	return 0;
}
