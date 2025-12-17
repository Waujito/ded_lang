#include <string.h>

#include "pvector.h"
#include "expression.h"
#include "backend.h"

struct variable {
	const char *var_name;
	size_t var_pointer;
};

#define TRANSLATOR_STATUS_GEN(status_) \
	((struct TranslatorStatus) {.status = status_})

struct translation_context {
	struct expression *expr;
	FILE *asm_output;

	// of struct variable
	struct pvector variables;

	// Internal jump tracking index
	// Helps to escape name overlaps
	size_t jmp_idx;
};



static struct variable *find_variable(struct translation_context *ctx, const char *varname) {
	assert (ctx);
	assert (varname);

	for (size_t i = 0; i < ctx->variables.len; i++) {
      		struct variable *var = NULL;
		if (pvector_get(&ctx->variables, i, (void **)&var)) {
			log_error("pvector_get error (normally unreachable)");
			return NULL;
		}

		if (!strcmp(varname, var->var_name)) {
			return var;
		}
	}

	return NULL;
}

static TranslatorStatus push_variable(struct translation_context *ctx, const char *varname,
			       struct variable **nvar) {
	assert (ctx);
	assert (varname);

	if (find_variable(ctx, varname)) {
		log_error("Already declared variable: %s", varname);
		return TRANSLATOR_STATUS_GEN(BTRST_ALREADY_DECLARED_VAR);
	}

	size_t var_idx = ctx->variables.len;

	struct variable var = {
		.var_name = varname,
		.var_pointer = var_idx,
	};

	if (pvector_push_back(&ctx->variables, &var)) {
		log_error("pvector_push_back: Allocation error");
		return TRANSLATOR_STATUS_GEN(BTRST_ALLOCATION);
	}

	struct variable *rvar = NULL;
	if (pvector_get(&ctx->variables, var_idx, (void **)&rvar)
		|| rvar->var_pointer != var.var_pointer) {
		log_error("pvector_get error (normally unreachable)");
		return TRANSLATOR_STATUS_GEN(BTRST_INTERNAL_FAILURE);
	}

	if (nvar) {
		*nvar = rvar;
	}

	return TRANSLATOR_STATUS_GEN(BTRST_OK);
}

static TranslatorStatus tpush_expression(struct tree_node *tnode,
					   struct translation_context *ctx);

static TranslatorStatus tpush_number(struct tree_node *tnode,
					struct translation_context *ctx) {

	assert (tnode);
	assert (ctx);
	assert (EXPR_TNODE_IS_NUMBER(tnode));

	fprintf(ctx->asm_output,
		"ldc r0 $%ld\n"
		"push r0\n",
		tnode->value.snum);
	return TRANSLATOR_STATUS_GEN(BTRST_OK);
}

static TranslatorStatus tpush_variable(struct tree_node *tnode,
					struct translation_context *ctx) {

	assert (tnode);
	assert (ctx);
	assert (EXPR_TNODE_IS_VARIABLE(tnode));

	struct variable *var = find_variable(ctx, tnode->value.varname);
	if (!var) {
		log_error("Undeclared variable: %s", tnode->value.varname);
		return TRANSLATOR_STATUS_GEN(BTRST_UNDECLARED_VARIABLE);
	}
	
	fprintf(ctx->asm_output,
		"ldc r0 $%zu\n"
		"ldm r0 r0\n"
		"push r0\n",
		var->var_pointer);

	return TRANSLATOR_STATUS_GEN(BTRST_OK);
}

// tpush_cmp.
// left and right nodes are loaded in r0 and r1.
// Pushes the 1 or 0 on stack.
static TranslatorStatus tpush_cmp_r01(struct tree_node *tnode,
				      struct translation_context *ctx) {
	assert (tnode);
	assert (ctx);
	assert (EXPR_TNODE_IS_OPERATOR(tnode));

	struct expression_operator *op = tnode->value.ptr;

	fprintf(ctx->asm_output, "cmp r0 r1\n");
	size_t jmp_idx = ctx->jmp_idx++;

	fprintf(ctx->asm_output, "ldc r0 $0\n");
	switch ((int)op->idx) {
		case EXPR_IDX_EQUALS_CMP:
			fprintf(ctx->asm_output, "jmp.neq ._jmp_tps__%zu\n", jmp_idx);
			break;
		case EXPR_IDX_LESS_CMP:
			fprintf(ctx->asm_output, "jmp.geq ._jmp_tps__%zu\n", jmp_idx);
			break;
		case EXPR_IDX_GREATER_CMP:
			fprintf(ctx->asm_output, "jmp.leq ._jmp_tps__%zu\n", jmp_idx);
			break;
		case EXPR_IDX_NOT_EQUALS_CMP:
			fprintf(ctx->asm_output, "jmp.eq ._jmp_tps__%zu\n", jmp_idx);
			break;
		case EXPR_IDX_LESS_EQ_CMP:
			fprintf(ctx->asm_output, "jmp.gt ._jmp_tps__%zu\n", jmp_idx);
			break;
		case EXPR_IDX_GREATER_EQ_CMP:
			fprintf(ctx->asm_output, "jmp.lt ._jmp_tps__%zu\n", jmp_idx);
			break;
		default:
			assert(0 && "unreachable");
			break;
	}

	fprintf(ctx->asm_output, "ldc r0 $1\n");
	fprintf(ctx->asm_output, "._jmp_tps__%zu:\n", jmp_idx);
	fprintf(ctx->asm_output, "push r0\n");

	return TRANSLATOR_STATUS_GEN(BTRST_OK);
}

