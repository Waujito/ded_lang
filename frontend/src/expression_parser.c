#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ctio.h"
#include "types.h"
#include "expression.h"
#include "expression_parser.h"
#include "lexer.h"
#include <stdbool.h>
#include <ctype.h>

// TODO: Error reporting system

#define S_CONTINUE (-2)
#define S_EMPTY_EXPRESSION (-3)

#define CALL_PARSER(parserName, expr, lexer, s, node)		\
({								\
	int cpret_ = parserName(expr, lexer, s, node);		\
	cpret_;							\
})

#define PARSER_RET_STATUS(status)				\
({								\
	typeof(status) prs_status_ = status;			\
	if ((int)prs_status_ && (int)prs_status_ != S_CONTINUE)	\
		eprintf("%s: ", __func__);			\
	(int)prs_status_;					\
})

static int getPrimaryExpression(struct expression *expr, struct lexer *lexer,
				size_t *lexer_idx, struct tree_node **node);
static int getRoundBracketsExpression(struct expression *expr, struct lexer *lexer,
			 size_t *lexer_idx, struct tree_node **node);
static int getComprasion(struct expression *expr, struct lexer *lexer,
			 size_t *lexer_idx, struct tree_node **node);
static int getCodeBlock(struct expression *expr, struct lexer *lexer,
				  size_t *lexer_idx, struct tree_node **node);
static int getCommaSeparatedList(struct expression *expr, struct lexer *lexer,
			 size_t *lexer_idx, struct tree_node **node);

static int getNumber(struct expression *expr, struct lexer *lexer,
			 size_t *lexer_idx, struct tree_node **node) {
	assert (expr);
	assert (lexer);
	assert (lexer_idx);
	assert (node);

	struct lexer_token *tok = NULL;
	if (!((tok = lexer_get_token(lexer, *lexer_idx)) &&
		tok->tok_type == LXTOK_NUMBER)) {
		return S_CONTINUE;
	}

	(*lexer_idx)++;

	*node = expr_create_number_tnode(tok->lexer_number);

	if (!(*node)) {
		return PARSER_RET_STATUS(S_FAIL);
	}

	return PARSER_RET_STATUS(S_OK);
}

static int getVariable(struct expression *expr, struct lexer *lexer,
			 size_t *lexer_idx, struct tree_node **node) {
	assert (expr);
	assert (lexer);
	assert (lexer_idx);
	assert (node);

	struct lexer_token *tok = NULL;
	if (!((tok = lexer_get_token(lexer, *lexer_idx)) &&
		tok->tok_type == LXTOK_VARIABLE)) {
		return S_CONTINUE;
	}

	struct expression_variable *var = expr_find_variable(expr, tok->word);
	if (!var) {
		if (expr_push_variable(expr, tok->word, &var)) {
			return PARSER_RET_STATUS(S_FAIL);
		}
	}

	(*lexer_idx)++;

	*node = expr_create_variable_tnode(var->var_name);

	if (!(*node)) {
		return PARSER_RET_STATUS(S_FAIL);
	}

	return PARSER_RET_STATUS(S_OK);
}

static int getFunctionCall(struct expression *expr, struct lexer *lexer,
			 size_t *lexer_idx, struct tree_node **node) {
	assert (expr);
	assert (lexer);
	assert (lexer_idx);
	assert (node);

	struct lexer_token *tok = NULL;
	if (!((tok = lexer_get_token(lexer, *lexer_idx)) &&
		tok->tok_type == LXTOK_VARIABLE)) {
		return S_CONTINUE;
	}
	(*lexer_idx)++;

	struct expression_variable *var = expr_find_variable(expr, tok->word);
	if (!var) {
		if (expr_push_variable(expr, tok->word, &var)) {
			return PARSER_RET_STATUS(S_FAIL);
		}
	}

	if (!((tok = lexer_get_token(lexer, *lexer_idx)) &&
		tok->tok_type == LXTOK_BROUND_OPEN)) {
		(*lexer_idx)--;
		return S_CONTINUE;
	}
	(*lexer_idx)++;

	struct tree_node *call_args = NULL;
	if (!((tok = lexer_get_token(lexer, *lexer_idx)) &&
		tok->tok_type == LXTOK_BROUND_CLOSE)) {

		int ret = CALL_PARSER(getCommaSeparatedList, expr, lexer,
					lexer_idx, &call_args);
		if (ret) {
			return PARSER_RET_STATUS(ret);
		}
	}
	

	if (!((tok = lexer_get_token(lexer, *lexer_idx)) &&
		tok->tok_type == LXTOK_BROUND_CLOSE)) {
		tnode_recursive_dtor(call_args, NULL);
		return PARSER_RET_STATUS(S_FAIL);
	}
	(*lexer_idx)++;	

	struct tree_node *func_name = expr_create_variable_tnode(var->var_name);

	if (!func_name) {
		tnode_recursive_dtor(call_args, NULL);
		return PARSER_RET_STATUS(S_FAIL);
	}

	*node = expr_create_operator_tnode(expression_operators[EXPR_IDX_CALL],
						func_name, call_args);

	if (!(*node)) {
		tnode_recursive_dtor(call_args, NULL);
		tnode_recursive_dtor(func_name, NULL);
		return PARSER_RET_STATUS(S_FAIL);
	}

	return PARSER_RET_STATUS(S_OK);
}

