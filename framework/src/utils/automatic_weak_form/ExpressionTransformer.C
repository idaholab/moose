#include "ExpressionTransformer.h"
#include "MooseError.h"
#include "MooseAST.h"

using namespace moose::automatic_weak_form;

namespace moose
{
namespace automatic_weak_form
{

NodePtr
ExpressionTransformer::transform(const NodePtr & expr)
{
  if (!expr)
    return nullptr;
    
  return transformNode(expr);
}

NodePtr
ExpressionTransformer::transformNode(const NodePtr & node)
{
  if (!node)
    return nullptr;
    
  switch (node->type())
  {
    case NodeType::Constant:
    case NodeType::FieldVariable:
    case NodeType::Variable:
      // These are leaf nodes, no transformation needed
      return node;
      
    case NodeType::Gradient:
      return transformGradient(static_cast<const UnaryOpNode*>(node.get()));
      
    case NodeType::Divergence:
      return transformDivergence(static_cast<const UnaryOpNode*>(node.get()));
      
    case NodeType::Add:
    case NodeType::Subtract:
    case NodeType::Multiply:
    case NodeType::Divide:
    case NodeType::Power:
    case NodeType::Dot:
    case NodeType::Cross:
    case NodeType::Contract:
    case NodeType::Outer:
    {
      // Binary operations: transform both operands
      auto binary = static_cast<const BinaryOpNode*>(node.get());
      auto left = transformNode(binary->left());
      auto right = transformNode(binary->right());
      
      if (left != binary->left() || right != binary->right())
        return std::make_shared<BinaryOpNode>(node->type(), left, right, node->shape());
      return node;
    }
    
    case NodeType::Negate:
    case NodeType::Norm:
    case NodeType::Normalize:
    case NodeType::Trace:
    case NodeType::Determinant:
    case NodeType::Transpose:
    {
      // Unary operations: transform the operand
      auto unary = static_cast<const UnaryOpNode*>(node.get());
      auto operand = transformNode(unary->operand());
      
      if (operand != unary->operand())
        return std::make_shared<UnaryOpNode>(node->type(), operand, node->shape());
      return node;
    }
    
    case NodeType::Function:
    {
      // Functions: transform all arguments
      auto func = static_cast<const FunctionNode*>(node.get());
      std::vector<NodePtr> transformed_args;
      bool changed = false;
      
      for (const auto & arg : func->args())
      {
        auto transformed = transformNode(arg);
        transformed_args.push_back(transformed);
        if (transformed != arg)
          changed = true;
      }
      
      if (changed)
        return std::make_shared<FunctionNode>(func->name(), transformed_args, node->shape());
      return node;
    }
    
    default:
      return node;
  }
}

NodePtr
ExpressionTransformer::transformGradient(const UnaryOpNode * grad_node)
{
  auto operand = grad_node->operand();
  
  // Check if this is a second-order derivative: grad(grad(u))
  if (operand->type() == NodeType::Gradient)
  {
    auto inner_grad = static_cast<const UnaryOpNode*>(operand.get());
    auto base_expr = inner_grad->operand();
    
    // Check if the base is a field variable
    if (base_expr->type() == NodeType::FieldVariable)
    {
      auto field_var = static_cast<const FieldVariableNode*>(base_expr.get());
      std::string var_name = field_var->name();
      
      // Look for split variable q = grad(u)
      auto split_var = findSplitVariable(var_name, 1);
      if (split_var)
      {
        // Replace grad(grad(u)) with grad(q)
        return std::make_shared<UnaryOpNode>(NodeType::Gradient, split_var, VectorShape{static_cast<unsigned int>(_split_variables.begin()->second.shape.index())});
      }
    }
  }
  // Check if this is grad(u) where u should use a split variable
  else if (operand->type() == NodeType::FieldVariable)
  {
    auto field_var = static_cast<const FieldVariableNode*>(operand.get());
    std::string var_name = field_var->name();
    
    // Check if we should replace grad(u) with the split variable q
    // This happens when max_fe_order is very low (e.g., 0)
    for (const auto & [split_name, sv] : _split_variables)
    {
      if (sv.original_variable == var_name && sv.derivative_order == 1)
      {
        // Instead of grad(u), return the split variable q directly
        // We need to return a vector of component variables
        std::vector<NodePtr> components;
        std::vector<std::string> comp_names = {"x", "y", "z"};
        
        // Use actual mesh dimension
        for (unsigned int i = 0; i < _dim; ++i)
        {
          std::string comp_var_name = sv.name + "_" + comp_names[i];
          components.push_back(fieldVariable(comp_var_name));
        }
        
        // Return as vector assembly
        return std::make_shared<VectorAssemblyNode>(components, VectorShape{_dim});
      }
    }
  }
  
  // No transformation needed, but transform the operand
  auto transformed_operand = transformNode(operand);
  if (transformed_operand != operand)
  {
    // Preserve the original shape from grad_node
    return std::make_shared<UnaryOpNode>(NodeType::Gradient, transformed_operand, grad_node->shape());
  }
  
  return std::make_shared<UnaryOpNode>(*grad_node);
}

NodePtr
ExpressionTransformer::transformDivergence(const UnaryOpNode * div_node)
{
  auto operand = div_node->operand();
  
  // Check if this is div(grad(u)) = laplacian(u)
  if (operand->type() == NodeType::Gradient)
  {
    auto grad_node = static_cast<const UnaryOpNode*>(operand.get());
    auto base_expr = grad_node->operand();
    
    if (base_expr->type() == NodeType::FieldVariable)
    {
      auto field_var = static_cast<const FieldVariableNode*>(base_expr.get());
      std::string var_name = field_var->name();
      
      // Look for split variable q = grad(u)
      auto split_var = findSplitVariable(var_name, 1);
      if (split_var)
      {
        // Replace div(grad(u)) with div(q)
        return std::make_shared<UnaryOpNode>(NodeType::Divergence, split_var, ScalarShape{});
      }
    }
  }
  
  // Transform the operand
  auto transformed_operand = transformNode(operand);
  if (transformed_operand != operand)
    return std::make_shared<UnaryOpNode>(NodeType::Divergence, transformed_operand, ScalarShape{});
  
  return std::make_shared<UnaryOpNode>(*div_node);
}

NodePtr
ExpressionTransformer::findSplitVariable(const std::string & original_var, 
                                          unsigned int derivative_order)
{
  // Look for a split variable matching the original variable and derivative order
  for (const auto & [name, sv] : _split_variables)
  {
    if (sv.original_variable == original_var && sv.derivative_order == derivative_order)
    {
      // For now, return a field variable with the split variable name
      // In reality, we might need to handle vector split variables differently
      return fieldVariable(sv.name);
    }
  }
  
  return nullptr;
}

std::string
ExpressionTransformer::getBaseVariable(const NodePtr & expr)
{
  if (!expr)
    return "";
    
  if (expr->type() == NodeType::FieldVariable)
  {
    auto field_var = static_cast<const FieldVariableNode*>(expr.get());
    return field_var->name();
  }
  else if (expr->type() == NodeType::Gradient || 
           expr->type() == NodeType::Divergence)
  {
    auto unary = static_cast<const UnaryOpNode*>(expr.get());
    return getBaseVariable(unary->operand());
  }
  
  return "";
}

unsigned int
ExpressionTransformer::getDerivativeOrder(const NodePtr & expr)
{
  if (!expr)
    return 0;
    
  if (expr->type() == NodeType::FieldVariable)
    return 0;
  else if (expr->type() == NodeType::Gradient)
  {
    auto unary = static_cast<const UnaryOpNode*>(expr.get());
    return 1 + getDerivativeOrder(unary->operand());
  }
  else if (expr->type() == NodeType::Divergence)
  {
    auto unary = static_cast<const UnaryOpNode*>(expr.get());
    return 1 + getDerivativeOrder(unary->operand());
  }
  
  return 0;
}

} // namespace automatic_weak_form
} // namespace moose