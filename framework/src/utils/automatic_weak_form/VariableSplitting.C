#include "VariableSplitting.h"
#include "WeakFormGenerator.h"
#include "MooseError.h"
#include <algorithm>
#include <array>
#include <functional>
#include <limits>
#include <queue>

namespace moose
{
namespace automatic_weak_form
{

namespace
{

std::vector<SplitVariable>
collectSplitVariables(const std::string & var_name,
                      unsigned int fe_order,
                      const std::map<std::string, SplitVariable> & split_vars)
{
  std::vector<SplitVariable> all_splits;
  all_splits.reserve(split_vars.size());

  for (const auto & [_, sv] : split_vars)
    if (sv.original_variable == var_name)
      all_splits.push_back(sv);

  if (all_splits.empty())
    return {};

  std::sort(all_splits.begin(), all_splits.end(), [](const SplitVariable & a, const SplitVariable & b) {
    if (a.derivative_order == b.derivative_order)
      return a.name < b.name;
    return a.derivative_order < b.derivative_order;
  });

  unsigned int max_required = 0;
  for (const auto & sv : all_splits)
    if (sv.derivative_order > fe_order)
      max_required = std::max(max_required, sv.derivative_order);

  if (!max_required)
    return {};

  std::vector<SplitVariable> result;
  for (const auto & sv : all_splits)
    if (sv.derivative_order <= max_required)
      result.push_back(sv);

  return result;
}

unsigned int
shapeComponentCount(const Shape & shape)
{
  if (std::holds_alternative<ScalarShape>(shape))
    return 1u;

  if (const auto * vec = std::get_if<VectorShape>(&shape))
    return vec->dim;

  if (const auto * tensor = std::get_if<TensorShape>(&shape))
    return tensor->dim * tensor->dim;

  if (const auto * rank3 = std::get_if<RankThreeShape>(&shape))
    return rank3->dim * rank3->dim * rank3->dim;

  if (const auto * rank4 = std::get_if<RankFourShape>(&shape))
    return rank4->dim1 * rank4->dim2;

  return 1u;
}

unsigned int
computePlanDofs(const HigherOrderSplittingStrategy::SplitPlan & plan)
{
  unsigned int total = 1u; // primary variable
  for (const auto & sv : plan.variables)
    total += shapeComponentCount(sv.shape);
  return total;
}

} // namespace

std::vector<VariableSplittingAnalyzer::SplitRequirement>
VariableSplittingAnalyzer::analyzeExpression(const NodePtr & expr)
{
  std::map<std::string, unsigned int> max_orders;
  std::map<std::string, std::map<unsigned int, std::set<NodeType>>> derivative_ops;
  std::vector<NodeType> derivative_stack;
  analyzeNode(expr, max_orders, derivative_ops, derivative_stack);

  std::vector<SplitRequirement> requirements;

  const unsigned int available = _use_hessians ? std::max<unsigned int>(_fe_order, 2) : _fe_order;

  for (const auto & [var_name, max_order] : max_orders)
  {
    SplitRequirement req;
    req.variable_name = var_name;
    req.max_derivative_order = max_order;
    req.available_order = available;
    req.requires_splitting = (max_order > req.available_order);

    if (req.requires_splitting)
      for (unsigned int order = req.available_order + 1; order <= max_order; ++order)
        req.split_orders.push_back(order);

    requirements.push_back(std::move(req));
  }

  return requirements;
}

bool
VariableSplittingAnalyzer::requiresSplitting(const NodePtr & expr) const
{
  auto requirements = const_cast<VariableSplittingAnalyzer *>(this)->analyzeExpression(expr);
  for (const auto & req : requirements)
    if (req.requires_splitting)
      return true;
  return false;
}

unsigned int
VariableSplittingAnalyzer::getMaxDerivativeOrder(const NodePtr & expr, const std::string & var_name) const
{
  std::map<std::string, unsigned int> max_orders;
  std::map<std::string, std::map<unsigned int, std::set<NodeType>>> derivative_ops;
  std::vector<NodeType> derivative_stack;
  const_cast<VariableSplittingAnalyzer *>(this)->analyzeNode(
      expr, max_orders, derivative_ops, derivative_stack);

  auto it = max_orders.find(var_name);
  return it != max_orders.end() ? it->second : 0;
}

std::map<std::string, SplitVariable>
VariableSplittingAnalyzer::generateSplitVariables(const NodePtr & expr)
{
  std::map<std::string, unsigned int> max_orders;
  std::map<std::string, std::map<unsigned int, std::set<NodeType>>> derivative_ops;
  std::vector<NodeType> derivative_stack;
  analyzeNode(expr, max_orders, derivative_ops, derivative_stack);

  std::map<std::string, SplitVariable> split_vars;

  for (const auto & [var_name, max_order] : max_orders)
  {
    if (max_order <= _fe_order)
      continue;

    const Shape original_shape = findVariableShape(expr, var_name);

    for (unsigned int order = 1; order <= max_order; ++order)
    {
      NodePtr definition = buildDerivativeExpression(var_name, order, original_shape, derivative_ops);

      if (!definition)
        mooseError("Unable to build definition for split variable order ", order,
                   " of variable ", var_name);

      if (!split_vars.empty())
      {
        ExpressionTransformer transformer(split_vars, _dim);
        NodePtr transformed = transformer.transform(definition);
        if (transformed)
          definition = transformed;
      }

      SplitVariable sv;
      sv.name = generateSplitVariableName(var_name, order);
      sv.original_variable = var_name;
      sv.derivative_order = order;
      sv.shape = definition->shape();
      sv.definition = definition;
      sv.constraint_residual = subtract(definition->clone(), fieldVariable(sv.name, sv.shape));
      sv.is_primary = false;

      auto [iter, inserted] = split_vars.emplace(sv.name, std::move(sv));
      (void)inserted;
    }
  }

  return split_vars;
}

NodePtr
VariableSplittingAnalyzer::transformExpression(const NodePtr & expr,
                                                const std::map<std::string, SplitVariable> & split_vars)
{
  return transformNode(expr, split_vars);
}

std::vector<NodePtr>
VariableSplittingAnalyzer::generateConstraintEquations(
    const std::map<std::string, SplitVariable> & split_vars)
{
  std::vector<NodePtr> constraints;

  for (const auto & [name, sv] : split_vars)
    if (sv.constraint_residual)
      constraints.push_back(sv.constraint_residual->clone());

  return constraints;
}

bool
VariableSplittingAnalyzer::canHandleWithCurrentOrder(const NodePtr & expr) const
{
  return !requiresSplitting(expr);
}

void
VariableSplittingAnalyzer::analyzeNode(const NodePtr & node,
                                       std::map<std::string, unsigned int> & max_orders,
                                       std::map<std::string, std::map<unsigned int, std::set<NodeType>>> & derivative_ops,
                                       std::vector<NodeType> & derivative_stack) const
{
  if (!node)
    return;

  switch (node->type())
  {
    case NodeType::FieldVariable:
    {
      auto field = std::static_pointer_cast<FieldVariableNode>(node);
      const std::string & name = field->name();
      const unsigned int order = derivative_stack.size();
      max_orders[name] = std::max(max_orders[name], order);

      for (unsigned int depth = 0; depth < derivative_stack.size(); ++depth)
      {
        const unsigned int derivative_order = depth + 1;
        const NodeType op = derivative_stack[derivative_stack.size() - 1 - depth];
        derivative_ops[name][derivative_order].insert(op);

        if (op == NodeType::Divergence && derivative_order == 2 && depth > 0)
        {
          const std::size_t prev_index = derivative_stack.size() - depth;
          if (prev_index < derivative_stack.size() &&
              derivative_stack[prev_index] == NodeType::Gradient)
            derivative_ops[name][derivative_order].insert(NodeType::Laplacian);
        }
      }
      break;
    }

    case NodeType::Gradient:
    case NodeType::Divergence:
    case NodeType::Curl:
    {
      derivative_stack.push_back(node->type());
      auto unary = std::static_pointer_cast<UnaryOpNode>(node);
      analyzeNode(unary->operand(), max_orders, derivative_ops, derivative_stack);
      derivative_stack.pop_back();
      break;
    }

    case NodeType::Laplacian:
    {
      auto unary = std::static_pointer_cast<UnaryOpNode>(node);
      derivative_stack.push_back(NodeType::Divergence);
      derivative_stack.push_back(NodeType::Gradient);
      analyzeNode(unary->operand(), max_orders, derivative_ops, derivative_stack);
      derivative_stack.pop_back();
      derivative_stack.pop_back();
      break;
    }

    default:
    {
      if (auto unary = std::dynamic_pointer_cast<UnaryOpNode>(node))
      {
        analyzeNode(unary->operand(), max_orders, derivative_ops, derivative_stack);
      }
      else if (auto bin = std::dynamic_pointer_cast<BinaryOpNode>(node))
      {
        analyzeNode(bin->left(), max_orders, derivative_ops, derivative_stack);
        analyzeNode(bin->right(), max_orders, derivative_ops, derivative_stack);
      }
      else if (auto func = std::dynamic_pointer_cast<FunctionNode>(node))
      {
        for (const auto & arg : func->args())
          analyzeNode(arg, max_orders, derivative_ops, derivative_stack);
      }
      else if (auto vec = std::dynamic_pointer_cast<VectorAssemblyNode>(node))
      {
        for (const auto & entry : vec->components())
          analyzeNode(entry, max_orders, derivative_ops, derivative_stack);
      }
      else
      {
        for (const auto & child : node->children())
          analyzeNode(child, max_orders, derivative_ops, derivative_stack);
      }
      break;
    }
  }
}

NodePtr
VariableSplittingAnalyzer::transformNode(const NodePtr & node,
                                         const std::map<std::string, SplitVariable> & split_vars) const
{
  if (!node)
    return nullptr;

  for (const auto & [name, sv] : split_vars)
    if (sv.definition && node->equals(*sv.definition))
      return fieldVariable(name, sv.shape);

  switch (node->type())
  {
    case NodeType::Constant:
    case NodeType::Variable:
    case NodeType::FieldVariable:
      return node->clone();

    case NodeType::VectorComponent:
    {
      auto component = std::static_pointer_cast<ComponentNode>(node);
      auto transformed_expr = transformNode(component->expr(), split_vars);
      return std::make_shared<ComponentNode>(transformed_expr, component->component(), node->shape());
    }

    case NodeType::TensorComponent:
    {
      auto component = std::static_pointer_cast<ComponentNode>(node);
      auto transformed_expr = transformNode(component->expr(), split_vars);
      return std::make_shared<ComponentNode>(transformed_expr, component->i(), component->j(), node->shape());
    }

    case NodeType::Gradient:
    {
      auto unary = std::static_pointer_cast<UnaryOpNode>(node);
      auto transformed_operand = transformNode(unary->operand(), split_vars);
      auto info = getDerivativeInfo(transformed_operand, split_vars);

      if (info.pure)
      {
        unsigned int result_order = info.order + 1;
        std::string split_name = generateSplitVariableName(info.variable, result_order);
        auto it = split_vars.find(split_name);
        if (result_order > _fe_order && it != split_vars.end())
          return fieldVariable(it->second.name, it->second.shape);
      }

      return grad(transformed_operand, _dim);
    }

    case NodeType::Divergence:
    {
      auto unary = std::static_pointer_cast<UnaryOpNode>(node);
      auto transformed_operand = transformNode(unary->operand(), split_vars);
      auto info = getDerivativeInfo(transformed_operand, split_vars);

      if (info.pure)
      {
        unsigned int result_order = info.order + 1;
        std::string split_name = generateSplitVariableName(info.variable, result_order);
        auto it = split_vars.find(split_name);
        if (result_order > _fe_order && it != split_vars.end())
          return fieldVariable(it->second.name, it->second.shape);
      }

      return div(transformed_operand);
    }

    case NodeType::Curl:
    {
      auto unary = std::static_pointer_cast<UnaryOpNode>(node);
      auto transformed_operand = transformNode(unary->operand(), split_vars);
      auto info = getDerivativeInfo(transformed_operand, split_vars);

      if (info.pure)
      {
        unsigned int result_order = info.order + 1;
        std::string split_name = generateSplitVariableName(info.variable, result_order);
        auto it = split_vars.find(split_name);
        if (result_order > _fe_order && it != split_vars.end())
          return fieldVariable(it->second.name, it->second.shape);
      }

      return curl(transformed_operand, _dim);
    }

    case NodeType::Laplacian:
    {
      auto unary = std::static_pointer_cast<UnaryOpNode>(node);
      auto transformed_operand = transformNode(unary->operand(), split_vars);
      auto info = getDerivativeInfo(transformed_operand, split_vars);

      if (info.pure)
      {
        unsigned int result_order = info.order + 2;
        std::string split_name = generateSplitVariableName(info.variable, result_order);
        auto it = split_vars.find(split_name);
        if (result_order > _fe_order && it != split_vars.end())
          return fieldVariable(it->second.name, it->second.shape);
      }

      return laplacian(transformed_operand);
    }

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
      auto binary = std::static_pointer_cast<BinaryOpNode>(node);
      auto left = transformNode(binary->left(), split_vars);
      auto right = transformNode(binary->right(), split_vars);

      switch (node->type())
      {
        case NodeType::Add: return add(left, right);
        case NodeType::Subtract: return subtract(left, right);
        case NodeType::Multiply: return multiply(left, right);
        case NodeType::Divide: return divide(left, right);
        case NodeType::Power: return power(left, right);
        case NodeType::Dot: return dot(left, right);
        case NodeType::Cross: return cross(left, right);
        case NodeType::Contract: return contract(left, right);
        case NodeType::Outer: return outer(left, right);
        default: break;
      }

      return node->clone();
    }

    default:
    {
      if (auto unary = std::dynamic_pointer_cast<UnaryOpNode>(node))
      {
        auto transformed_operand = transformNode(unary->operand(), split_vars);
        return std::make_shared<UnaryOpNode>(node->type(), transformed_operand, node->shape());
      }
      else if (auto func = std::dynamic_pointer_cast<FunctionNode>(node))
      {
        std::vector<NodePtr> args;
        args.reserve(func->args().size());
        for (const auto & arg : func->args())
          args.push_back(transformNode(arg, split_vars));
        return std::make_shared<FunctionNode>(func->name(), args, node->shape());
      }
      else if (auto vec = std::dynamic_pointer_cast<VectorAssemblyNode>(node))
      {
        std::vector<NodePtr> components;
        components.reserve(vec->components().size());
        for (const auto & comp : vec->components())
          components.push_back(transformNode(comp, split_vars));
        return std::make_shared<VectorAssemblyNode>(components, node->shape());
      }
      else
      {
        std::vector<NodePtr> children;
        for (const auto & child : node->children())
          children.push_back(transformNode(child, split_vars));
        if (children.empty())
          return node->clone();
        // Fallback: rebuild using clone when possible
        return node->clone();
      }
    }
  }
}

Shape
VariableSplittingAnalyzer::findVariableShape(const NodePtr & node, const std::string & var_name) const
{
  Shape result = ScalarShape{};
  bool found = false;

  std::function<void(const NodePtr &)> recurse = [&](const NodePtr & current) {
    if (!current || found)
      return;

    if (current->type() == NodeType::FieldVariable)
    {
      auto field = std::static_pointer_cast<FieldVariableNode>(current);
      if (field->name() == var_name)
      {
        result = field->shape();
        found = true;
        return;
      }
    }

    if (auto unary = std::dynamic_pointer_cast<UnaryOpNode>(current))
    {
      recurse(unary->operand());
      return;
    }

    if (auto binary = std::dynamic_pointer_cast<BinaryOpNode>(current))
    {
      recurse(binary->left());
      recurse(binary->right());
      return;
    }

    if (auto func = std::dynamic_pointer_cast<FunctionNode>(current))
    {
      for (const auto & arg : func->args())
        recurse(arg);
      return;
    }

    if (auto vec = std::dynamic_pointer_cast<VectorAssemblyNode>(current))
    {
      for (const auto & comp : vec->components())
        recurse(comp);
      return;
    }

    for (const auto & child : current->children())
      recurse(child);
  };

  recurse(node);
  return result;
}

NodePtr
VariableSplittingAnalyzer::applyOperator(NodeType type, const NodePtr & operand) const
{
  switch (type)
  {
    case NodeType::Gradient:
      return grad(operand, _dim);
    case NodeType::Divergence:
      return div(operand);
    case NodeType::Curl:
      return curl(operand, _dim);
    case NodeType::Laplacian:
      return laplacian(operand);
    default:
      return nullptr;
  }
}

NodeType
VariableSplittingAnalyzer::chooseOperator(const std::set<NodeType> & ops) const
{
  static const std::array<NodeType, 4> precedence = {NodeType::Gradient,
                                                      NodeType::Divergence,
                                                      NodeType::Curl,
                                                      NodeType::Laplacian};

  if (ops.empty())
    return NodeType::Gradient;

  if (ops.count(NodeType::Laplacian) && ops.size() == 1)
    return NodeType::Laplacian;

  for (auto preferred : precedence)
    if (ops.count(preferred))
      return preferred;

  return *ops.begin();
}

NodePtr
VariableSplittingAnalyzer::buildDerivativeExpression(
    const std::string & original_var,
    unsigned int order,
    const Shape & original_shape,
    const std::map<std::string, std::map<unsigned int, std::set<NodeType>>> & derivative_ops) const
{
  if (!order)
    return fieldVariable(original_var, original_shape);

  auto ops_it = derivative_ops.find(original_var);
  if (ops_it == derivative_ops.end())
    mooseError("No derivative information recorded for variable ", original_var);

  NodePtr expr = fieldVariable(original_var, original_shape);

  for (unsigned int k = 1; k <= order; ++k)
  {
    auto order_it = ops_it->second.find(k);
    if (order_it == ops_it->second.end() || order_it->second.empty())
      mooseError("Missing derivative operator of order ", k, " for variable ", original_var);

    const auto & ops = order_it->second;

    if (ops.count(NodeType::Laplacian) && k == 2)
    {
      expr = laplacian(fieldVariable(original_var, original_shape));
      continue;
    }

    NodeType op = chooseOperator(ops);
    expr = applyOperator(op, expr);
  }

  return expr;
}

VariableSplittingAnalyzer::DerivativeInfo
VariableSplittingAnalyzer::getDerivativeInfo(
    const NodePtr & node,
    const std::map<std::string, SplitVariable> & split_vars) const
{
  if (!node)
    return {};

  if (node->type() == NodeType::FieldVariable)
  {
    auto field = std::static_pointer_cast<FieldVariableNode>(node);
    auto it = split_vars.find(field->name());
    if (it != split_vars.end())
    {
      DerivativeInfo info;
      info.pure = true;
      info.variable = it->second.original_variable;
      info.order = it->second.derivative_order;
      info.base_is_split = true;
      return info;
    }

    return {true, field->name(), 0, false};
  }

  if (node->type() == NodeType::Gradient || node->type() == NodeType::Divergence ||
      node->type() == NodeType::Curl)
  {
    auto unary = std::static_pointer_cast<UnaryOpNode>(node);
    auto child_info = getDerivativeInfo(unary->operand(), split_vars);
    if (!child_info.pure)
      return {};
    child_info.order += getDerivativeIncrement(node->type());
    return child_info;
  }

  if (node->type() == NodeType::Laplacian)
  {
    auto unary = std::static_pointer_cast<UnaryOpNode>(node);
    auto child_info = getDerivativeInfo(unary->operand(), split_vars);
    if (!child_info.pure)
      return {};
    child_info.order += 2;
    return child_info;
  }

  return {};
}

bool
VariableSplittingAnalyzer::isDerivativeOperator(NodeType type) const
{
  return type == NodeType::Gradient ||
         type == NodeType::Divergence ||
         type == NodeType::Laplacian ||
         type == NodeType::Curl;
}

unsigned int
VariableSplittingAnalyzer::getDerivativeIncrement(NodeType type) const
{
  switch (type)
  {
    case NodeType::Gradient:
    case NodeType::Divergence:
    case NodeType::Curl:
      return 1;
    case NodeType::Laplacian:
      return 2;
    default:
      return 0;
  }
}

std::string
VariableSplittingAnalyzer::generateSplitVariableName(const std::string & original_var,
                                                      unsigned int derivative_order) const
{
  return original_var + "_d" + std::to_string(derivative_order);
}

Shape
VariableSplittingAnalyzer::computeSplitVariableShape(const Shape & original_shape,
                                                      unsigned int derivative_order) const
{
  if (derivative_order == 0)
    return original_shape;

  if (std::holds_alternative<ScalarShape>(original_shape))
  {
    if (derivative_order == 1)
      return VectorShape{_dim};
    else if (derivative_order == 2)
      return TensorShape{_dim};
    else if (derivative_order == 3)
      return RankThreeShape{_dim};
  }
  else if (std::holds_alternative<VectorShape>(original_shape))
  {
    if (derivative_order == 1)
      return TensorShape{_dim};
    else if (derivative_order == 2)
      return RankThreeShape{_dim};
  }

  mooseError("Cannot compute shape for derivative order ", derivative_order);
}

std::vector<SplitVariableKernelGenerator::KernelInfo>
SplitVariableKernelGenerator::generateKernels(const std::map<std::string, SplitVariable> & split_vars)
{
  std::vector<KernelInfo> kernels;

  for (const auto & [name, sv] : split_vars)
  {
    kernels.push_back(generateConstraintKernel(sv));
  }

  return kernels;
}

SplitVariableKernelGenerator::KernelInfo
SplitVariableKernelGenerator::generateConstraintKernel(const SplitVariable & split_var)
{
  KernelInfo info;
  info.kernel_name = split_var.name + "_constraint";
  info.variable_name = split_var.name;
  info.kernel_type = "SplitVariableConstraint";
  info.weak_form = split_var.constraint_residual;
  info.coupled_variables = {split_var.original_variable};
  info.is_auxiliary = true;

  return info;
}

NodePtr
SplitVariableKernelGenerator::createGradientConstraint(const std::string & var_name,
                                                        const std::string & split_var_name)
{
  auto var = fieldVariable(var_name);
  auto grad_var = grad(var, 3);
  auto split_var = fieldVariable(split_var_name, VectorShape{3});
  return subtract(split_var, grad_var);
}

NodePtr
SplitVariableKernelGenerator::createHessianConstraint(const std::string & var_name,
                                                       const std::string & split_var_name)
{
  auto grad_var = fieldVariable(var_name + "_grad", VectorShape{3});
  auto grad_grad_var = grad(grad_var, 3);
  auto split_var = fieldVariable(split_var_name, TensorShape{3});
  return subtract(split_var, grad_grad_var);
}

MixedFormulationGenerator::MixedSystem
MixedFormulationGenerator::generateMixedFormulation(const NodePtr & energy_density,
                                                     const std::vector<std::string> & variables,
                                                     unsigned int fe_order)
{
  MixedSystem system;
  system.primary_variables = variables;

  VariableSplittingAnalyzer analyzer(fe_order);
  auto split_vars = analyzer.generateSplitVariables(energy_density);

  for (const auto & [name, sv] : split_vars)
  {
    system.auxiliary_variables.push_back(sv.name);
    system.weak_forms[sv.name] = sv.constraint_residual;
  }

  WeakFormGenerator gen;
  for (const auto & var : variables)
  {
    system.weak_forms[var] = gen.generateWeakForm(energy_density, var);
  }

  system.is_saddle_point = !system.auxiliary_variables.empty();

  return system;
}

bool
MixedFormulationGenerator::requiresMixedFormulation(const NodePtr & energy_density,
                                                     unsigned int fe_order) const
{
  VariableSplittingAnalyzer analyzer(fe_order);
  return analyzer.requiresSplitting(energy_density);
}

HigherOrderSplittingStrategy::SplitPlan
HigherOrderSplittingStrategy::computeOptimalSplitting(
    const NodePtr & energy_density,
    const std::string & primary_var,
    unsigned int max_derivative_order,
    unsigned int fe_order,
    const std::map<std::string, SplitVariable> & split_vars)
{
  const std::map<std::string, SplitVariable> * splits = &split_vars;
  std::map<std::string, SplitVariable> generated;

  auto computeRelevant = [&](const std::map<std::string, SplitVariable> & map) {
    return collectSplitVariables(primary_var, fe_order, map);
  };

  auto relevant_splits = computeRelevant(*splits);

  if ((splits->empty() || (relevant_splits.empty() && max_derivative_order > fe_order)) &&
      energy_density)
  {
    VariableSplittingAnalyzer analyzer(fe_order);
    generated = analyzer.generateSplitVariables(energy_density);
    splits = &generated;
    relevant_splits = computeRelevant(*splits);
  }

  if (relevant_splits.empty() || max_derivative_order <= fe_order)
  {
    SplitPlan no_split;
    no_split.primary_variable = primary_var;
    no_split.strategy = Strategy::DIRECT;
    no_split.total_dofs = 1u;
    no_split.bandwidth = 1u;
    no_split.estimated_condition_number = estimateConditionNumber(no_split);
    return no_split;
  }

  std::vector<SplitPlan> candidates;
  candidates.push_back(createRecursiveSplitting(primary_var, *splits, fe_order));
  candidates.push_back(createDirectSplitting(primary_var, *splits, fe_order));

  if (relevant_splits.size() > 1)
    candidates.push_back(createMixedSplitting(primary_var, *splits, fe_order));

  SplitPlan best_plan;
  Real best_cost = std::numeric_limits<Real>::max();

  for (auto & candidate : candidates)
  {
    optimizeBandwidth(candidate);
    Real cost = estimateComputationalCost(candidate);
    if (cost < best_cost)
    {
      best_cost = cost;
      best_plan = candidate;
    }
  }

  if (best_plan.bandwidth == 0u)
  {
    best_plan.bandwidth = 1u;
    best_plan.total_dofs = computePlanDofs(best_plan);
    best_plan.estimated_condition_number = estimateConditionNumber(best_plan);
  }

  return best_plan;
}

HigherOrderSplittingStrategy::SplitPlan
HigherOrderSplittingStrategy::createRecursiveSplitting(const std::string & var_name,
                                                        const std::map<std::string, SplitVariable> & split_vars,
                                                        unsigned int fe_order)
{
  SplitPlan plan;
  plan.strategy = Strategy::RECURSIVE;
  plan.primary_variable = var_name;
  plan.total_dofs = 1u;
  plan.bandwidth = 1u;
  plan.estimated_condition_number = 0.0;

  auto relevant = collectSplitVariables(var_name, fe_order, split_vars);
  plan.variables = relevant;

  std::string previous = var_name;
  for (const auto & sv : plan.variables)
  {
    plan.dependencies.emplace_back(sv.name, previous);
    previous = sv.name;
  }

  return plan;
}

HigherOrderSplittingStrategy::SplitPlan
HigherOrderSplittingStrategy::createDirectSplitting(const std::string & var_name,
                                                     const std::map<std::string, SplitVariable> & split_vars,
                                                     unsigned int fe_order)
{
  SplitPlan plan;
  plan.strategy = Strategy::DIRECT;
  plan.primary_variable = var_name;
  plan.total_dofs = 1u;
  plan.bandwidth = 1u;
  plan.estimated_condition_number = 0.0;

  auto relevant = collectSplitVariables(var_name, fe_order, split_vars);
  plan.variables = relevant;

  for (const auto & sv : plan.variables)
    plan.dependencies.emplace_back(sv.name, var_name);

  return plan;
}

HigherOrderSplittingStrategy::SplitPlan
HigherOrderSplittingStrategy::createMixedSplitting(const std::string & var_name,
                                                   const std::map<std::string, SplitVariable> & split_vars,
                                                   unsigned int fe_order,
                                                   unsigned int threshold)
{
  SplitPlan plan;
  plan.strategy = Strategy::MIXED;
  plan.primary_variable = var_name;
  plan.total_dofs = 1u;
  plan.bandwidth = 1u;
  plan.estimated_condition_number = 0.0;

  auto relevant = collectSplitVariables(var_name, fe_order, split_vars);
  plan.variables = relevant;

  if (threshold == 0)
    threshold = 1;

  unsigned int recursive_count = std::min<unsigned int>(threshold, plan.variables.size());
  std::string previous = var_name;

  for (unsigned int i = 0; i < plan.variables.size(); ++i)
  {
    const auto & sv = plan.variables[i];
    if (i < recursive_count)
    {
      plan.dependencies.emplace_back(sv.name, previous);
      previous = sv.name;
    }
    else
      plan.dependencies.emplace_back(sv.name, var_name);
  }

  return plan;
}

Real
HigherOrderSplittingStrategy::estimateComputationalCost(const SplitPlan & plan)
{
  Real cost = 0.0;

  cost += plan.total_dofs * plan.total_dofs;

  cost += plan.bandwidth * plan.total_dofs;

  cost += estimateConditionNumber(plan) * 0.1;

  return cost;
}

void
HigherOrderSplittingStrategy::optimizeBandwidth(SplitPlan & plan)
{
  plan.total_dofs = computePlanDofs(plan);

  if (plan.variables.empty())
  {
    plan.bandwidth = 1u;
    plan.estimated_condition_number = estimateConditionNumber(plan);
    return;
  }

  auto connectivity = buildConnectivityMatrix(plan);
  auto ordering = performRCMOrdering(connectivity);
  plan.bandwidth = computeBandwidth(plan, ordering);

  if (plan.bandwidth == 0u)
    plan.bandwidth = 1u;

  plan.estimated_condition_number = estimateConditionNumber(plan);
}

unsigned int
HigherOrderSplittingStrategy::computeBandwidth(const SplitPlan & plan,
                                              const std::vector<unsigned int> & ordering)
{
  const unsigned int n = plan.variables.size();
  if (n == 0 || ordering.size() != n)
    return 0;

  std::vector<unsigned int> position(n);
  for (unsigned int idx = 0; idx < n; ++idx)
  {
    unsigned int var_index = ordering[idx];
    if (var_index >= n)
      mooseError("Invalid ordering index in computeBandwidth");
    position[var_index] = idx;
  }

  auto connectivity = buildConnectivityMatrix(plan);

  unsigned int bandwidth = 1;
  for (unsigned int i = 0; i < n; ++i)
    for (unsigned int j = i + 1; j < n; ++j)
      if (connectivity[i][j])
      {
        unsigned int distance = position[i] > position[j] ? position[i] - position[j]
                                                          : position[j] - position[i];
        bandwidth = std::max(bandwidth, distance + 1);
      }

  return bandwidth;
}

std::vector<std::vector<bool>>
HigherOrderSplittingStrategy::buildConnectivityMatrix(const SplitPlan & plan)
{
  const unsigned int n = plan.variables.size();
  std::vector<std::vector<bool>> connectivity(n, std::vector<bool>(n, false));

  if (n == 0)
    return connectivity;

  std::map<std::string, unsigned int> index_map;
  for (unsigned int i = 0; i < n; ++i)
  {
    index_map[plan.variables[i].name] = i;
    connectivity[i][i] = true;
  }

  auto link = [&](const std::string & a, const std::string & b) {
    auto it_a = index_map.find(a);
    auto it_b = index_map.find(b);
    if (it_a != index_map.end() && it_b != index_map.end())
    {
      connectivity[it_a->second][it_b->second] = true;
      connectivity[it_b->second][it_a->second] = true;
    }
  };

  for (const auto & dep : plan.dependencies)
    link(dep.first, dep.second);

  // Ensure auxiliary variables are connected to their original variable
  for (const auto & var : plan.variables)
    link(var.name, var.original_variable);

  return connectivity;
}

std::vector<unsigned int>
HigherOrderSplittingStrategy::performRCMOrdering(const std::vector<std::vector<bool>> & connectivity)
{
  unsigned int n = connectivity.size();
  if (n == 0)
    return {};
  std::vector<unsigned int> ordering(n);
  std::vector<bool> visited(n, false);
  std::vector<unsigned int> degree(n);

  for (unsigned int i = 0; i < n; ++i)
  {
    degree[i] = std::count(connectivity[i].begin(), connectivity[i].end(), true);
  }

  std::vector<unsigned int> level_order;
  level_order.reserve(n);

  auto pick_start = [&]() -> unsigned int {
    unsigned int best = n;
    unsigned int best_degree = std::numeric_limits<unsigned int>::max();
    for (unsigned int i = 0; i < n; ++i)
      if (!visited[i] && degree[i] < best_degree)
      {
        best = i;
        best_degree = degree[i];
      }
    return best;
  };

  while (true)
  {
    unsigned int start = pick_start();
    if (start == n)
      break;

    std::queue<unsigned int> q;
    q.push(start);
    visited[start] = true;

    while (!q.empty())
    {
      unsigned int current = q.front();
      q.pop();
      level_order.push_back(current);

      std::vector<std::pair<unsigned int, unsigned int>> neighbors;
      for (unsigned int i = 0; i < n; ++i)
        if (connectivity[current][i] && !visited[i])
        {
          neighbors.emplace_back(degree[i], i);
          visited[i] = true;
        }

      std::sort(neighbors.begin(), neighbors.end());

      for (const auto & [deg, idx] : neighbors)
        q.push(idx);
    }
  }

  std::reverse(level_order.begin(), level_order.end());
  for (unsigned int i = 0; i < level_order.size(); ++i)
    ordering[i] = level_order[i];

  return ordering;
}

Real
HigherOrderSplittingStrategy::estimateConditionNumber(const SplitPlan & plan)
{
  return static_cast<Real>(plan.total_dofs * plan.total_dofs);
}

}
}
