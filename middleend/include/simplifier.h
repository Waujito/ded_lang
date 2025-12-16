#ifndef SIMPLIFIER_H
#define SIMPLIFIER_H

#include "expression.h"

#ifdef __cplusplus
extern "C" {
#endif

struct tree_node *tnode_simplify(struct expression *expr, struct tree_node *node);
int expression_simplify(struct expression *expr, struct expression *simplified);

#ifdef __cplusplus
}
#endif

#endif /* SIMPLIFIER_H */
