#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "tree.h"

#include "expression.h"

static void pv_free_variable(void *el) {
	struct expression_variable *var = el;
	free(var->var_name);
	var->var_name = NULL;
}

int expression_ctor(struct expression *expr) {
	assert (expr);

	if (tree_ctor(&expr->tree)) {
		return S_FAIL;
	}

	if (pvector_init(&expr->variables, sizeof(struct expression_variable))) {
		tree_dtor(&expr->tree);
		return S_FAIL;
	}

	if (pvector_set_element_destructor(&expr->variables, pv_free_variable)) {
		tree_dtor(&expr->tree);
		pvector_destroy(&expr->variables);
		return S_FAIL;
	}

	return S_OK;
}

int expression_dtor(struct expression *expr) {
	assert (expr);

	tree_dtor(&expr->tree);
	pvector_destroy(&expr->variables);

	return S_OK;
}

struct expression_variable *expr_find_variable(struct expression *expr,
						      const char *varname) {
	assert (expr);
	assert (varname);

	for (size_t i = 0; i < expr->variables.len; i++) {
      		struct expression_variable *var = NULL;
		if (pvector_get(&expr->variables, i, (void **)&var)) {
			log_error("pvector_get error (normally unreachable)");
			return NULL;
		}

		if (!strcmp(varname, var->var_name)) {
			return var;
		}
	}

	return NULL;
}

int expr_push_variable(struct expression *expr, const char *varname,
			       struct expression_variable **nvar) {
	assert (expr);
	assert (varname);

	if (expr_find_variable(expr, varname)) {
		log_error("Already declared variable: %s", varname);
		return S_FAIL;
	}

	size_t var_idx = expr->variables.len;

	struct expression_variable var = {
		.var_name = strdup(varname),
		.var_pointer = var_idx,
	};

	if (!var.var_name) {
		log_error("strdup: Allocation error");
		return S_FAIL;
	}

	if (pvector_push_back(&expr->variables, &var)) {
		log_error("pvector_push_back: Allocation error");
		free(var.var_name);
		return S_FAIL;
	}

	struct expression_variable *rvar = NULL;
	if (pvector_get(&expr->variables, var_idx, (void **)&rvar)) {
		log_error("pvector_get error (normally unreachable)");
		return S_FAIL;
	}

	if (nvar) {
		*nvar = rvar;
	}

	return S_OK;
}

int expression_load(struct expression *expr, const char *filename) {
	assert (expr);
	assert (filename);

	expression_ctor(expr);

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

static int expr_parse_var(const char *text, const char **text_end_ptr) {
	assert (text);
	assert (text_end_ptr);

	const char *text_end = text;

	if (!isalpha(*text)) {
		return S_FAIL;
	}

	text_end++;

	while (isalpha(*text_end) || isdigit(*text_end)) {
		text_end++;
	}

	*text_end_ptr = text_end;
	
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

	
	if (expr_parse_var(str, &var_endpt) || *var_endpt != '\0') {
		return DS_INVALID_ARG;
	}
	
	struct expression_variable *var = expr_find_variable(expr, str);
	if (!var) {
		if (expr_push_variable(expr, str, &var)) {
			return DS_ALLOCATION;
		}
	}

	value->varname = var->var_name;
	value->flags = EXPRESSION_F_VARIABLE;
	return DS_OK;
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
		struct expression_operator *op = value.ptr;
		if (!ctx && op->name && (op->name[0] == '<' || op->name[0] == '>')) {
			if (op->name[0] == '<') {
				fprintf(out_stream, "&lt;");
			} else {
				fprintf(out_stream, "&gt;");
			}
		} else {
			fprintf(out_stream, "%s", op->name);
		}
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