static int getFunKeywordOp(struct expression *expr, struct lexer *lexer,
			 size_t *lexer_idx, struct tree_node **node) {
	assert (expr);
	assert (lexer);
	assert (lexer_idx);
	assert (node);

	struct lexer_token *tok = NULL;
	if (!(tok = lexer_get_token(lexer, *lexer_idx))) {
		return S_CONTINUE;
	}

	size_t op_idx = 0;
	int is_no_arg = 0;
	switch ((int)tok->tok_type) {
		case LXTOK_PRINT:
			op_idx = EXPR_IDX_PRINT;
			break;
		case LXTOK_INPUT:
			op_idx = EXPR_IDX_INPUT;
			is_no_arg = 1;
			break;
		case LXTOK_SQRT:
			op_idx = EXPR_IDX_SQRT;
			break;
		case LXTOK_RETURN:
			op_idx = EXPR_IDX_RETURN;
			break;
		case LXTOK_SCRHT:
			op_idx = EXPR_IDX_SCRHT;
			is_no_arg = 1;
			break;
		case LXTOK_SCRWT:
			op_idx = EXPR_IDX_SCRWT;
			is_no_arg = 1;
			break;
		case LXTOK_DRAW:
			op_idx = EXPR_IDX_DRAW;
			break;
		case LXTOK_MEM_READ:
			op_idx = EXPR_IDX_MEM_READ;
			break;
		default:
			return S_CONTINUE;
	}

	(*lexer_idx)++;

	struct tree_node *lnode = NULL;

	int ret = 0;
	ret = CALL_PARSER(getRoundBracketsExpression, expr,
					lexer, lexer_idx, &lnode);
	if (ret != S_OK && ret != S_EMPTY_EXPRESSION) {
		return PARSER_RET_STATUS(ret);
	}

	if (!is_no_arg && ret == S_EMPTY_EXPRESSION) {
		return PARSER_RET_STATUS(S_FAIL);
	}
	if (is_no_arg && ret != S_EMPTY_EXPRESSION) {
		return PARSER_RET_STATUS(S_FAIL);
	}

	*node = expr_create_operator_tnode(expression_operators[op_idx], lnode, NULL);

	if (!(*node)) {
		tnode_recursive_dtor(lnode, NULL);
		return PARSER_RET_STATUS(S_FAIL);
	}

	return PARSER_RET_STATUS(S_OK);
}

static int getC(struct expression *expr, struct lexer *lexer,
			 size_t *lexer_idx, struct tree_node **node) {
	assert (expr);
	assert (lexer);
	assert (lexer_idx);
	assert (node);

	int ret = 0;

	if ((ret = CALL_PARSER(getNumber, expr, lexer, lexer_idx, node)) != S_CONTINUE) {
		return PARSER_RET_STATUS(ret);
	}

	if ((ret = CALL_PARSER(getFunKeywordOp, expr, lexer, lexer_idx, node)) != S_CONTINUE) {
		return PARSER_RET_STATUS(ret);
	}

	if ((ret = CALL_PARSER(getFunctionCall, expr, lexer, lexer_idx, node)) != S_CONTINUE) {
		return PARSER_RET_STATUS(ret);
	}

	if ((ret = CALL_PARSER(getVariable, expr, lexer, lexer_idx, node)) != S_CONTINUE) {
		return PARSER_RET_STATUS(ret);
	}

	eprintf("Expression item is not detected\n");

	return PARSER_RET_STATUS(S_FAIL);
}

static int getRoundBracketsExpression(struct expression *expr, struct lexer *lexer,
			 size_t *lexer_idx, struct tree_node **node) {
	assert (expr);
	assert (lexer);
	assert (lexer_idx);
	assert (node);

	int ret = 0;

	struct lexer_token *tok = NULL;
	if ((tok = lexer_get_token(lexer, *lexer_idx)) &&
		tok->tok_type == LXTOK_BROUND_OPEN) {
		(*lexer_idx)++;

		if (!((tok = lexer_get_token(lexer, *lexer_idx)) &&
			tok->tok_type == LXTOK_BROUND_CLOSE)) {
			if ((ret = CALL_PARSER(getComprasion, expr, lexer, lexer_idx, node))) {
				return PARSER_RET_STATUS(ret);
			}

			ret = S_OK;
		} else {
			ret = S_EMPTY_EXPRESSION;
		}

		if (!((tok = lexer_get_token(lexer, *lexer_idx)) &&
			tok->tok_type == LXTOK_BROUND_CLOSE)) {
			return PARSER_RET_STATUS(S_FAIL);
		}
		(*lexer_idx)++;


		return ret;
	}

	return S_CONTINUE;
}

