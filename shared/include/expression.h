#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "pvector.h"
#include "tree.h"

#ifdef __cplusplus
extern "C" {
#endif

struct expression_variable {
	char *var_name;
	size_t var_pointer;
};

struct expression {
	struct tree tree;
	// vector of expression_variable
	struct pvector variables;
};

int expression_ctor(struct expression *expr);
int expression_dtor(struct expression *expr);

struct expression_variable *expr_find_variable(struct expression *expr,
						      const char *varname);
int expr_push_variable(struct expression *expr, const char *varname,
			       struct expression_variable **nvar);

int expression_load(struct expression *expr, const char *filename);
int expression_store(struct expression *expr, const char *filename);


// Update the operators array with derivative functions
// extern const struct expression_operator expression_operators[];
enum {
	EXPRESSION_F_NUMBER	= 0x1,
	EXPRESSION_F_VARIABLE	= 0x2,
	EXPRESSION_F_OPERATOR	= 0x7,
};

enum expression_op_indexes {
	EXPR_IDX_PLUS,	
	EXPR_IDX_MINUS,	
	EXPR_IDX_MULTIPLY,	
	EXPR_IDX_DIVIDE,	
	EXPR_IDX_POW,	
	EXPR_IDX_ASSIGN,	
	EXPR_IDX_EQUALS_CMP, 
	EXPR_IDX_SEMICOLON,	
	EXPR_IDX_DECL_ASSIGN,
	EXPR_IDX_COMMA,
	EXPR_IDX_GREATER_CMP,
	EXPR_IDX_LESS_CMP,
	EXPR_IDX_NOT_EQUALS_CMP,	
	EXPR_IDX_GREATER_EQ_CMP,	
	EXPR_IDX_LESS_EQ_CMP,
	EXPR_IDX_PRINT,
	EXPR_IDX_INPUT,
};

enum expression_op_type {
	EXPR_OP_T_UNARY,
	EXPR_OP_T_NOARG,
	EXPR_OP_T_BINARY,
	EXPR_OP_T_KEYWORD,
};

struct expression_operator {
	enum expression_op_indexes idx;
	const char *name;
	int priority;
	enum expression_op_type type;
};

#define DECLARE_EXPERSSION_OP(_idx, opname, opstring_name, oppriority, optype)	\
	static const struct expression_operator expr_operator_##opname = {	\
		.idx = _idx,							\
		.name = opstring_name,						\
		.priority = oppriority,						\
		.type = optype,							\
	}

DECLARE_EXPERSSION_OP(EXPR_IDX_PRINT,		print,		"print",  7, EXPR_OP_T_UNARY);
DECLARE_EXPERSSION_OP(EXPR_IDX_INPUT,		input,		"input",  7, EXPR_OP_T_NOARG);
DECLARE_EXPERSSION_OP(EXPR_IDX_SEMICOLON,	semicolon,	";",  7, EXPR_OP_T_KEYWORD);
DECLARE_EXPERSSION_OP(EXPR_IDX_ASSIGN,		assign,		"=",  6, EXPR_OP_T_KEYWORD);
DECLARE_EXPERSSION_OP(EXPR_IDX_DECL_ASSIGN,	decl_assign,	":=", 6, EXPR_OP_T_KEYWORD);
DECLARE_EXPERSSION_OP(EXPR_IDX_COMMA,		comma,		",",  5, EXPR_OP_T_KEYWORD);
DECLARE_EXPERSSION_OP(EXPR_IDX_EQUALS_CMP,	equals_cmp,	"==", 4, EXPR_OP_T_BINARY);
DECLARE_EXPERSSION_OP(EXPR_IDX_GREATER_CMP,	greater_cmp,	">",  4, EXPR_OP_T_BINARY);
DECLARE_EXPERSSION_OP(EXPR_IDX_LESS_CMP,	less_cmp,	"<",  4, EXPR_OP_T_BINARY);
DECLARE_EXPERSSION_OP(EXPR_IDX_NOT_EQUALS_CMP,	nequals_cmp,	"!=", 4, EXPR_OP_T_BINARY);
DECLARE_EXPERSSION_OP(EXPR_IDX_GREATER_EQ_CMP,	greater_eq_cmp,	">=", 4, EXPR_OP_T_BINARY);
DECLARE_EXPERSSION_OP(EXPR_IDX_LESS_EQ_CMP,	less_eq_cmp,	"<=", 4, EXPR_OP_T_BINARY);
DECLARE_EXPERSSION_OP(EXPR_IDX_PLUS,		addition,	"+",  3, EXPR_OP_T_BINARY);
DECLARE_EXPERSSION_OP(EXPR_IDX_MINUS,		subtraction,	"-",  3, EXPR_OP_T_BINARY);
DECLARE_EXPERSSION_OP(EXPR_IDX_MULTIPLY,	multiplication, "*",  2, EXPR_OP_T_BINARY);
DECLARE_EXPERSSION_OP(EXPR_IDX_DIVIDE,		division,	"/",  2, EXPR_OP_T_BINARY);
DECLARE_EXPERSSION_OP(EXPR_IDX_POW,		power,		"^",  0, EXPR_OP_T_BINARY);


