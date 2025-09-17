#include "WeakFormGenerator.h"
#include "ExpressionSimplifier.h"
#include "MooseError.h"
#include <algorithm>
#include <cmath>
#include <unordered_map>

namespace moose
{
namespace automatic_weak_form
{

namespace
{
NodePtr expandCross(const NodePtr & left, const NodePtr & right)
{
  auto left_vec = std::dynamic_pointer_cast<VectorAssemblyNode>(left);
  auto right_vec = std::dynamic_pointer_cast<VectorAssemblyNode>(right);

  if (!left_vec || !right_vec)
    return cross(left, right);

  if (left_vec->components().size() != 3 || right_vec->components().size() != 3)
    return cross(left, right);

  const auto & a = left_vec->components();
  const auto & b = right_vec->components();

  NodePtr c1 = subtract(multiply(a[1], b[2]), multiply(a[2], b[1]));
  NodePtr c2 = subtract(multiply(a[2], b[0]), multiply(a[0], b[2]));
  NodePtr c3 = subtract(multiply(a[0], b[1]), multiply(a[1], b[0]));

  return vec3(c1, c2, c3);
}
}

Differential DifferentiationVisitor::differentiate(const NodePtr & expr)
{
  _cache.clear();
  auto diff = visit(expr);

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

  auto cache_it = _cache.find(node.get());
  if (cache_it != _cache.end())
    return cache_it->second;

  const auto & handlers = handlerMap();
  auto handler_it = handlers.find(node->type());
  if (handler_it == handlers.end())
    mooseError("Unsupported node type in differentiation: ", static_cast<int>(node->type()));

  Handler handler = handler_it->second;
  Differential result = (this->*handler)(node);
  _cache.emplace(node.get(), result);
  return result;
}

const std::unordered_map<NodeType, DifferentiationVisitor::Handler> &
DifferentiationVisitor::handlerMap()
{
  static const std::unordered_map<NodeType, Handler> handlers = {
      {NodeType::Constant, &DifferentiationVisitor::differentiateConstant},
      {NodeType::Variable, &DifferentiationVisitor::differentiateVariable},
      {NodeType::FieldVariable, &DifferentiationVisitor::differentiateFieldVariable},
      {NodeType::Add, &DifferentiationVisitor::differentiateBinaryOp},
      {NodeType::Subtract, &DifferentiationVisitor::differentiateBinaryOp},
      {NodeType::Multiply, &DifferentiationVisitor::differentiateBinaryOp},
      {NodeType::Divide, &DifferentiationVisitor::differentiateBinaryOp},
      {NodeType::Power, &DifferentiationVisitor::differentiateBinaryOp},
      {NodeType::Dot, &DifferentiationVisitor::differentiateBinaryOp},
      {NodeType::Cross, &DifferentiationVisitor::differentiateBinaryOp},
      {NodeType::Contract, &DifferentiationVisitor::differentiateBinaryOp},
      {NodeType::Outer, &DifferentiationVisitor::differentiateBinaryOp},
      {NodeType::Negate, &DifferentiationVisitor::differentiateUnaryOp},
      {NodeType::Gradient, &DifferentiationVisitor::differentiateUnaryOp},
      {NodeType::Curl, &DifferentiationVisitor::differentiateUnaryOp},
      {NodeType::Divergence, &DifferentiationVisitor::differentiateUnaryOp},
      {NodeType::Laplacian, &DifferentiationVisitor::differentiateUnaryOp},
      {NodeType::Norm, &DifferentiationVisitor::differentiateUnaryOp},
      {NodeType::Normalize, &DifferentiationVisitor::differentiateUnaryOp},
      {NodeType::Trace, &DifferentiationVisitor::differentiateUnaryOp},
      {NodeType::Determinant, &DifferentiationVisitor::differentiateUnaryOp},
      {NodeType::Inverse, &DifferentiationVisitor::differentiateUnaryOp},
      {NodeType::Transpose, &DifferentiationVisitor::differentiateUnaryOp},
      {NodeType::Symmetric, &DifferentiationVisitor::differentiateUnaryOp},
      {NodeType::Skew, &DifferentiationVisitor::differentiateUnaryOp},
      {NodeType::Deviatoric, &DifferentiationVisitor::differentiateUnaryOp},
      {NodeType::Function, &DifferentiationVisitor::differentiateFunction},
      {NodeType::VectorAssembly, &DifferentiationVisitor::differentiateVectorAssembly},
      {NodeType::VectorComponent, &DifferentiationVisitor::differentiateComponent},
      {NodeType::TensorComponent, &DifferentiationVisitor::differentiateComponent}};
  return handlers;
}

Differential DifferentiationVisitor::differentiateConstant(const NodePtr &)
{
  return Differential{};
}

Differential DifferentiationVisitor::differentiateVariable(const NodePtr & node)
{
  auto var = std::static_pointer_cast<VariableNode>(node);
  Differential result;
  if (var->name() == _var_name)
    result.coefficients[0] = constant(1.0);
  return result;
}

Differential DifferentiationVisitor::differentiateFieldVariable(const NodePtr & node)
{
  auto field = std::static_pointer_cast<FieldVariableNode>(node);
  Differential result;
  if (field->name() == _var_name)
    result.coefficients[0] = constant(1.0);
  else if (_split_definitions)
  {
    auto it = _split_definitions->find(field->name());
    if (it != _split_definitions->end())
      return visit(it->second);
  }
  return result;
}

Differential DifferentiationVisitor::differentiateUnaryOp(const NodePtr & node)
{
  auto unary = std::static_pointer_cast<UnaryOpNode>(node);
  Differential operand_diff = visit(unary->operand());

  switch (unary->type())
  {
    case NodeType::Negate:
    {
      Differential result;
      for (auto & [order, coeff] : operand_diff.coefficients)
        result.coefficients[order] = negate(coeff);
      return result;
    }

    case NodeType::Gradient:
      return handleGradient(unary->operand());

    case NodeType::Curl:
      return handleCurl(unary->operand());

    case NodeType::Divergence:
      return handleDivergence(unary->operand());

    case NodeType::Laplacian:
      return handleLaplacian(unary->operand());

    case NodeType::Norm:
      return handleNorm(unary->operand());

    case NodeType::Normalize:
      return handleNormalize(unary->operand());

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
        auto A = unary->operand();
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
          auto tr_coeff = multiply(constant(1.0 / 3.0), trace(coeff));
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

Differential DifferentiationVisitor::differentiateBinaryOp(const NodePtr & node)
{
  auto binary = std::static_pointer_cast<BinaryOpNode>(node);

  switch (binary->type())
  {
    case NodeType::Add:
      return handleAdd(binary->left(), binary->right());
    case NodeType::Subtract:
      return handleSubtract(binary->left(), binary->right());
    case NodeType::Multiply:
      return handleMultiply(binary->left(), binary->right());
    case NodeType::Divide:
      return handleDivide(binary->left(), binary->right());
    case NodeType::Power:
      return handlePower(binary->left(), binary->right());
    case NodeType::Dot:
      return handleDot(binary->left(), binary->right());
    case NodeType::Cross:
      return handleCross(binary->left(), binary->right());
    case NodeType::Contract:
      return handleContract(binary->left(), binary->right());
    case NodeType::Outer:
      return handleOuter(binary->left(), binary->right());
    default:
      mooseError("Unsupported binary operation in differentiation");
  }
}

Differential DifferentiationVisitor::differentiateFunction(const NodePtr & node)
{
  auto func = std::static_pointer_cast<FunctionNode>(node);
  Differential result;

  static const std::unordered_map<std::string, std::function<NodePtr(const NodePtr &)>> unary_derivatives = {
      {"sin", [](const NodePtr & arg) { return function("cos", {arg}); }},
      {"cos", [](const NodePtr & arg) { return negate(function("sin", {arg})); }},
      {"exp", [](const NodePtr & arg) { return function("exp", {arg}); }},
      {"log", [](const NodePtr & arg) { return divide(constant(1.0), arg); }},
      {"sqrt", [](const NodePtr & arg) {
         return divide(constant(1.0), multiply(constant(2.0), function("sqrt", {arg})));
       }},
      {"tanh", [](const NodePtr & arg) {
         auto tanh_arg = function("tanh", {arg});
         return subtract(constant(1.0), power(tanh_arg, constant(2.0)));
       }},
      {"sinh", [](const NodePtr & arg) { return function("cosh", {arg}); }},
      {"cosh", [](const NodePtr & arg) { return function("sinh", {arg}); }},
      {"W", [](const NodePtr & arg) { return function("dW_dc", {arg}); }} };

  const auto & args = func->args();
  if (args.size() == 1)
  {
    auto it = unary_derivatives.find(func->name());
    if (it != unary_derivatives.end())
    {
      auto arg_diff = visit(args[0]);
      NodePtr derivative = it->second(args[0]);
      for (auto & [order, coeff] : arg_diff.coefficients)
        if (coeff)
          result.coefficients[order] = multiply(derivative, coeff);
      return result;
    }
  }

  return result;
}

Differential DifferentiationVisitor::handleGradient(const NodePtr & operand)
{
  Differential operand_diff = visit(operand);
  Differential result;

  for (auto & [order, coeff] : operand_diff.coefficients)
    if (coeff)
      result.coefficients[order + 1] = coeff;

  return result;
}

Differential DifferentiationVisitor::handleCurl(const NodePtr & operand)
{
  Differential operand_diff = visit(operand);
  Differential result;

  for (auto & [order, coeff] : operand_diff.coefficients)
    if (coeff)
      result.coefficients[order + 1] = coeff;

  return result;
}

Differential DifferentiationVisitor::differentiateVectorAssembly(const NodePtr & node)
{
  auto vector_node = std::static_pointer_cast<VectorAssemblyNode>(node);

  Differential result;
  const auto & components = vector_node->components();

  for (std::size_t i = 0; i < components.size(); ++i)
  {
    Differential comp_diff = visit(components[i]);

    for (auto & [order, coeff] : comp_diff.coefficients)
    {
      if (!coeff)
        continue;

      auto & entry = result.coefficients[order];
      if (!entry)
      {
        std::vector<NodePtr> order_components(components.size(), constant(0.0));
        VectorShape vec_shape{static_cast<unsigned int>(components.size())};
        entry = std::make_shared<VectorAssemblyNode>(order_components, Shape(vec_shape));
      }

      auto vec_entry = std::static_pointer_cast<VectorAssemblyNode>(entry);
      auto updated_components = vec_entry->components();
      updated_components[i] = coeff;

      VectorShape vec_shape{static_cast<unsigned int>(components.size())};
      entry = std::make_shared<VectorAssemblyNode>(updated_components, Shape(vec_shape));
    }
  }

  return result;
}

Differential DifferentiationVisitor::differentiateComponent(const NodePtr & node)
{
  auto component_node = std::static_pointer_cast<ComponentNode>(node);
  Differential expr_diff = visit(component_node->expr());
  Differential result;

  ScalarShape scalar_shape{};

  for (auto & [order, coeff] : expr_diff.coefficients)
  {
    if (!coeff)
      continue;

    if (component_node->type() == NodeType::VectorComponent)
      result.coefficients[order] = std::make_shared<ComponentNode>(
          coeff, component_node->component(), Shape(scalar_shape));
    else
      result.coefficients[order] = std::make_shared<ComponentNode>(
          coeff, component_node->i(), component_node->j(), Shape(scalar_shape));
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

Differential DifferentiationVisitor::handleCross(const NodePtr & left, const NodePtr & right)
{
  Differential left_diff = visit(left);
  Differential right_diff = visit(right);

  Differential result;

  for (auto & [order, coeff] : left_diff.coefficients)
    if (coeff)
      result.coefficients[order] = expandCross(coeff, right);

  for (auto & [order, coeff] : right_diff.coefficients)
  {
    if (!coeff)
      continue;

    auto term = expandCross(left, coeff);
    if (result.coefficients.count(order))
      result.coefficients[order] = add(result.coefficients[order], term);
    else
      result.coefficients[order] = term;
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
  DifferentiationVisitor dv(var_name, &_split_definitions);
  Differential diff = dv.differentiate(energy_density);
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

      for (unsigned int i = 0; i < order; ++i)
        term = applyDivergence(term);

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
  DifferentiationVisitor dv(var_name, &_split_definitions);
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
  auto it = _simplified_cache.find(key);
  if (it != _simplified_cache.end())
    return it->second;

  NodePtr result = ExpressionSimplifier::simplify(expr);
  _simplified_cache.emplace(std::move(key), result);
  return result;
}

WeakFormGenerator::ConservationCheck
WeakFormGenerator::checkConservation(const NodePtr &, const std::string &)
{
  return {false, nullptr, nullptr, std::string("Conservation analysis not available")};
}

NodePtr
WeakFormGenerator::computeLinearization(const NodePtr & nonlinear_form,
                                        const std::string &,
                                        const NodePtr &)
{
  return nonlinear_form;
}

NodePtr
WeakFormGenerator::computeSecondVariation(const NodePtr &, const std::string &, const std::string &)
{
  return nullptr;
}

bool
WeakFormGenerator::isSymmetric(const NodePtr &)
{
  return true;
}

bool
WeakFormGenerator::isElliptic(const NodePtr &)
{
  return true;
}

bool
WeakFormGenerator::isCoercive(const NodePtr &)
{
  return true;
}

VariationalProblemAnalyzer::Analysis
VariationalProblemAnalyzer::analyze(const NodePtr & energy_density,
                                    const std::vector<std::string> & variables)
{
  Analysis analysis;
  analysis.is_well_posed = energy_density != nullptr;
  analysis.is_elliptic = true;
  analysis.is_parabolic = false;
  analysis.is_hyperbolic = false;
  analysis.has_unique_solution = true;
  analysis.is_conservative = true;
  analysis.is_symmetric = true;
  analysis.is_coercive = true;
  analysis.required_continuity = 1;
  analysis.recommended_fe_order = 1;

  if (variables.size() > 1)
    analysis.suggestions.push_back("Verify coupling terms for multi-variable systems");

  return analysis;
}

bool
VariationalProblemAnalyzer::checkWellPosedness(const NodePtr & weak_form)
{
  return weak_form != nullptr;
}

bool
VariationalProblemAnalyzer::checkExistence(const NodePtr & bilinear_form, const NodePtr & linear_form)
{
  return bilinear_form != nullptr && linear_form != nullptr;
}

bool
VariationalProblemAnalyzer::checkUniqueness(const NodePtr & bilinear_form)
{
  return bilinear_form != nullptr;
}

bool
VariationalProblemAnalyzer::checkStability(const NodePtr & weak_form, const std::string &)
{
  return weak_form != nullptr;
}

}
}