static int getPrimaryExpression(struct expression *expr, struct lexer *lexer,
			 size_t *lexer_idx, struct tree_node **node) {
	assert (expr);
	assert (lexer);
	assert (lexer_idx);
	assert (node);

	int ret = 0;

	struct lexer_token *tok = NULL;
	if ((tok = lexer_get_token(lexer, *lexer_idx)) && tok->tok_type == LXTOK_BROUND_OPEN) {
		(*lexer_idx)++;

		if ((ret = CALL_PARSER(getComprasion, expr, lexer, lexer_idx, node))) {
			return PARSER_RET_STATUS(ret);
		}

		if (!((tok = lexer_get_token(lexer, *lexer_idx)) &&
			tok->tok_type == LXTOK_BROUND_CLOSE)) {
			tnode_recursive_dtor(*node, NULL);
			*node = NULL;
			return PARSER_RET_STATUS(S_FAIL);
		}
		(*lexer_idx)++;

		return PARSER_RET_STATUS(S_OK);
	}

	ret = CALL_PARSER(getC, expr, lexer, lexer_idx, node);
	return PARSER_RET_STATUS(ret);
}

static int getPow(struct expression *expr, struct lexer *lexer,
			 size_t *lexer_idx, struct tree_node **node) {
	assert (expr);
	assert (lexer);
	assert (lexer_idx);
	assert (node);

	int ret = 0;

	struct tree_node *lnode = NULL;
	if ((ret = CALL_PARSER(getPrimaryExpression, expr, lexer, lexer_idx, &lnode))) {
		return PARSER_RET_STATUS(ret);
	}

	struct lexer_token *tok = NULL;
	if ((tok = lexer_get_token(lexer, *lexer_idx)) && tok->tok_type == LXTOK_POW) {
		(*lexer_idx)++;

		struct tree_node *rnode = NULL;
		if ((ret = CALL_PARSER(getPow, expr, lexer, lexer_idx, &rnode))) {
			tnode_recursive_dtor(lnode, NULL);
			return PARSER_RET_STATUS(ret);
		}

		struct tree_node *mnode = expr_create_operator_tnode(
			expression_operators[EXPR_IDX_POW], lnode, rnode);

		if (!mnode) {
			tnode_recursive_dtor(lnode, NULL);
			tnode_recursive_dtor(rnode, NULL);
			return PARSER_RET_STATUS(S_FAIL);
		}

		lnode = mnode;
	}

	*node = lnode;

	return PARSER_RET_STATUS(S_OK);
}

static int getTerm(struct expression *expr, struct lexer *lexer,
			 size_t *lexer_idx, struct tree_node **node) {
	assert (expr);
	assert (lexer);
	assert (lexer_idx);
	assert (node);

	int ret = 0;

	struct tree_node *lnode = NULL;
	if ((ret = CALL_PARSER(getPow, expr, lexer, lexer_idx, &lnode))) {
		return PARSER_RET_STATUS(ret);
	}

	struct lexer_token *tok = NULL;
	while ((tok = lexer_get_token(lexer, *lexer_idx)) &&
		((tok->tok_type == LXTOK_MULTIPLY || tok->tok_type == LXTOK_DIVIDE))) {

		(*lexer_idx)++;

		struct tree_node *rnode = NULL;
		if ((ret = CALL_PARSER(getPow, expr, lexer, lexer_idx, &rnode))) {
			tnode_recursive_dtor(lnode, NULL);
			return PARSER_RET_STATUS(ret);
		}

		struct tree_node *mnode = NULL;

		if (tok->tok_type == LXTOK_MULTIPLY) {
			mnode = expr_create_operator_tnode(
				expression_operators[EXPR_IDX_MULTIPLY], lnode, rnode);

		} else if (tok->tok_type == LXTOK_DIVIDE) {
			mnode = expr_create_operator_tnode(
				expression_operators[EXPR_IDX_DIVIDE], lnode, rnode);
		}

		if (!mnode) {
			tnode_recursive_dtor(lnode, NULL);
			tnode_recursive_dtor(rnode, NULL);
			return PARSER_RET_STATUS(S_FAIL);
		}

		lnode = mnode;
	}

	*node = lnode;

	return PARSER_RET_STATUS(S_OK);
}

static int getExpression(struct expression *expr, struct lexer *lexer,
			 size_t *lexer_idx, struct tree_node **node) {
	assert (expr);
	assert (lexer);
	assert (lexer_idx);
	assert (node);

	int ret = 0;

	struct tree_node *lnode = NULL;
	if ((ret = CALL_PARSER(getTerm, expr, lexer, lexer_idx, &lnode))) {
		return PARSER_RET_STATUS(ret);
	}

	struct lexer_token *tok = NULL;
	while ((tok = lexer_get_token(lexer, *lexer_idx)) &&
		((tok->tok_type == LXTOK_PLUS || tok->tok_type == LXTOK_MINUS))) {
		(*lexer_idx)++;

		struct tree_node *rnode = NULL;
		if ((ret = CALL_PARSER(getTerm, expr, lexer, lexer_idx, &rnode))) {
			tnode_recursive_dtor(lnode, NULL);
			return PARSER_RET_STATUS(ret);
		}

		struct tree_node *mnode = NULL;

		if (tok->tok_type == LXTOK_PLUS) {
			mnode = expr_create_operator_tnode(
				expression_operators[EXPR_IDX_PLUS], lnode, rnode);

		} else if (tok->tok_type == LXTOK_MINUS) {
			mnode = expr_create_operator_tnode(
				expression_operators[EXPR_IDX_MINUS], lnode, rnode);
		}

		if (!mnode) {
			tnode_recursive_dtor(lnode, NULL);
			tnode_recursive_dtor(rnode, NULL);
			return PARSER_RET_STATUS(S_FAIL);
		}

		lnode = mnode;
	}

	*node = lnode;

	return PARSER_RET_STATUS(S_OK);
}

