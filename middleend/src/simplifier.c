#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"
#include "expression.h"

static int64_t fastpow(int64_t num, int64_t deg) {
	if (num == 1) {
		return 1;
	}
	if (deg < 0) {
		return 0;
	}

	if (deg == 0) {
		return 1;
	}

	if (deg % 2 != 0) {
		return fastpow(num, deg - 1) * num;
	}

	int64_t nnum = fastpow(num, deg / 2);
	nnum *= nnum;

	return nnum;
}

static const double deps = 1e-9;

#define EXPR_TNODE_IS_NUMBER(node) ((node->value.flags & EXPRESSION_F_OPERATOR) \
						== EXPRESSION_F_NUMBER)
#define EXPR_TNODE_IS_VARIABLE(node) ((node->value.flags & EXPRESSION_F_OPERATOR) \
						== EXPRESSION_F_VARIABLE)
#define EXPR_TNODE_IS_OPERATOR(node) ((node->value.flags & EXPRESSION_F_OPERATOR) \
						== EXPRESSION_F_OPERATOR)
#define EXPR_TNODE_IS_CONSTANT(node) (node->value.flags & EXPRESSION_F_CONSTANT)

struct tree_node *tnode_simplify(struct expression *expr, struct tree_node *node) {

	if (!node) {
		return NULL;
	}

	if (EXPR_TNODE_IS_NUMBER(node)) {
		return expr_copy_tnode(expr, node);
	}

	struct expression_operator *op = node->value.ptr;

	struct tree_node *lnode = NULL, *rnode = NULL;
	if (node->left) {
		lnode = tnode_simplify(expr, node->left);
		if (!lnode) {
			return NULL;
		}
	}
	if (node->right) {
		rnode = tnode_simplify(expr, node->right);
	
		if (!rnode) {
			if (lnode) tnode_recursive_dtor(lnode, NULL);

			return NULL;
		}
	}

	if (lnode && rnode && EXPR_TNODE_IS_NUMBER(lnode) && EXPR_TNODE_IS_NUMBER(rnode)) {
		switch (op->idx) {
			case EXPR_IDX_MULTIPLY:
				return expr_create_number_tnode(lnode->value.snum * rnode->value.snum);
			case EXPR_IDX_PLUS:
				return expr_create_number_tnode(lnode->value.snum + rnode->value.snum);
			case EXPR_IDX_MINUS:
				return expr_create_number_tnode(lnode->value.snum - rnode->value.snum);
			case EXPR_IDX_DIVIDE:
				if (rnode->value.snum == 0) {
					eprintf("WARNING: Possible division by zero.");
					break;
				}
				return expr_create_number_tnode(lnode->value.snum / rnode->value.snum);
			case EXPR_IDX_POW:
				return expr_create_number_tnode(fastpow(
					lnode->value.snum, rnode->value.snum));
			default:
				break;
		}
	}
	
	struct tree_node *new_node = expr_create_operator_tnode(op, lnode, rnode);
	if (!new_node) {
		if (lnode) tnode_recursive_dtor(lnode, NULL);
		if (rnode) tnode_recursive_dtor(rnode, NULL);

		return NULL;
	}

	return new_node;
}

int expression_simplify(struct expression *expr, struct expression *simplified) {
	assert (expr);
	assert (simplified);

	struct tree_node *simplified_root = tnode_simplify(expr, expr->tree.root);
	if (!simplified_root && expr->tree.root) {
		return S_FAIL;
	}

	if (expression_ctor(simplified)) {
		tnode_recursive_dtor(simplified_root, NULL);
		return S_FAIL;
	}

	simplified->tree.root = simplified_root;

	if (LEXER_STATUS(lexer_clone(&expr->lexer_ctx, &simplified->lexer_ctx))) {
		tnode_recursive_dtor(simplified_root, NULL);
		return S_FAIL;
	}

	return S_OK;
}
