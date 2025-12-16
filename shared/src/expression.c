#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "tree.h"

#include "expression.h"

int expression_ctor(struct expression *expr) {
	assert (expr);

	if (tree_ctor(&expr->tree)) {
		return S_FAIL;
	}

	if (LEXER_STATUS(lexer_ctor(&expr->lexer_ctx))) {
		tree_dtor(&expr->tree);
		return S_FAIL;
	}

	return S_OK;
}

int expression_dtor(struct expression *expr) {
	assert (expr);

	tree_dtor(&expr->tree);
	lexer_dtor(&expr->lexer_ctx);

	return S_OK;
}

int expression_load(struct expression *expr, const char *filename) {
	assert (expr);
	assert (filename);

	tree_dtor(&expr->tree);

	if (tree_load(&expr->tree, filename, expression_deserializer, expr)) {
		return S_FAIL;
	}

	return S_OK;
}

int expression_store(struct expression *expr, const char *filename) {
	assert (expr);
	assert (filename);

	if (tree_store(&expr->tree, filename, expression_serializer, expr)) {
		return S_FAIL;
	}

	return S_OK;
}

DSError_t expression_deserializer(tree_dtype *value, const char *str, void *ctx) {
	assert (value);
	assert (str);
	
	if (!ctx) {
		return DS_INVALID_POINTER;
	}

	struct expression *expr = ctx;

	char *endptr = NULL;
	long long snum = strtol(str, &endptr, 10);

	if (*endptr == '\0' && *str != '\0') {
		value->snum = snum;
		value->flags = EXPRESSION_F_NUMBER;
		return DS_OK;
	}

	const struct expression_operator *const *derop_ptr = expression_operators;
	while (*derop_ptr != NULL) {
		if (!strcmp(str, (*derop_ptr)->name)) {
			value->ptr = (void *)(*derop_ptr);
			value->flags = EXPRESSION_F_OPERATOR;

			return DS_OK;
		}

		derop_ptr++;
	}

	const char *var_endpt = NULL;
	struct lexer_token token = {0};
	LexerStatus parser_status = lexer_parse_var(&expr->lexer_ctx, str, &var_endpt, &token);
	if (LEXER_STATUS(parser_status) == LXST_OK && *var_endpt == '\0') {
		value->varname = token.word;
		value->flags = EXPRESSION_F_VARIABLE;
		return DS_OK;
	}

	return DS_INVALID_ARG;
}

DSError_t expression_serializer(tree_dtype value, FILE *out_stream, void *ctx) {
	assert (out_stream);

	if ((value.flags & EXPRESSION_F_OPERATOR) == EXPRESSION_F_NUMBER) {
		fprintf(out_stream, "%ld", value.snum);
		return DS_OK;
	}

	if ((value.flags & EXPRESSION_F_OPERATOR) == EXPRESSION_F_VARIABLE) {
		fprintf(out_stream, "%s", value.varname);
		return DS_OK;
	}

	if ((value.flags & EXPRESSION_F_OPERATOR) == EXPRESSION_F_OPERATOR) {
		fprintf(out_stream, "%s", ((struct expression_operator *)value.ptr)->name);
		return DS_OK;
	}

	return DS_INVALID_ARG;
}

struct tree_node *expr_create_number_tnode(int64_t snum) {
	struct tree_node *node = tnode_ctor();

	if (!node)
		return NULL;

	node->value.snum = snum;
	node->value.flags = EXPRESSION_F_NUMBER;
	node->left = NULL;
	node->right = NULL;

	return node;
}

struct tree_node *expr_create_variable_tnode(const char *varname) {
	struct tree_node *node = tnode_ctor();

	if (!node)
		return NULL;

	node->value.varname = varname;
	node->value.flags = EXPRESSION_F_VARIABLE;
	node->left = NULL;
	node->right = NULL;

	return node;
}

struct tree_node *expr_create_operator_tnode(const struct expression_operator *op, 
                                              struct tree_node *left, 
                                              struct tree_node *right) {
	struct tree_node *node = tnode_ctor();
	if (!node)
		return NULL;

	node->value.ptr = (void*)op;
	node->value.flags = EXPRESSION_F_OPERATOR;
	node->left = left;
	node->right = right;

	return node;
}

struct tree_node *expr_copy_tnode(struct expression *expr, struct tree_node *original) {
	assert (original);

	struct tree_node *copy = tnode_ctor();
	if (!copy)
		return NULL;

	copy->value = original->value;

	if (original->left) {
		copy->left = expr_copy_tnode(expr, original->left);

		if (!copy->left) {
			tnode_recursive_dtor(copy, NULL);
			return NULL;
		}
	}

	if (original->right) {
		copy->right = expr_copy_tnode(expr, original->right);

		if (!copy->right) {
			tnode_recursive_dtor(copy, NULL);
			return NULL;
		}
	}

	return copy;
}