static TranslatorStatus tpush_operator(struct tree_node *tnode,
					struct translation_context *ctx) {

	assert (tnode);
	assert (ctx);
	assert (EXPR_TNODE_IS_OPERATOR(tnode));

	struct expression_operator *op = tnode->value.ptr;

	if (op->type != EXPR_OP_T_UNARY
		&& op->type != EXPR_OP_T_BINARY) {
	}

	TranslatorStatus ret = TRANSLATOR_STATUS_GEN(BTRST_OK);

	if (op->type == EXPR_OP_T_UNARY) {
		ret = tpush_expression(tnode->left, ctx);

		if (TRANSLATOR_STATUS(ret)) {
			return ret;
		}

		fprintf(ctx->asm_output, "pop r0\n");
	} else if (op->type == EXPR_OP_T_BINARY) {
		ret = tpush_expression(tnode->left, ctx);

		if (TRANSLATOR_STATUS(ret)) {
			return ret;
		}

		ret = tpush_expression(tnode->right, ctx);
		if (TRANSLATOR_STATUS(ret)) {
			return ret;
		}
		// Inversion because of stack
		fprintf(ctx->asm_output, "pop r1\n"
	  				 "pop r0\n");
	} else if (op->type == EXPR_OP_T_NOARG) {
		// Nothing here :) !!
	} else {
		log_error("Attempt to evaluate non-evaluateable");
		return TRANSLATOR_STATUS_GEN(BTRST_TREE_INVALID);
	}

	switch ((int)op->idx) {
		case EXPR_IDX_PLUS:
			fprintf(ctx->asm_output, "add r0 r0 r1\n" "push r0\n");
			break;
		case EXPR_IDX_MINUS:
			fprintf(ctx->asm_output, "sub r0 r0 r1\n" "push r0\n");
			break;
		case EXPR_IDX_MULTIPLY:
			fprintf(ctx->asm_output, "mul r0 r0 r1\n" "push r0\n");
			break;
		case EXPR_IDX_DIVIDE:
			fprintf(ctx->asm_output, "div r0 r0 r1\n" "push r0\n");
			break;
		case EXPR_IDX_LESS_CMP:
		case EXPR_IDX_GREATER_CMP:
		case EXPR_IDX_EQUALS_CMP:
		case EXPR_IDX_NOT_EQUALS_CMP:
		case EXPR_IDX_LESS_EQ_CMP:
		case EXPR_IDX_GREATER_EQ_CMP:
			return tpush_cmp_r01(tnode, ctx);
		case EXPR_IDX_PRINT:
			fprintf(ctx->asm_output, "print r0\n" "push r0\n");
			break;
		case EXPR_IDX_INPUT:
			fprintf(ctx->asm_output, "input r0\n" "push r0\n");
			break;
		default:
			log_error("Not implemented :(");
			return TRANSLATOR_STATUS_GEN(BTRST_INTERNAL_FAILURE);
	}

	return TRANSLATOR_STATUS_GEN(BTRST_OK);
}

static TranslatorStatus tpush_expression(struct tree_node *tnode,
					   struct translation_context *ctx) {
	assert (ctx);

	if (!tnode) {
		log_error("Tree is corrupted!");
		return TRANSLATOR_STATUS_GEN(BTRST_TREE_INVALID);
	}

	if (EXPR_TNODE_IS_NUMBER(tnode)) {
		return tpush_number(tnode, ctx);
	}

	if (EXPR_TNODE_IS_VARIABLE(tnode)) {
		return tpush_variable(tnode, ctx);
	}

	if (EXPR_TNODE_IS_OPERATOR(tnode)) {
		return tpush_operator(tnode, ctx);
	}	

	log_error("Not implemented :(");
	return TRANSLATOR_STATUS_GEN(BTRST_INTERNAL_FAILURE);
}