static int getComprasion(struct expression *expr, struct lexer *lexer,
			 size_t *lexer_idx, struct tree_node **node) {
	assert (expr);
	assert (lexer);
	assert (lexer_idx);
	assert (node);

	int ret = 0;

	struct tree_node *lnode = NULL;
	if ((ret = CALL_PARSER(getExpression, expr, lexer, lexer_idx, &lnode))) {
		return PARSER_RET_STATUS(ret);
	}

	struct lexer_token *tok = NULL;
	while ((tok = lexer_get_token(lexer, *lexer_idx)) &&
		(tok->tok_type == LXTOK_EQUALS_CMP	||
		 tok->tok_type == LXTOK_GREATER_CMP	||
		 tok->tok_type == LXTOK_LESS_CMP	||
		 tok->tok_type == LXTOK_NOT_EQUALS_CMP	||
		 tok->tok_type == LXTOK_GREATER_EQ_CMP	||
		 tok->tok_type == LXTOK_LESS_EQ_CMP	||
		// TODO: MOVE OUT OF COMPRASION
		 tok->tok_type == LXTOK_SHL		||
		 tok->tok_type == LXTOK_SHR		||
		 tok->tok_type == LXTOK_BITAND		||
		 tok->tok_type == LXTOK_BITOR		)) {	

		(*lexer_idx)++;

		struct tree_node *rnode = NULL;
		if ((ret = CALL_PARSER(getExpression, expr, lexer, lexer_idx, &rnode))) {
			tnode_recursive_dtor(lnode, NULL);
			return PARSER_RET_STATUS(ret);
		}

		struct tree_node *mnode = NULL;
		size_t op_idx = 0;
		switch ((int)tok->tok_type) {
			case LXTOK_EQUALS_CMP:
				op_idx = EXPR_IDX_EQUALS_CMP;
				break;
			case LXTOK_GREATER_CMP:
				op_idx = EXPR_IDX_GREATER_CMP;
				break;
			case LXTOK_LESS_CMP:
				op_idx = EXPR_IDX_LESS_CMP;
				break;
			case LXTOK_NOT_EQUALS_CMP:
				op_idx = EXPR_IDX_NOT_EQUALS_CMP;
				break;
			case LXTOK_GREATER_EQ_CMP:
				op_idx = EXPR_IDX_GREATER_EQ_CMP;
				break;
			case LXTOK_LESS_EQ_CMP:
				op_idx = EXPR_IDX_LESS_EQ_CMP;
				break;
			case LXTOK_SHL:
				op_idx = EXPR_IDX_SHL;
				break;
			case LXTOK_SHR:
				op_idx = EXPR_IDX_SHR;
				break;
			case LXTOK_BITAND:
				op_idx = EXPR_IDX_BITAND;
				break;
			case LXTOK_BITOR:
				op_idx = EXPR_IDX_BITOR;
				break;
			default:
				assert (0 && "unreachable");
				break;
		}

		mnode = expr_create_operator_tnode(
				expression_operators[op_idx], lnode, rnode);
		

		if (!mnode) {
			tnode_recursive_dtor(lnode, NULL);
			tnode_recursive_dtor(rnode, NULL);
			return PARSER_RET_STATUS(S_FAIL);
		}

		lnode = mnode;
	}

	*node = lnode;

	return PARSER_RET_STATUS(S_OK);
}

static int getDeclarationOrAssignment(struct expression *expr, struct lexer *lexer,
			 size_t *lexer_idx, struct tree_node **node) {
	assert (expr);
	assert (lexer);
	assert (lexer_idx);
	assert (node);

	int ret = 0;

	struct tree_node *lnode = NULL;
	if (CALL_PARSER(getVariable, expr, lexer, lexer_idx, &lnode)) {
		return S_CONTINUE;
	}

	struct lexer_token *next_tok = lexer_get_token(lexer, *lexer_idx);
	if (!next_tok) {
		tnode_recursive_dtor(lnode, NULL);
		(*lexer_idx)--;
		return S_CONTINUE;
	}

	enum expression_op_indexes op_idx = 0;
	if (next_tok->tok_type == LXTOK_DECL_ASSIGN) {
		op_idx = EXPR_IDX_DECL_ASSIGN;
	} else if (next_tok->tok_type == LXTOK_ASSIGN) {
		op_idx = EXPR_IDX_ASSIGN;
	}  else if (next_tok->tok_type == LXTOK_MEM_WRITE) {
		op_idx = EXPR_IDX_MEM_WRITE;
	} else {
		tnode_recursive_dtor(lnode, NULL);
		(*lexer_idx)--;
		return S_CONTINUE;
	}

	(*lexer_idx)++;

	struct tree_node *rnode = NULL;
	if ((ret = CALL_PARSER(getComprasion, expr, lexer, lexer_idx, &rnode))) {
		tnode_recursive_dtor(lnode, NULL);
		return PARSER_RET_STATUS(ret);
	}	

	*node = expr_create_operator_tnode(
		expression_operators[op_idx], lnode, rnode);

	if (!(*node)) {
		tnode_recursive_dtor(lnode, NULL);
		tnode_recursive_dtor(rnode, NULL);
		return PARSER_RET_STATUS(S_FAIL);
	}

	return PARSER_RET_STATUS(S_OK);
}

