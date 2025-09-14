#include "WeakFormGenerator.h"
#include "ExpressionSimplifier.h"
#include "MooseError.h"
#include <algorithm>
#include <cmath>

namespace moose
{
namespace automatic_weak_form
{

Differential DifferentiationVisitor::differentiate(const NodePtr & expr)
{
  auto diff = visit(expr);
  
  // Simplify all coefficients
  for (auto & [order, coeff] : diff.coefficients)
  {
    if (coeff)
      coeff = ExpressionSimplifier::simplify(coeff);
  }
  
  return diff;
}

Differential DifferentiationVisitor::visit(const NodePtr & node)
{
  if (!node)
    return Differential{};
  
  switch (node->type())
  {
    case NodeType::Constant:
      return visitConstant(static_cast<const ConstantNode *>(node.get()));
    case NodeType::Variable:
      return visitVariable(static_cast<const VariableNode *>(node.get()));
    case NodeType::FieldVariable:
      return visitFieldVariable(static_cast<const FieldVariableNode *>(node.get()));
    case NodeType::Add:
    case NodeType::Subtract:
    case NodeType::Multiply:
    case NodeType::Divide:
    case NodeType::Power:
    case NodeType::Dot:
    case NodeType::Contract:
    case NodeType::Outer:
      return visitBinaryOp(static_cast<const BinaryOpNode *>(node.get()));
    case NodeType::Negate:
    case NodeType::Gradient:
    case NodeType::Divergence:
    case NodeType::Laplacian:
    case NodeType::Norm:
    case NodeType::Normalize:
    case NodeType::Trace:
    case NodeType::Determinant:
    case NodeType::Inverse:
    case NodeType::Transpose:
    case NodeType::Symmetric:
    case NodeType::Skew:
    case NodeType::Deviatoric:
      return visitUnaryOp(static_cast<const UnaryOpNode *>(node.get()));
    case NodeType::Function:
      return visitFunction(static_cast<const FunctionNode *>(node.get()));
    default:
      mooseError("Unsupported node type in differentiation");
  }
}

Differential DifferentiationVisitor::visitConstant(const ConstantNode * node)
{
  Differential result;
  return result;
}

Differential DifferentiationVisitor::visitVariable(const VariableNode * node)
{
  Differential result;
  if (node->name() == _var_name)
  {
    result.coefficients[0] = constant(1.0);
  }
  return result;
}

Differential DifferentiationVisitor::visitFieldVariable(const FieldVariableNode * node)
{
  Differential result;
  if (node->name() == _var_name)
  {
    result.coefficients[0] = constant(1.0);
  }
  return result;
}

Differential DifferentiationVisitor::visitUnaryOp(const UnaryOpNode * node)
{
  Differential operand_diff = visit(node->operand());
  
  switch (node->type())
  {
    case NodeType::Negate:
    {
      Differential result;
      for (auto & [order, coeff] : operand_diff.coefficients)
        result.coefficients[order] = negate(coeff);
      return result;
    }
    
    case NodeType::Gradient:
      return handleGradient(node->operand());
    
    case NodeType::Divergence:
      return handleDivergence(node->operand());
    
    case NodeType::Laplacian:
      return handleLaplacian(node->operand());
    
    case NodeType::Norm:
      return handleNorm(node->operand());
    
    case NodeType::Normalize:
      return handleNormalize(node->operand());
    
    case NodeType::Trace:
    {
      Differential result;
      for (auto & [order, coeff] : operand_diff.coefficients)
        if (coeff)
          result.coefficients[order] = trace(coeff);
      return result;
    }
    
    case NodeType::Determinant:
    {
      Differential result;
      if (operand_diff.hasOrder(0))
      {
        auto A = node->operand();
        auto detA = det(A);
        auto invA = inv(A);
        auto ddetA = multiply(detA, trace(multiply(invA, operand_diff.coefficients[0])));
        result.coefficients[0] = ddetA;
      }
      return result;
    }
    
    case NodeType::Inverse:
    {
      Differential result;
      if (operand_diff.hasOrder(0))
      {
        // For inverse: d(A^-1) = -A^-1 * dA * A^-1
        // The node itself represents inv(A), so we reuse it
        auto unary = static_cast<const UnaryOpNode *>(node);
        auto invA = inv(unary->operand());
        auto dA = operand_diff.coefficients[0];
        auto dinvA = negate(multiply(invA, multiply(dA, invA)));
        result.coefficients[0] = dinvA;
      }
      return result;
    }
    
    case NodeType::Transpose:
    {
      Differential result;
      for (auto & [order, coeff] : operand_diff.coefficients)
        if (coeff)
          result.coefficients[order] = transpose(coeff);
      return result;
    }
    
    case NodeType::Symmetric:
    {
      Differential result;
      for (auto & [order, coeff] : operand_diff.coefficients)
        if (coeff)
          result.coefficients[order] = sym(coeff);
      return result;
    }
    
    case NodeType::Skew:
    {
      Differential result;
      for (auto & [order, coeff] : operand_diff.coefficients)
        if (coeff)
          result.coefficients[order] = skew(coeff);
      return result;
    }
    
    case NodeType::Deviatoric:
    {
      Differential result;
      for (auto & [order, coeff] : operand_diff.coefficients)
        if (coeff)
        {
          auto tr_coeff = multiply(constant(1.0/3.0), trace(coeff));
          auto I = constant(RankTwoTensor::Identity(), 3);
          auto spherical = multiply(tr_coeff, I);
          result.coefficients[order] = subtract(coeff, spherical);
        }
      return result;
    }
    
    default:
      mooseError("Unsupported unary operation in differentiation");
  }
}

Differential DifferentiationVisitor::visitBinaryOp(const BinaryOpNode * node)
{
  switch (node->type())
  {
    case NodeType::Add:
      return handleAdd(node->left(), node->right());
    case NodeType::Subtract:
      return handleSubtract(node->left(), node->right());
    case NodeType::Multiply:
      return handleMultiply(node->left(), node->right());
    case NodeType::Divide:
      return handleDivide(node->left(), node->right());
    case NodeType::Power:
      return handlePower(node->left(), node->right());
    case NodeType::Dot:
      return handleDot(node->left(), node->right());
    case NodeType::Contract:
      return handleContract(node->left(), node->right());
    case NodeType::Outer:
      return handleOuter(node->left(), node->right());
    default:
      mooseError("Unsupported binary operation in differentiation");
  }
}

Differential DifferentiationVisitor::visitFunction(const FunctionNode * node)
{
  Differential result;
  
  if (node->name() == "W" && node->args().size() == 1)
  {
    auto arg_diff = visit(node->args()[0]);
    if (arg_diff.hasOrder(0))
    {
      auto dW_dc = function("dW_dc", {node->args()[0]});
      for (auto & [order, coeff] : arg_diff.coefficients)
      {
        if (coeff)
          result.coefficients[order] = multiply(dW_dc, coeff);
      }
    }
  }
  else if (node->name() == "log" && node->args().size() == 1)
  {
    auto arg_diff = visit(node->args()[0]);
    if (arg_diff.hasOrder(0))
    {
      auto one_over_arg = divide(constant(1.0), node->args()[0]);
      for (auto & [order, coeff] : arg_diff.coefficients)
      {
        if (coeff)
          result.coefficients[order] = multiply(one_over_arg, coeff);
      }
    }
  }
  else if (node->name() == "exp" && node->args().size() == 1)
  {
    auto arg_diff = visit(node->args()[0]);
    if (arg_diff.hasOrder(0))
    {
      auto exp_arg = function("exp", {node->args()[0]});
      for (auto & [order, coeff] : arg_diff.coefficients)
      {
        if (coeff)
          result.coefficients[order] = multiply(exp_arg, coeff);
      }
    }
  }
  else if (node->name() == "sin" && node->args().size() == 1)
  {
    auto arg_diff = visit(node->args()[0]);
    if (arg_diff.hasOrder(0))
    {
      auto cos_arg = function("cos", {node->args()[0]});
      for (auto & [order, coeff] : arg_diff.coefficients)
      {
        if (coeff)
          result.coefficients[order] = multiply(cos_arg, coeff);
      }
    }
  }
  else if (node->name() == "cos" && node->args().size() == 1)
  {
    auto arg_diff = visit(node->args()[0]);
    if (arg_diff.hasOrder(0))
    {
      auto neg_sin = negate(function("sin", {node->args()[0]}));
      for (auto & [order, coeff] : arg_diff.coefficients)
      {
        if (coeff)
          result.coefficients[order] = multiply(neg_sin, coeff);
      }
    }
  }
  
  return result;
}

Differential DifferentiationVisitor::handleGradient(const NodePtr & operand)
{
  Differential operand_diff = visit(operand);
  Differential result;
  
  for (auto & [order, coeff] : operand_diff.coefficients)
  {
    if (coeff)
      result.coefficients[order + 1] = coeff;
  }
  
  return result;
}

Differential DifferentiationVisitor::handleDivergence(const NodePtr & operand)
{
  Differential operand_diff = visit(operand);
  Differential result;
  
  for (auto & [order, coeff] : operand_diff.coefficients)
  {
    if (coeff && order > 0)
      result.coefficients[order - 1] = coeff;
  }
  
  return result;
}

Differential DifferentiationVisitor::handleLaplacian(const NodePtr & operand)
{
  Differential operand_diff = visit(operand);
  Differential result;
  
  for (auto & [order, coeff] : operand_diff.coefficients)
  {
    if (coeff)
      result.coefficients[order + 2] = coeff;
  }
  
  return result;
}

Differential DifferentiationVisitor::handleNorm(const NodePtr & operand)
{
  Differential operand_diff = visit(operand);
  Differential result;
  
  if (operand_diff.hasOrder(0))
  {
    auto n = norm(operand);
    auto normalized = divide(operand, n);
    result.coefficients[0] = dot(normalized, operand_diff.coefficients[0]);
  }
  
  for (unsigned int order = 1; order <= operand_diff.maxOrder(); ++order)
  {
    if (operand_diff.hasOrder(order))
    {
      auto normalized = divide(operand, norm(operand));
      result.coefficients[order] = dot(normalized, operand_diff.coefficients[order]);
    }
  }
  
  return result;
}

Differential DifferentiationVisitor::handleNormalize(const NodePtr & operand)
{
  Differential operand_diff = visit(operand);
  Differential result;
  
  if (operand_diff.hasOrder(0))
  {
    auto n = norm(operand);
    auto normalized = divide(operand, n);
    auto I = constant(RankTwoTensor::Identity(), 3);
    auto nn = outer(normalized, normalized);
    auto P = subtract(I, nn);
    auto Pdu = multiply(P, operand_diff.coefficients[0]);
    result.coefficients[0] = divide(Pdu, n);
  }
  
  for (unsigned int order = 1; order <= operand_diff.maxOrder(); ++order)
  {
    if (operand_diff.hasOrder(order))
    {
      auto n = norm(operand);
      auto normalized = divide(operand, n);
      auto I = constant(RankTwoTensor::Identity(), 3);
      auto nn = outer(normalized, normalized);
      auto P = subtract(I, nn);
      auto Pdu = multiply(P, operand_diff.coefficients[order]);
      result.coefficients[order] = divide(Pdu, n);
    }
  }
  
  return result;
}

Differential DifferentiationVisitor::handleAdd(const NodePtr & left, const NodePtr & right)
{
  Differential left_diff = visit(left);
  Differential right_diff = visit(right);
  
  return combineDifferentials(left_diff, right_diff,
    [](const NodePtr & a, const NodePtr & b) { return add(a, b); });
}

Differential DifferentiationVisitor::handleSubtract(const NodePtr & left, const NodePtr & right)
{
  Differential left_diff = visit(left);
  Differential right_diff = visit(right);
  
  return combineDifferentials(left_diff, right_diff,
    [](const NodePtr & a, const NodePtr & b) { return subtract(a, b); });
}

Differential DifferentiationVisitor::handleMultiply(const NodePtr & left, const NodePtr & right)
{
  Differential left_diff = visit(left);
  Differential right_diff = visit(right);
  
  bool left_depends = dependsOnVariable(left);
  bool right_depends = dependsOnVariable(right);
  
  Differential result;
  
  if (!left_depends && right_depends)
  {
    for (auto & [order, coeff] : right_diff.coefficients)
      if (coeff)
        result.coefficients[order] = multiply(left, coeff);
  }
  else if (left_depends && !right_depends)
  {
    for (auto & [order, coeff] : left_diff.coefficients)
      if (coeff)
        result.coefficients[order] = multiply(coeff, right);
  }
  else if (left_depends && right_depends)
  {
    for (auto & [order_l, coeff_l] : left_diff.coefficients)
    {
      if (coeff_l)
        result.coefficients[order_l] = multiply(coeff_l, right);
    }
    
    for (auto & [order_r, coeff_r] : right_diff.coefficients)
    {
      if (coeff_r)
      {
        auto term = multiply(left, coeff_r);
        if (result.coefficients.count(order_r))
          result.coefficients[order_r] = add(result.coefficients[order_r], term);
        else
          result.coefficients[order_r] = term;
      }
    }
    
    for (auto & [order_l, coeff_l] : left_diff.coefficients)
    {
      for (auto & [order_r, coeff_r] : right_diff.coefficients)
      {
        if (coeff_l && coeff_r && (order_l > 0 || order_r > 0))
        {
          unsigned int combined_order = order_l + order_r;
          auto term = multiply(coeff_l, coeff_r);
          if (result.coefficients.count(combined_order))
            result.coefficients[combined_order] = add(result.coefficients[combined_order], term);
          else
            result.coefficients[combined_order] = term;
        }
      }
    }
  }
  
  return result;
}

Differential DifferentiationVisitor::handleDivide(const NodePtr & left, const NodePtr & right)
{
  Differential left_diff = visit(left);
  Differential right_diff = visit(right);
  
  bool right_depends = dependsOnVariable(right);
  
  Differential result;
  
  if (!right_depends)
  {
    for (auto & [order, coeff] : left_diff.coefficients)
      if (coeff)
        result.coefficients[order] = divide(coeff, right);
  }
  else
  {
    auto right_sq = multiply(right, right);
    
    for (auto & [order, coeff] : left_diff.coefficients)
      if (coeff)
        result.coefficients[order] = divide(coeff, right);
    
    for (auto & [order, coeff] : right_diff.coefficients)
    {
      if (coeff)
      {
        auto term = divide(multiply(negate(left), coeff), right_sq);
        if (result.coefficients.count(order))
          result.coefficients[order] = add(result.coefficients[order], term);
        else
          result.coefficients[order] = term;
      }
    }
  }
  
  return result;
}

Differential DifferentiationVisitor::handlePower(const NodePtr & left, const NodePtr & right)
{
  if (!dependsOnVariable(right))
  {
    Differential left_diff = visit(left);
    Differential result;
    
    if (auto const_node = std::dynamic_pointer_cast<ConstantNode>(right))
    {
      Real exponent = const_node->value().asScalar();
      
      for (auto & [order, coeff] : left_diff.coefficients)
      {
        if (coeff)
        {
          auto factor = multiply(constant(exponent), power(left, constant(exponent - 1)));
          result.coefficients[order] = multiply(factor, coeff);
        }
      }
    }
    
    return result;
  }
  else
  {
    mooseError("Power with variable exponent not yet supported");
  }
}

Differential DifferentiationVisitor::handleDot(const NodePtr & left, const NodePtr & right)
{
  Differential left_diff = visit(left);
  Differential right_diff = visit(right);
  
  Differential result;
  
  // Handle d/du dot(left, right) = dot(d(left)/du, right)
  for (auto & [order, coeff] : left_diff.coefficients)
  {
    if (coeff)
    {
      // If coeff is a scalar (from differentiating a field variable),
      // we need to multiply by it, not take dot product
      if (coeff->type() == NodeType::Constant)
      {
        auto const_node = static_cast<const ConstantNode*>(coeff.get());
        if (const_node->value().isScalar())
          result.coefficients[order] = multiply(coeff, right);
        else
          result.coefficients[order] = dot(coeff, right);
      }
      else
        result.coefficients[order] = dot(coeff, right);
    }
  }
  
  // Handle d/du dot(left, right) = dot(left, d(right)/du)
  for (auto & [order, coeff] : right_diff.coefficients)
  {
    if (coeff)
    {
      NodePtr term;
      // If coeff is a scalar, multiply instead of dot
      if (coeff->type() == NodeType::Constant)
      {
        auto const_node = static_cast<const ConstantNode*>(coeff.get());
        if (const_node->value().isScalar())
          term = multiply(coeff, left);
        else
          term = dot(left, coeff);
      }
      else
        term = dot(left, coeff);
        
      if (result.coefficients.count(order))
        result.coefficients[order] = add(result.coefficients[order], term);
      else
        result.coefficients[order] = term;
    }
  }
  
  return result;
}

Differential DifferentiationVisitor::handleContract(const NodePtr & left, const NodePtr & right)
{
  Differential left_diff = visit(left);
  Differential right_diff = visit(right);
  
  Differential result;
  
  for (auto & [order, coeff] : left_diff.coefficients)
    if (coeff)
      result.coefficients[order] = contract(coeff, right);
  
  for (auto & [order, coeff] : right_diff.coefficients)
  {
    if (coeff)
    {
      auto term = contract(left, coeff);
      if (result.coefficients.count(order))
        result.coefficients[order] = add(result.coefficients[order], term);
      else
        result.coefficients[order] = term;
    }
  }
  
  return result;
}

Differential DifferentiationVisitor::handleOuter(const NodePtr & left, const NodePtr & right)
{
  Differential left_diff = visit(left);
  Differential right_diff = visit(right);
  
  Differential result;
  
  for (auto & [order, coeff] : left_diff.coefficients)
    if (coeff)
      result.coefficients[order] = outer(coeff, right);
  
  for (auto & [order, coeff] : right_diff.coefficients)
  {
    if (coeff)
    {
      auto term = outer(left, coeff);
      if (result.coefficients.count(order))
        result.coefficients[order] = add(result.coefficients[order], term);
      else
        result.coefficients[order] = term;
    }
  }
  
  return result;
}

bool DifferentiationVisitor::dependsOnVariable(const NodePtr & expr) const
{
  if (!expr)
    return false;
  
  if (expr->type() == NodeType::Variable)
  {
    auto var = std::static_pointer_cast<VariableNode>(expr);
    return var->name() == _var_name;
  }
  
  if (expr->type() == NodeType::FieldVariable)
  {
    auto var = std::static_pointer_cast<FieldVariableNode>(expr);
    return var->name() == _var_name;
  }
  
  for (const auto & child : expr->children())
    if (dependsOnVariable(child))
      return true;
  
  return false;
}

NodePtr DifferentiationVisitor::applyChainRule(const NodePtr & df_du, const Differential & du_dx)
{
  NodePtr result = nullptr;
  
  for (auto & [order, coeff] : du_dx.coefficients)
  {
    if (coeff)
    {
      auto term = multiply(df_du, coeff);
      if (result)
        result = add(result, term);
      else
        result = term;
    }
  }
  
  return result;
}

Differential DifferentiationVisitor::combineDifferentials(
    const Differential & d1,
    const Differential & d2,
    std::function<NodePtr(const NodePtr &, const NodePtr &)> combiner)
{
  Differential result;
  
  unsigned int max_order = std::max(d1.maxOrder(), d2.maxOrder());
  
  for (unsigned int order = 0; order <= max_order; ++order)
  {
    NodePtr c1 = d1.getCoefficient(order);
    NodePtr c2 = d2.getCoefficient(order);
    
    if (c1 && c2)
      result.coefficients[order] = combiner(c1, c2);
    else if (c1)
      result.coefficients[order] = c1;
    else if (c2)
      result.coefficients[order] = c2;
  }
  
  return result;
}

NodePtr WeakFormGenerator::generateWeakForm(const NodePtr & energy_density, const std::string & var_name)
{
  DifferentiationVisitor dv(var_name);
  Differential diff = dv.differentiate(energy_density);
  
  // Debug: Print differential coefficients
  mooseInfo("Differential coefficients for variable ", var_name, ":");
  for (auto & [order, coeff] : diff.coefficients)
  {
    if (coeff)
      mooseInfo("  C^", order, " = ", coeff->toString());
  }
  
  return computeEulerLagrange(diff);
}

NodePtr WeakFormGenerator::computeEulerLagrange(const Differential & diff)
{
  NodePtr result = nullptr;
  
  for (auto & [order, coeff] : diff.coefficients)
  {
    if (coeff)
    {
      NodePtr term = coeff;
      mooseInfo("  Processing C^", order, " = ", term->toString());
      
      for (unsigned int i = 0; i < order; ++i)
      {
        term = applyDivergence(term);
        mooseInfo("    After divergence ", i+1, ": ", term->toString());
      }
      
      if (order % 2 == 1)
      {
        term = negate(term);
        mooseInfo("    After negation: ", term->toString());
      }
      
      if (result)
        result = add(result, term);
      else
        result = term;
    }
  }
  
  mooseInfo("Final Euler-Lagrange equation: ", result ? result->toString() : "null");
  return result;
}

NodePtr WeakFormGenerator::computeResidualContribution(
    const Differential & diff,
    const std::string & var_name,
    bool use_test_functions)
{
  NodePtr result = nullptr;
  
  for (auto & [order, coeff] : diff.coefficients)
  {
    if (coeff)
    {
      NodePtr term = coeff;
      
      if (use_test_functions)
        term = multiplyByTestFunction(term, var_name, order);
      
      if (order % 2 == 1)
        term = negate(term);
      
      if (result)
        result = add(result, term);
      else
        result = term;
    }
  }
  
  return result;
}

WeakFormGenerator::WeakFormContributions WeakFormGenerator::computeContributions(
    const NodePtr & energy_density,
    const std::string & var_name)
{
  DifferentiationVisitor dv(var_name);
  Differential diff = dv.differentiate(energy_density);
  
  WeakFormContributions contributions;
  contributions.max_order = diff.maxOrder();
  
  contributions.c0_term = diff.getCoefficient(0);
  contributions.c1_term = diff.getCoefficient(1);
  contributions.c2_term = diff.getCoefficient(2);
  contributions.c3_term = diff.getCoefficient(3);
  
  contributions.total_residual = computeResidualContribution(diff, var_name);
  
  return contributions;
}

bool WeakFormGenerator::requiresVariableSplitting(const Differential & diff, unsigned int fe_order) const
{
  unsigned int max_deriv = diff.maxOrder();
  
  if (fe_order == 1 && max_deriv > 1)
    return true;
  if (fe_order == 2 && max_deriv > 2)
    return true;
  
  return false;
}

NodePtr WeakFormGenerator::applyDivergence(const NodePtr & expr, unsigned int times)
{
  NodePtr result = expr;
  for (unsigned int i = 0; i < times; ++i)
    result = div(result);
  return result;
}

NodePtr WeakFormGenerator::multiplyByTestFunction(
    const NodePtr & expr,
    const std::string & var_name,
    unsigned int derivative_order)
{
  if (derivative_order == 0)
    return multiply(expr, testFunction(var_name, false));
  else if (derivative_order == 1)
    return dot(expr, testFunction(var_name, true));
  else
    mooseError("Test functions for derivative order > 1 not yet implemented");
}

NodePtr SymbolicSimplifier::simplify(const NodePtr & expr)
{
  if (!expr)
    return expr;
  
  std::string key = expr->toString();
  if (_simplified_cache.count(key))
    return _simplified_cache[key];
  
  NodePtr result = simplifyNode(expr);
  _simplified_cache[key] = result;
  return result;
}

NodePtr SymbolicSimplifier::simplifyNode(const NodePtr & node)
{
  if (!node)
    return node;
  
  switch (node->type())
  {
    case NodeType::Add:
      return simplifyAdd(static_cast<const BinaryOpNode *>(node.get()));
    case NodeType::Multiply:
      return simplifyMultiply(static_cast<const BinaryOpNode *>(node.get()));
    case NodeType::Divide:
      return simplifyDivide(static_cast<const BinaryOpNode *>(node.get()));
    case NodeType::Power:
      return simplifyPower(static_cast<const BinaryOpNode *>(node.get()));
    default:
      return node;
  }
}

NodePtr SymbolicSimplifier::simplifyAdd(const BinaryOpNode * node)
{
  NodePtr left = simplify(node->left());
  NodePtr right = simplify(node->right());
  
  if (isZero(left))
    return right;
  if (isZero(right))
    return left;
  
  if (isConstant(left) && isConstant(right))
    return foldConstants(left, right, NodeType::Add);
  
  return add(left, right);
}

NodePtr SymbolicSimplifier::simplifyMultiply(const BinaryOpNode * node)
{
  NodePtr left = simplify(node->left());
  NodePtr right = simplify(node->right());
  
  if (isZero(left) || isZero(right))
    return constant(0.0);
  
  if (isOne(left))
    return right;
  if (isOne(right))
    return left;
  
  if (isConstant(left) && isConstant(right))
    return foldConstants(left, right, NodeType::Multiply);
  
  return multiply(left, right);
}

NodePtr SymbolicSimplifier::simplifyDivide(const BinaryOpNode * node)
{
  NodePtr left = simplify(node->left());
  NodePtr right = simplify(node->right());
  
  if (isZero(left))
    return constant(0.0);
  
  if (isOne(right))
    return left;
  
  if (isConstant(left) && isConstant(right))
    return foldConstants(left, right, NodeType::Divide);
  
  return divide(left, right);
}

NodePtr SymbolicSimplifier::simplifyPower(const BinaryOpNode * node)
{
  NodePtr left = simplify(node->left());
  NodePtr right = simplify(node->right());
  
  if (isOne(left))
    return constant(1.0);
  
  if (isZero(right))
    return constant(1.0);
  
  if (isOne(right))
    return left;
  
  if (isConstant(left) && isConstant(right))
    return foldConstants(left, right, NodeType::Power);
  
  return power(left, right);
}

bool SymbolicSimplifier::isZero(const NodePtr & node) const
{
  if (node->type() != NodeType::Constant)
    return false;
  
  auto const_node = std::static_pointer_cast<ConstantNode>(node);
  if (const_node->value().isScalar())
    return std::abs(const_node->value().asScalar()) < 1e-12;
  
  return false;
}

bool SymbolicSimplifier::isOne(const NodePtr & node) const
{
  if (node->type() != NodeType::Constant)
    return false;
  
  auto const_node = std::static_pointer_cast<ConstantNode>(node);
  if (const_node->value().isScalar())
    return std::abs(const_node->value().asScalar() - 1.0) < 1e-12;
  
  return false;
}

bool SymbolicSimplifier::isConstant(const NodePtr & node) const
{
  return node->type() == NodeType::Constant;
}

Real SymbolicSimplifier::getConstantValue(const NodePtr & node) const
{
  if (node->type() != NodeType::Constant)
    return 0.0;
  
  auto const_node = std::static_pointer_cast<ConstantNode>(node);
  if (const_node->value().isScalar())
    return const_node->value().asScalar();
  
  return 0.0;
}

NodePtr SymbolicSimplifier::foldConstants(const NodePtr & left, const NodePtr & right, NodeType op)
{
  Real left_val = getConstantValue(left);
  Real right_val = getConstantValue(right);
  
  switch (op)
  {
    case NodeType::Add:
      return constant(left_val + right_val);
    case NodeType::Subtract:
      return constant(left_val - right_val);
    case NodeType::Multiply:
      return constant(left_val * right_val);
    case NodeType::Divide:
      if (std::abs(right_val) > 1e-12)
        return constant(left_val / right_val);
      else
        mooseError("Division by zero in constant folding");
    case NodeType::Power:
      return constant(std::pow(left_val, right_val));
    default:
      mooseError("Unsupported operation in constant folding");
  }
}

}
}