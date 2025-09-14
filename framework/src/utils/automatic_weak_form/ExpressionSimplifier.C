#include "ExpressionSimplifier.h"
#include "MooseAST.h"

namespace moose
{
namespace automatic_weak_form
{

NodePtr ExpressionSimplifier::simplify(const NodePtr & expr)
{
  if (!expr)
    return nullptr;
  return simplifyNode(expr);
}

NodePtr ExpressionSimplifier::simplifyNode(const NodePtr & node)
{
  if (!node)
    return nullptr;
    
  switch (node->type())
  {
    case NodeType::Constant:
    case NodeType::FieldVariable:
      return node;
      
    case NodeType::Add:
    case NodeType::Subtract:
    case NodeType::Multiply:
    case NodeType::Divide:
    case NodeType::Power:
      return simplifyBinaryOp(static_cast<const BinaryOpNode*>(node.get()));
      
    case NodeType::Negate:
      return simplifyUnaryOp(static_cast<const UnaryOpNode*>(node.get()));
      
    case NodeType::Gradient:
    {
      auto unary = static_cast<const UnaryOpNode*>(node.get());
      auto simplified_operand = simplifyNode(unary->operand());
      if (simplified_operand != unary->operand())
        return std::make_shared<UnaryOpNode>(NodeType::Gradient, simplified_operand, node->shape());
      return node;
    }
    
    case NodeType::Divergence:
    {
      auto unary = static_cast<const UnaryOpNode*>(node.get());
      auto simplified_operand = simplifyNode(unary->operand());
      if (simplified_operand != unary->operand())
        return std::make_shared<UnaryOpNode>(NodeType::Divergence, simplified_operand, node->shape());
      return node;
    }
    
    case NodeType::Dot:
    {
      auto binary = static_cast<const BinaryOpNode*>(node.get());
      auto left = simplifyNode(binary->left());
      auto right = simplifyNode(binary->right());
      if (left != binary->left() || right != binary->right())
        return dot(left, right);
      return node;
    }
    
    case NodeType::Function:
    {
      auto func = static_cast<const FunctionNode*>(node.get());
      std::vector<NodePtr> simplified_args;
      bool changed = false;
      for (const auto & arg : func->args())
      {
        auto simplified = simplifyNode(arg);
        simplified_args.push_back(simplified);
        if (simplified != arg)
          changed = true;
      }
      if (changed)
        return std::make_shared<FunctionNode>(func->name(), simplified_args, node->shape());
      return node;
    }
    
    default:
      return node;
  }
}

NodePtr ExpressionSimplifier::simplifyBinaryOp(const BinaryOpNode * node)
{
  auto left = simplifyNode(node->left());
  auto right = simplifyNode(node->right());
  
  switch (node->type())
  {
    case NodeType::Add:
      return simplifyAdd(left, right);
    case NodeType::Subtract:
      return simplifySubtract(left, right);
    case NodeType::Multiply:
      return simplifyMultiply(left, right);
    case NodeType::Divide:
      return simplifyDivide(left, right);
    case NodeType::Power:
      return simplifyPower(left, right);
    default:
      if (left != node->left() || right != node->right())
        return std::make_shared<BinaryOpNode>(node->type(), left, right, node->shape());
      // Return a clone since we can't return const node as non-const shared_ptr
      return node->clone();
  }
}

NodePtr ExpressionSimplifier::simplifyUnaryOp(const UnaryOpNode * node)
{
  auto operand = simplifyNode(node->operand());
  
  if (node->type() == NodeType::Negate)
    return simplifyNegate(operand);
    
  if (operand != node->operand())
    return std::make_shared<UnaryOpNode>(node->type(), operand, node->shape());
  return node->clone();
}

NodePtr ExpressionSimplifier::simplifyAdd(const NodePtr & left, const NodePtr & right)
{
  // 0 + x = x
  if (isZero(left))
    return right;
  // x + 0 = x
  if (isZero(right))
    return left;
    
  // Constant folding
  if (left->type() == NodeType::Constant && right->type() == NodeType::Constant)
  {
    auto left_const = static_cast<const ConstantNode*>(left.get());
    auto right_const = static_cast<const ConstantNode*>(right.get());
    if (left_const->value().isScalar() && right_const->value().isScalar())
    {
      Real sum = left_const->value().asScalar() + right_const->value().asScalar();
      return constant(sum);
    }
  }
  
  // x + x = 2*x
  if (left->equals(*right))
    return multiply(constant(2.0), left);
    
  // a*x + b*x = (a+b)*x
  if (left->type() == NodeType::Multiply && right->type() == NodeType::Multiply)
  {
    auto left_mul = static_cast<const BinaryOpNode*>(left.get());
    auto right_mul = static_cast<const BinaryOpNode*>(right.get());
    
    // Check if they have the same variable part
    if (left_mul->right()->equals(*right_mul->right()) && 
        left_mul->left()->type() == NodeType::Constant &&
        right_mul->left()->type() == NodeType::Constant)
    {
      auto left_coeff = static_cast<const ConstantNode*>(left_mul->left().get());
      auto right_coeff = static_cast<const ConstantNode*>(right_mul->left().get());
      if (left_coeff->value().isScalar() && right_coeff->value().isScalar())
      {
        Real sum = left_coeff->value().asScalar() + right_coeff->value().asScalar();
        if (std::abs(sum) < 1e-10)
          return constant(0.0);
        return multiply(constant(sum), left_mul->right());
      }
    }
  }
  
  return add(left, right);
}

NodePtr ExpressionSimplifier::simplifySubtract(const NodePtr & left, const NodePtr & right)
{
  // x - 0 = x
  if (isZero(right))
    return left;
  // 0 - x = -x
  if (isZero(left))
    return negate(right);
  // x - x = 0
  if (left->equals(*right))
    return constant(0.0);
    
  // Constant folding
  if (left->type() == NodeType::Constant && right->type() == NodeType::Constant)
  {
    auto left_const = static_cast<const ConstantNode*>(left.get());
    auto right_const = static_cast<const ConstantNode*>(right.get());
    if (left_const->value().isScalar() && right_const->value().isScalar())
    {
      Real diff = left_const->value().asScalar() - right_const->value().asScalar();
      return constant(diff);
    }
  }
  
  return subtract(left, right);
}

NodePtr ExpressionSimplifier::simplifyMultiply(const NodePtr & left, const NodePtr & right)
{
  // 0 * x = 0
  if (isZero(left) || isZero(right))
    return constant(0.0);
  // 1 * x = x
  if (isOne(left))
    return right;
  // x * 1 = x
  if (isOne(right))
    return left;
    
  // Constant folding
  if (left->type() == NodeType::Constant && right->type() == NodeType::Constant)
  {
    auto left_const = static_cast<const ConstantNode*>(left.get());
    auto right_const = static_cast<const ConstantNode*>(right.get());
    if (left_const->value().isScalar() && right_const->value().isScalar())
    {
      Real product = left_const->value().asScalar() * right_const->value().asScalar();
      return constant(product);
    }
  }
  
  // Combine constants: (a * x) * b = (a*b) * x
  if (left->type() == NodeType::Multiply && right->type() == NodeType::Constant)
  {
    auto left_mul = static_cast<const BinaryOpNode*>(left.get());
    if (left_mul->left()->type() == NodeType::Constant)
    {
      auto combined = simplifyMultiply(left_mul->left(), right);
      if (combined->type() == NodeType::Constant)
        return multiply(combined, left_mul->right());
    }
  }
  
  // x * x = x^2
  if (left->equals(*right))
    return power(left, constant(2.0));
    
  return multiply(left, right);
}

NodePtr ExpressionSimplifier::simplifyDivide(const NodePtr & left, const NodePtr & right)
{
  // 0 / x = 0
  if (isZero(left))
    return constant(0.0);
  // x / 1 = x
  if (isOne(right))
    return left;
  // x / x = 1
  if (left->equals(*right))
    return constant(1.0);
    
  // Constant folding
  if (left->type() == NodeType::Constant && right->type() == NodeType::Constant)
  {
    auto left_const = static_cast<const ConstantNode*>(left.get());
    auto right_const = static_cast<const ConstantNode*>(right.get());
    if (left_const->value().isScalar() && right_const->value().isScalar())
    {
      Real divisor = right_const->value().asScalar();
      if (std::abs(divisor) > 1e-10)
      {
        Real quotient = left_const->value().asScalar() / divisor;
        return constant(quotient);
      }
    }
  }
  
  return divide(left, right);
}

NodePtr ExpressionSimplifier::simplifyPower(const NodePtr & left, const NodePtr & right)
{
  // x^0 = 1
  if (isZero(right))
    return constant(1.0);
  // x^1 = x
  if (isOne(right))
    return left;
  // 0^x = 0 (for x > 0)
  if (isZero(left))
    return constant(0.0);
  // 1^x = 1
  if (isOne(left))
    return constant(1.0);
    
  // Constant folding
  if (left->type() == NodeType::Constant && right->type() == NodeType::Constant)
  {
    auto left_const = static_cast<const ConstantNode*>(left.get());
    auto right_const = static_cast<const ConstantNode*>(right.get());
    if (left_const->value().isScalar() && right_const->value().isScalar())
    {
      Real base = left_const->value().asScalar();
      Real exponent = right_const->value().asScalar();
      if (base > 0 || (base < 0 && std::floor(exponent) == exponent))
      {
        Real result = std::pow(base, exponent);
        return constant(result);
      }
    }
  }
  
  return power(left, right);
}

NodePtr ExpressionSimplifier::simplifyNegate(const NodePtr & operand)
{
  // -(-x) = x
  if (operand->type() == NodeType::Negate)
  {
    auto neg = static_cast<const UnaryOpNode*>(operand.get());
    return neg->operand();
  }
  
  // -(constant)
  if (operand->type() == NodeType::Constant)
  {
    auto const_node = static_cast<const ConstantNode*>(operand.get());
    if (const_node->value().isScalar())
    {
      Real negated = -const_node->value().asScalar();
      return constant(negated);
    }
  }
  
  // -0 = 0
  if (isZero(operand))
    return constant(0.0);
    
  return negate(operand);
}

bool ExpressionSimplifier::isZero(const NodePtr & node)
{
  return isConstant(node, 0.0);
}

bool ExpressionSimplifier::isOne(const NodePtr & node)
{
  return isConstant(node, 1.0);
}

bool ExpressionSimplifier::isConstant(const NodePtr & node, Real value)
{
  if (!node || node->type() != NodeType::Constant)
    return false;
  auto const_node = static_cast<const ConstantNode*>(node.get());
  if (!const_node->value().isScalar())
    return false;
  return std::abs(const_node->value().asScalar() - value) < 1e-10;
}

Real ExpressionSimplifier::getConstantValue(const NodePtr & node)
{
  if (!node || node->type() != NodeType::Constant)
    return 0.0;
  auto const_node = static_cast<const ConstantNode*>(node.get());
  if (!const_node->value().isScalar())
    return 0.0;
  return const_node->value().asScalar();
}

} // namespace automatic_weak_form
} // namespace moose