static int getStatement(struct expression *expr, struct lexer *lexer,
				  size_t *lexer_idx, struct tree_node **node) {
	assert (expr);
	assert (lexer);
	assert (lexer_idx);
	assert (node);

	int ret = 0;

	struct lexer_token *tok = NULL;
	if (((tok = lexer_get_token(lexer, *lexer_idx)) &&
		tok->tok_type == LXTOK_SEMICOLON)) {
		*node = NULL;

		(*lexer_idx)++;

		return PARSER_RET_STATUS(S_OK);
	}

	if ((ret = CALL_PARSER(getDeclarationOrAssignment, expr, lexer, lexer_idx, node))
			!= S_CONTINUE) {
		if (ret) {
			return PARSER_RET_STATUS(ret);
		}
	} else if ((ret = CALL_PARSER(getComprasion, expr, lexer, lexer_idx, node))) {
		return PARSER_RET_STATUS(ret);
	}

	if (!((tok = lexer_get_token(lexer, *lexer_idx)) &&
		tok->tok_type == LXTOK_SEMICOLON)) {
		tnode_recursive_dtor(*node, NULL);
		*node = NULL;
		eprintf("Did you forget semicolon?\n");
		return PARSER_RET_STATUS(S_FAIL);
	}

	(*lexer_idx)++;

	return PARSER_RET_STATUS(S_OK);
}

static int getConditionalExpression(struct expression *expr, struct lexer *lexer,
			 size_t *lexer_idx, struct tree_node **node) {
	assert (expr);
	assert (lexer);
	assert (lexer_idx);
	assert (node);

	struct lexer_token *tok = NULL;
	if (!(tok = lexer_get_token(lexer, *lexer_idx))) {
		return S_CONTINUE;
	}
	if (tok->tok_type != LXTOK_IF) {
		return S_CONTINUE;
	}

	(*lexer_idx)++;

	int ret = 0;

	struct tree_node *if_expr_node = NULL;
	ret = getRoundBracketsExpression(expr, lexer, lexer_idx, &if_expr_node);
	if (ret) {
		return PARSER_RET_STATUS(ret);
	}

	struct tree_node *if_positive_node = NULL;
	ret = getCodeBlock(expr, lexer, lexer_idx, &if_positive_node);
	if (ret) {
		tnode_recursive_dtor(if_expr_node, NULL);
		return PARSER_RET_STATUS(ret);
	}

	struct tree_node *if_negative_node = NULL;

	if (((tok = lexer_get_token(lexer, *lexer_idx)) &&
			tok->tok_type == LXTOK_ELSE)) {
		(*lexer_idx)++;

		ret = getCodeBlock(expr, lexer, lexer_idx, &if_negative_node);
		if (ret) {
			tnode_recursive_dtor(if_expr_node, NULL);
			tnode_recursive_dtor(if_positive_node, NULL);
			return PARSER_RET_STATUS(ret);
		}
	}
	

	struct tree_node *tree_else_node = expr_create_operator_tnode(
		expression_operators[EXPR_IDX_ELSE], if_positive_node, if_negative_node);

	if (!tree_else_node) {
		tnode_recursive_dtor(if_expr_node, NULL);
		tnode_recursive_dtor(if_positive_node, NULL);
		tnode_recursive_dtor(if_negative_node, NULL);
		return PARSER_RET_STATUS(ret);
	}
	

	*node = expr_create_operator_tnode(expression_operators[EXPR_IDX_IF],
					if_expr_node, tree_else_node);

	if (!(*node)) {
		tnode_recursive_dtor(if_expr_node, NULL);
		return PARSER_RET_STATUS(S_FAIL);
	}

	return PARSER_RET_STATUS(S_OK);
}

