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

struct tree_node *tnode_simplify(struct expression *expr, struct tree_node *node) {

	if (!node) {
		return NULL;
	}

	if (EXPR_TNODE_IS_NUMBER(node) || EXPR_TNODE_IS_VARIABLE(node)) {
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

	if (lnode && rnode &&
		EXPR_TNODE_IS_NUMBER(lnode) && EXPR_TNODE_IS_NUMBER(rnode)) {
		struct tree_node *nnode = NULL;

		switch ((int)op->idx) {
			case EXPR_IDX_MULTIPLY:
				nnode = expr_create_number_tnode(
					lnode->value.snum * rnode->value.snum);
				break;
			case EXPR_IDX_PLUS:
				nnode = expr_create_number_tnode(
					lnode->value.snum + rnode->value.snum);
				break;
			case EXPR_IDX_MINUS:
				nnode = expr_create_number_tnode(
					lnode->value.snum - rnode->value.snum);
				break;
			case EXPR_IDX_DIVIDE:
				if (rnode->value.snum == 0) {
					eprintf("WARNING: Possible division by zero.\n");
					break;
				}
				nnode = expr_create_number_tnode(
					lnode->value.snum / rnode->value.snum);
				break;
			case EXPR_IDX_POW:
				nnode = expr_create_number_tnode(fastpow(
					lnode->value.snum, rnode->value.snum));
				break;
			case EXPR_IDX_LESS_CMP:
				nnode = expr_create_number_tnode(
					lnode->value.snum < rnode->value.snum);
				break;
			case EXPR_IDX_GREATER_CMP:
				nnode = expr_create_number_tnode(
					lnode->value.snum > rnode->value.snum);
				break;
			case EXPR_IDX_EQUALS_CMP:
				nnode = expr_create_number_tnode(
					lnode->value.snum == rnode->value.snum);
				break;
			case EXPR_IDX_LESS_EQ_CMP:
				nnode = expr_create_number_tnode(
					lnode->value.snum <= rnode->value.snum);
				break;
			case EXPR_IDX_GREATER_EQ_CMP:
				nnode = expr_create_number_tnode(
					lnode->value.snum >= rnode->value.snum);
				break;
			case EXPR_IDX_NOT_EQUALS_CMP:
				nnode = expr_create_number_tnode(
					lnode->value.snum != rnode->value.snum);
				break;
			default:
				break;
		}

		if (nnode) {
			if (lnode) tnode_recursive_dtor(lnode, NULL);
			if (rnode) tnode_recursive_dtor(rnode, NULL);

			return nnode;
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

	return S_OK;
}