#undef DECLARE_EXPERSSION_OP

#define REGISTER_EXPRESSION_OP(idx, opname)					\
	[idx] = &expr_operator_##opname

static const struct expression_operator *const expression_operators[] = {
	REGISTER_EXPRESSION_OP(EXPR_IDX_PLUS,		addition),
	REGISTER_EXPRESSION_OP(EXPR_IDX_MINUS,		subtraction),
	REGISTER_EXPRESSION_OP(EXPR_IDX_MULTIPLY,	multiplication),
	REGISTER_EXPRESSION_OP(EXPR_IDX_DIVIDE,		division),
	REGISTER_EXPRESSION_OP(EXPR_IDX_POW,		power),
	REGISTER_EXPRESSION_OP(EXPR_IDX_ASSIGN,		assign),
	REGISTER_EXPRESSION_OP(EXPR_IDX_EQUALS_CMP,	equals_cmp),
	REGISTER_EXPRESSION_OP(EXPR_IDX_SEMICOLON,	semicolon),
	REGISTER_EXPRESSION_OP(EXPR_IDX_DECL_ASSIGN,	decl_assign),
	REGISTER_EXPRESSION_OP(EXPR_IDX_COMMA,		comma),
	REGISTER_EXPRESSION_OP(EXPR_IDX_GREATER_CMP,	greater_cmp),
	REGISTER_EXPRESSION_OP(EXPR_IDX_LESS_CMP,	less_cmp),
	REGISTER_EXPRESSION_OP(EXPR_IDX_NOT_EQUALS_CMP,	nequals_cmp),
	REGISTER_EXPRESSION_OP(EXPR_IDX_GREATER_EQ_CMP,	greater_eq_cmp),
	REGISTER_EXPRESSION_OP(EXPR_IDX_LESS_EQ_CMP,	less_eq_cmp),
	REGISTER_EXPRESSION_OP(EXPR_IDX_PRINT,		print),
	REGISTER_EXPRESSION_OP(EXPR_IDX_INPUT,		input),
	NULL,
};

#undef REGISTER_EXPRESSION_OP

/**
 * implements value_deserializer
 */
DSError_t expression_deserializer(tree_dtype *value, char *str, void *ctx);

/**
 * implements value_serializer
 */
DSError_t expression_serializer(tree_dtype value, FILE *out_stream, void *ctx);

struct tree_node *expr_create_number_tnode(int64_t snum);
struct tree_node *expr_create_variable_tnode(const char *varname);
struct tree_node *expr_create_operator_tnode(const struct expression_operator *op, 
                                              struct tree_node *left, 
                                              struct tree_node *right);
struct tree_node *expr_copy_tnode(struct expression *expr, struct tree_node *original);

#define EXPR_TNODE_IS_NUMBER(node) ((node->value.flags & EXPRESSION_F_OPERATOR) \
						== EXPRESSION_F_NUMBER)
#define EXPR_TNODE_IS_VARIABLE(node) ((node->value.flags & EXPRESSION_F_OPERATOR) \
						== EXPRESSION_F_VARIABLE)
#define EXPR_TNODE_IS_OPERATOR(node) ((node->value.flags & EXPRESSION_F_OPERATOR) \
						== EXPRESSION_F_OPERATOR)

#ifdef __cplusplus
}
#endif

#endif /* EXPRESSION_H */