static int getCycleExpression(struct expression *expr, struct lexer *lexer,
			 size_t *lexer_idx, struct tree_node **node) {
	assert (expr);
	assert (lexer);
	assert (lexer_idx);
	assert (node);

	struct lexer_token *tok = NULL;
	if (!(tok = lexer_get_token(lexer, *lexer_idx))) {
		return S_CONTINUE;
	}
	if (tok->tok_type != LXTOK_WHILE) {
		return S_CONTINUE;
	}

	(*lexer_idx)++;

	int ret = 0;

	struct tree_node *if_expr_node = NULL;
	ret = getRoundBracketsExpression(expr, lexer, lexer_idx, &if_expr_node);
	if (ret) {
		return PARSER_RET_STATUS(ret);
	}

	struct tree_node *if_positive_node = NULL;
	ret = getCodeBlock(expr, lexer, lexer_idx, &if_positive_node);
	if (ret) {
		tnode_recursive_dtor(if_expr_node, NULL);
		return PARSER_RET_STATUS(ret);
	}

	*node = expr_create_operator_tnode(expression_operators[EXPR_IDX_WHILE],
					if_expr_node, if_positive_node);

	if (!(*node)) {
		tnode_recursive_dtor(if_expr_node, NULL);
		return PARSER_RET_STATUS(S_FAIL);
	}

	return PARSER_RET_STATUS(S_OK);
}

static int getLangPunct(struct expression *expr, struct lexer *lexer,
				  size_t *lexer_idx, struct tree_node **node) {
	assert (expr);
	assert (lexer);
	assert (lexer_idx);
	assert (node);

	int ret = 0;

	ret = CALL_PARSER(getConditionalExpression, expr, lexer, lexer_idx, node);
	if (ret != S_CONTINUE) {
		return PARSER_RET_STATUS(ret);
	}

	ret = CALL_PARSER(getCycleExpression, expr, lexer, lexer_idx, node);
	if (ret != S_CONTINUE) {
		return PARSER_RET_STATUS(ret);
	}

	return PARSER_RET_STATUS(CALL_PARSER(getStatement, expr, lexer, lexer_idx, node));
}

static int getCodeBlock(struct expression *expr, struct lexer *lexer,
				  size_t *lexer_idx, struct tree_node **node) {
	assert (expr);
	assert (lexer);
	assert (lexer_idx);
	assert (node);

	int ret = 0;
	struct lexer_token *tok = NULL;
	if ((tok = lexer_get_token(lexer, *lexer_idx)) &&
		tok->tok_type == LXTOK_BCURLY_OPEN) {
		(*lexer_idx)++;

		struct tree_node *lnode = NULL;
		while (!((tok = lexer_get_token(lexer, *lexer_idx)) &&
			tok->tok_type == LXTOK_BCURLY_CLOSE)) {

			struct tree_node *rnode = NULL;
			if ((ret = CALL_PARSER(getLangPunct, expr, lexer, lexer_idx, &rnode))) {
				tnode_recursive_dtor(lnode, NULL);
				return PARSER_RET_STATUS(ret);
			}

			if (!rnode) {
				continue;
			}

			if (!lnode) {
				lnode = rnode;
				continue;
			}

			struct tree_node *mnode = NULL;

			mnode = expr_create_operator_tnode(
				expression_operators[EXPR_IDX_SEMICOLON], lnode, rnode);

			if (!mnode) {
				tnode_recursive_dtor(lnode, NULL);
				tnode_recursive_dtor(rnode, NULL);
				return PARSER_RET_STATUS(S_FAIL);
			}

			lnode = mnode;
		}

		if (!((tok = lexer_get_token(lexer, *lexer_idx)) &&
			tok->tok_type == LXTOK_BCURLY_CLOSE)) {
			tnode_recursive_dtor(lnode, NULL);
			return PARSER_RET_STATUS(S_FAIL);
		}
		(*lexer_idx)++;

		*node = lnode;

		return PARSER_RET_STATUS(ret);
	}

	return PARSER_RET_STATUS(CALL_PARSER(getLangPunct, expr, lexer, lexer_idx, node));
}

static int getCommaSeparatedList(struct expression *expr, struct lexer *lexer,
			 size_t *lexer_idx, struct tree_node **node) {
	assert (expr);
	assert (lexer);
	assert (lexer_idx);
	assert (node);

	int ret = 0;

	struct tree_node *lnode = NULL;
	if ((ret = CALL_PARSER(getComprasion, expr, lexer, lexer_idx, &lnode))) {
		return PARSER_RET_STATUS(ret);
	}

	struct lexer_token *tok = NULL;
	while ((tok = lexer_get_token(lexer, *lexer_idx)) &&
		tok->tok_type == LXTOK_COMMA) {
		(*lexer_idx)++;

		struct tree_node *rnode = NULL;
		if ((ret = CALL_PARSER(getComprasion, expr, lexer, lexer_idx, &rnode))) {
			tnode_recursive_dtor(lnode, NULL);
			return PARSER_RET_STATUS(ret);
		}

		struct tree_node *mnode = NULL;

		mnode = expr_create_operator_tnode(
			expression_operators[EXPR_IDX_COMMA], lnode, rnode);


		if (!mnode) {
			tnode_recursive_dtor(lnode, NULL);
			tnode_recursive_dtor(rnode, NULL);
			return PARSER_RET_STATUS(S_FAIL);
		}

		lnode = mnode;
	}

	*node = lnode;

	return PARSER_RET_STATUS(S_OK);

}