static TranslatorStatus translate_assignment(struct tree_node *tnode,
				     struct translation_context *ctx) {
	assert (ctx);
	assert (tnode);
	assert (EXPR_TNODE_IS_OPERATOR(tnode));

	struct expression_operator *op = tnode->value.ptr;
	assert (op->idx == EXPR_IDX_ASSIGN || op->idx == EXPR_IDX_DECL_ASSIGN);

	if (!tnode->left || !EXPR_TNODE_IS_VARIABLE(tnode->left)) {
		log_error("Tree is corrupted!");
		return TRANSLATOR_STATUS_GEN(BTRST_TREE_INVALID);
	}

	struct variable *var = find_variable(ctx, tnode->left->value.varname);
	if (!var) {
		log_error("Undeclared variable: %s", tnode->left->value.varname);
		return TRANSLATOR_STATUS_GEN(BTRST_UNDECLARED_VARIABLE);
	};

	TranslatorStatus ret = tpush_expression(tnode->right, ctx);
	if (TRANSLATOR_STATUS(ret)) {
		return ret;
	}

	fprintf(ctx->asm_output,"pop r0\n"
	 			"ldc r1 $%zu\n"
	 			"stm r1 r0\n",
	 			var->var_pointer);

	return TRANSLATOR_STATUS_GEN(BTRST_OK);
}

static TranslatorStatus translate_declaration(struct tree_node *tnode,
				     struct translation_context *ctx) {
	assert (ctx);
	assert (tnode);
	assert (EXPR_TNODE_IS_OPERATOR(tnode));

	struct expression_operator *op = tnode->value.ptr;
	assert (op->idx == EXPR_IDX_DECL_ASSIGN);

	if (!tnode->left || !EXPR_TNODE_IS_VARIABLE(tnode->left)) {
		return TRANSLATOR_STATUS_GEN(BTRST_TREE_INVALID);
	}

	TranslatorStatus ret = push_variable(ctx, tnode->left->value.varname, NULL); 
	if (TRANSLATOR_STATUS(ret)) {
		return ret;
	};

	return translate_assignment(tnode, ctx);
}


static TranslatorStatus translate_statement(struct tree_node *tnode,
				     struct translation_context *ctx) {
	assert (ctx);

	if (!tnode) {
		return TRANSLATOR_STATUS_GEN(BTRST_OK);
	}

	if (EXPR_TNODE_IS_NUMBER(tnode)) {
		return TRANSLATOR_STATUS_GEN(BTRST_OK);
	}

	if (EXPR_TNODE_IS_VARIABLE(tnode)) {
		if (!find_variable(ctx, tnode->value.varname)) {
			log_error("Undeclared variable: %s", tnode->value.varname);
			return TRANSLATOR_STATUS_GEN(BTRST_UNDECLARED_VARIABLE);
		} else {
			return TRANSLATOR_STATUS_GEN(BTRST_OK);
		}
	}

	if (!EXPR_TNODE_IS_OPERATOR(tnode)) {
		log_error("Not implemented :(");
		return TRANSLATOR_STATUS_GEN(BTRST_INTERNAL_FAILURE);
	}

	TranslatorStatus ret = TRANSLATOR_STATUS_GEN(BTRST_OK);

	struct expression_operator *op = tnode->value.ptr;

	if (op->idx == EXPR_IDX_SEMICOLON) {
		ret = translate_statement(tnode->left, ctx);
		if (TRANSLATOR_STATUS(ret)) {
			return ret;
		}

		ret = translate_statement(tnode->right, ctx);
		return ret;
	}

	if (op->idx == EXPR_IDX_DECL_ASSIGN) {
		return translate_declaration(tnode, ctx);
	}

	if (op->idx == EXPR_IDX_ASSIGN) {
		return translate_assignment(tnode, ctx);
	}
	
	// Dummy expression
	ret = tpush_expression(tnode, ctx);
	fprintf(ctx->asm_output, "pop r0\n");

	return ret;
}

static TranslatorStatus translator_tnode(struct tree_node *tnode,
		     struct translation_context *ctx) {
	assert (tnode);
	assert (ctx);

	return translate_statement(tnode, ctx);
}

TranslatorStatus backend_translator(struct expression *expr, FILE *asm_output) {
	assert (expr);
	assert (asm_output);

	struct translation_context ctx = {
		.asm_output = asm_output,
		.variables = {0},
		.expr = expr,
		.jmp_idx = 0,
	};

	pvector_init(&ctx.variables, sizeof(struct variable));

	TranslatorStatus ret = translator_tnode(expr->tree.root, &ctx);

	pvector_destroy(&ctx.variables);
	fprintf(asm_output, "dump\n");

	return ret;
}
