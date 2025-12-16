#ifndef BACKEND_H
#define BACKEND_H

#include "types.h"
#include "expression.h"

enum TranslatorStatusType {
	BTRST_OK = 0,
	BTRST_INTERNAL_FAILURE = 1,
	BTRST_ALLOCATION = 2,
	BTRST_UNDECLARED_VARIABLE = 3,
	BTRST_TREE_INVALID = 4,
	BTRST_ALREADY_DECLARED_VAR = 5,
};

typedef struct TranslatorStatus {
	enum TranslatorStatusType status;
} TranslatorStatus;

#define TRANSLATOR_STATUS(status_) ((status_).status)

TranslatorStatus backend_translator(struct expression *expr, FILE *asm_output);

#endif /* BACKEND_H */