static int getArgumentsList(struct expression *expr, struct lexer *lexer,
			 size_t *lexer_idx, struct tree_node **node) {
	assert (expr);
	assert (lexer);
	assert (lexer_idx);
	assert (node);

	int ret = 0;

	struct lexer_token *tok = NULL;

	if ((tok = lexer_get_token(lexer, *lexer_idx)) &&
		tok->tok_type == LXTOK_BROUND_OPEN) {
		(*lexer_idx)++;

		if (!((tok = lexer_get_token(lexer, *lexer_idx)) &&
			tok->tok_type == LXTOK_BROUND_CLOSE)) {
			if ((ret = CALL_PARSER(getCommaSeparatedList, expr, lexer,
						lexer_idx, node))) {
				return PARSER_RET_STATUS(ret);
			}

			ret = S_OK;
		} else {
			*node = NULL;
			ret = S_OK;
		}

		if (!((tok = lexer_get_token(lexer, *lexer_idx)) &&
			tok->tok_type == LXTOK_BROUND_CLOSE)) {
			return PARSER_RET_STATUS(S_FAIL);
		}
		(*lexer_idx)++;


		return ret;
	}

	return S_CONTINUE;
}

static int getFunc(struct expression *expr, struct lexer *lexer,
				size_t *lexer_idx, struct tree_node **node) {
	assert (expr);
	assert (lexer);
	assert (lexer_idx);
	assert (node);

	int ret = 0;
	struct lexer_token *tok = NULL;
	if (!((tok = lexer_get_token(lexer, *lexer_idx)) &&
		tok->tok_type == LXTOK_FUNC)) {
		return PARSER_RET_STATUS(S_FAIL);
	}

	(*lexer_idx)++;

	struct tree_node *func_name = NULL;
	if (CALL_PARSER(getVariable, expr, lexer, lexer_idx, &func_name)) {
		return PARSER_RET_STATUS(S_FAIL);
	}

	int is_main = 0;
	if (!strcmp(func_name->value.varname, "main")) {
		is_main = 1;
	}

	struct tree_node *args_list = NULL;
	if (CALL_PARSER(getArgumentsList, expr, lexer, lexer_idx, &args_list)) {
		tnode_recursive_dtor(func_name, NULL);
		return PARSER_RET_STATUS(S_FAIL);
	}

	struct tree_node *func_body = NULL;
	if (CALL_PARSER(getCodeBlock, expr, lexer, lexer_idx, &func_body)) {
		tnode_recursive_dtor(func_name, NULL);
		tnode_recursive_dtor(args_list, NULL);
		return PARSER_RET_STATUS(S_FAIL);
	}


	struct tree_node *func_declaration = expr_create_operator_tnode(
		expression_operators[EXPR_IDX_COMMA], func_name, args_list);

	if (!func_declaration) {
		tnode_recursive_dtor(func_name, NULL);
		tnode_recursive_dtor(args_list, NULL);
		tnode_recursive_dtor(func_body, NULL);
		return PARSER_RET_STATUS(S_FAIL);
	}
	func_name = NULL;
	args_list = NULL;

	size_t func_idx = is_main ? EXPR_IDX_MAIN : EXPR_IDX_FUNC;

	struct tree_node *func = expr_create_operator_tnode(
		expression_operators[func_idx], func_declaration, func_body);

	if (!func) {
		tnode_recursive_dtor(func_declaration, NULL);
		tnode_recursive_dtor(func_body, NULL);
		return PARSER_RET_STATUS(S_FAIL);
	}
	func_declaration = NULL;
	func_body = NULL;

	*node = func;

	return PARSER_RET_STATUS(S_OK);
}

static int getFunctions(struct expression *expr, struct lexer *lexer,
				size_t *lexer_idx, struct tree_node **node) {
	assert (expr);
	assert (lexer);
	assert (lexer_idx);
	assert (node);

	int ret = 0;
	struct lexer_token *tok = NULL;
	struct tree_node *lnode = NULL;
	while (((tok = lexer_get_token(lexer, *lexer_idx)) &&
		tok->tok_type == LXTOK_FUNC)) {

		struct tree_node *rnode = NULL;
		if ((ret = CALL_PARSER(getFunc, expr, lexer, lexer_idx, &rnode))) {
			tnode_recursive_dtor(lnode, NULL);
			return PARSER_RET_STATUS(ret);
		}

		if (!rnode) {
			continue;
		}

		if (!lnode) {
			lnode = rnode;
			continue;
		}

		struct tree_node *mnode = NULL;

		mnode = expr_create_operator_tnode(
			expression_operators[EXPR_IDX_SEMICOLON], lnode, rnode);

		if (!mnode) {
			tnode_recursive_dtor(lnode, NULL);
			tnode_recursive_dtor(rnode, NULL);
			return PARSER_RET_STATUS(S_FAIL);
		}

		lnode = mnode;
	}

	*node = lnode;

	return PARSER_RET_STATUS(S_OK);
}

