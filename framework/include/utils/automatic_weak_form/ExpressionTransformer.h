#pragma once

#include "MooseAST.h"
#include "VariableSplitting.h"
#include <map>
#include <string>

namespace moose
{
namespace automatic_weak_form
{

/**
 * Transforms expressions to use split variables
 * Replaces high-order derivatives with split variable references
 * 
 * Example: grad(grad(u)) -> grad(q) where q = grad(u)
 */
class ExpressionTransformer
{
public:
  ExpressionTransformer(const std::map<std::string, SplitVariable> & split_vars, unsigned int dim = 3)
    : _split_variables(split_vars), _dim(dim) {}
  
  /**
   * Transform an expression to use split variables
   * @param expr Original expression with high-order derivatives
   * @return Transformed expression using split variables
   */
  NodePtr transform(const NodePtr & expr);
  
private:
  const std::map<std::string, SplitVariable> & _split_variables;
  unsigned int _dim;
  
  /**
   * Recursively transform a node
   */
  NodePtr transformNode(const NodePtr & node);
  
  /**
   * Check if a gradient should be replaced with a split variable
   */
  NodePtr transformGradient(const UnaryOpNode * grad_node);
  
  /**
   * Check if a divergence should be replaced
   */
  NodePtr transformDivergence(const UnaryOpNode * div_node);
  
  /**
   * Find split variable for a given derivative expression
   */
  NodePtr findSplitVariable(const std::string & original_var, unsigned int derivative_order);
  
  /**
   * Get the base variable name from a potentially nested derivative
   */
  std::string getBaseVariable(const NodePtr & expr);
  
  /**
   * Count the derivative order of an expression
   */
  unsigned int getDerivativeOrder(const NodePtr & expr);
};

} // namespace automatic_weak_form
} // namespace moose