static int getTerminator(struct expression *expr, struct lexer *lexer,
			 size_t *lexer_idx, struct tree_node **node) {
	assert (expr);
	assert (lexer);
	assert (lexer_idx);
	assert (node);

	eprintf("term: %zu %zu\n", *lexer_idx, lexer->tokens.len);

	if (lexer->tokens.len == *lexer_idx) {
		return PARSER_RET_STATUS(S_OK);
	}
	
	return PARSER_RET_STATUS(S_FAIL);
}

static int getG(struct expression *expr, struct lexer *lexer,
		size_t *lexer_idx, struct tree_node **node) {
	assert (expr);
	assert (lexer);
	assert (lexer_idx);
	assert (node);

	int ret = 0;

	if ((ret = CALL_PARSER(getFunctions, expr, lexer, lexer_idx, node))) {
		return PARSER_RET_STATUS(ret);
	}

	if ((ret = CALL_PARSER(getTerminator, expr, lexer, lexer_idx, node))) {
		tnode_recursive_dtor(*node, NULL);
		*node = NULL;
		return PARSER_RET_STATUS(ret);
	}

	return PARSER_RET_STATUS(S_OK);
}


int expression_parse_lexer(struct expression *expr, struct lexer *lexer) {
	assert (lexer);
	assert (expr);

	*expr = (struct expression){0};
	if (expression_ctor(expr)) {
		return S_FAIL;
	};

	size_t lexer_idx_copy = 0;
	size_t lexer_idx = 0;
	
	if (CALL_PARSER(getG, expr, lexer, &lexer_idx_copy, &expr->tree.root)) {
		size_t fail_pos = (size_t)(lexer_idx_copy - lexer_idx); 

		eprintf("\nExpression parsing failed in lexer position %zu:\n", fail_pos);

		expression_dtor(expr);

		return S_FAIL;
	}

	return S_OK;
}

static int log_str_neighborhood(const char *real_str,
			 const char *e_ptr, size_t side_len,
			 FILE *out_stream);

int expression_parse_str(const char *str, struct expression *expr) {
	assert (str);
	assert (expr);

	struct lexer lexer = {0};
	if (LEXER_STATUS(lexer_ctor(&lexer))) {
		return S_FAIL;
	}

	LexerStatus lexerStatus = lexer_parse_text(&lexer, str);
	if (LEXER_STATUS(lexerStatus)) {
		eprintf("\nExpression parsing failed in position %zd:\n",
				lexerStatus.text_position);

		log_str_neighborhood(str, str + lexerStatus.text_position,
						10, stdout);
		return S_FAIL;
	}


	if (expression_parse_lexer(expr, &lexer)) {
		lexer_dtor(&lexer);
		return S_FAIL;
	}

	lexer_dtor(&lexer);

	return S_OK;
}

/*

 */
/*

int expression_parse_str(char *str, struct expression *expr) {
	assert (str);
	assert (expr);

	*expr = (struct expression){0};
	if (expression_ctor(expr)) {
		return S_FAIL;
	};

	const char *s_copy = str;

	if (pvector_init(&var_names, sizeof(struct expression_variable))) {
		expression_dtor(expr);
		return S_FAIL;
	}

	if (pvector_set_element_destructor(&var_names, var_destructor)) {
		pvector_destroy(&var_names);
		expression_dtor(expr);
		return S_FAIL;
	}

	if (CALL_PARSER(getG, &s_copy, &expr->tree.root)) {
		size_t fail_pos = (size_t)(s_copy - str); 

		eprintf("\nExpression parsing failed in position %zu:\n", fail_pos);

		log_str_neighborhood(str, s_copy, 10, stdout);

		expression_dtor(expr);
		pvector_destroy(&var_names);

		return S_FAIL;
	}

	expr->variables = var_names;
	var_names = (struct pvector){0};

	return S_OK;
}
*/

int expression_parse_file(const char *filename, struct expression *expr) {
	assert (filename);
	assert (expr);

	char *bufptr = NULL;
	size_t read_bytes = 0;

	int ret = 0;
	if ((ret = read_file(filename, &bufptr, &read_bytes))) {
		return ret;
	}

	ret = expression_parse_str(bufptr, expr);

	free(bufptr);

	return ret;
}

static int log_str_neighborhood(const char *real_str,
			 const char *e_ptr, size_t side_len,
			 FILE *out_stream) {
	assert (real_str);
	assert (e_ptr);
	assert (out_stream);

	size_t fail_pos = (size_t)(e_ptr - real_str);
	size_t str_len = strlen(real_str);

	if (fail_pos > str_len) {
		return S_FAIL;
	}

	size_t left_logging = 0;
	if (fail_pos > side_len) {
		left_logging = fail_pos - side_len;
	}

	size_t right_logging = fail_pos + side_len + 1;
	if (right_logging > str_len) {
		right_logging = str_len;
	}

	for (size_t i = left_logging; i < right_logging; i++) {
		fprintf(out_stream, "%c", real_str[i]);
	}
	fprintf(out_stream, "\n");
	for (size_t i = left_logging; i < fail_pos; i++) {
		fprintf(out_stream, " ");
	}
	fprintf(out_stream, "^\n");

	return S_OK;
}
