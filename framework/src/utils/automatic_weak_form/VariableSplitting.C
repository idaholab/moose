#include "automatic_weak_form/VariableSplitting.h"
#include "automatic_weak_form/WeakFormGenerator.h"
#include "MooseError.h"
#include <algorithm>
#include <queue>

namespace moose
{
namespace automatic_weak_form
{

std::vector<VariableSplittingAnalyzer::SplitRequirement>
VariableSplittingAnalyzer::analyzeExpression(const NodePtr & expr)
{
  std::map<std::string, unsigned int> max_orders;
  analyzeNode(expr, max_orders, 0);
  
  std::vector<SplitRequirement> requirements;
  
  for (const auto & [var_name, max_order] : max_orders)
  {
    SplitRequirement req;
    req.variable_name = var_name;
    req.max_derivative_order = max_order;
    req.available_order = _use_hessians ? 2 : 1;
    req.requires_splitting = (max_order > req.available_order);
    
    if (req.requires_splitting)
    {
      for (unsigned int order = req.available_order + 1; order <= max_order; ++order)
        req.split_orders.push_back(order);
    }
    
    requirements.push_back(req);
  }
  
  return requirements;
}

bool
VariableSplittingAnalyzer::requiresSplitting(const NodePtr & expr) const
{
  auto requirements = const_cast<VariableSplittingAnalyzer*>(this)->analyzeExpression(expr);
  
  for (const auto & req : requirements)
    if (req.requires_splitting)
      return true;
  
  return false;
}

unsigned int
VariableSplittingAnalyzer::getMaxDerivativeOrder(const NodePtr & expr, const std::string & var_name) const
{
  std::map<std::string, unsigned int> max_orders;
  const_cast<VariableSplittingAnalyzer*>(this)->analyzeNode(expr, max_orders, 0);
  
  if (max_orders.count(var_name))
    return max_orders[var_name];
  
  return 0;
}

std::map<std::string, SplitVariable>
VariableSplittingAnalyzer::generateSplitVariables(const NodePtr & expr)
{
  auto requirements = analyzeExpression(expr);
  std::map<std::string, SplitVariable> split_vars;
  
  for (const auto & req : requirements)
  {
    if (req.requires_splitting)
    {
      for (unsigned int order : req.split_orders)
      {
        SplitVariable sv;
        sv.name = generateSplitVariableName(req.variable_name, order);
        sv.original_variable = req.variable_name;
        sv.derivative_order = order;
        sv.definition = createSplitVariableDefinition(req.variable_name, order);
        sv.is_primary = false;
        
        Shape orig_shape = ScalarShape{};
        sv.shape = computeSplitVariableShape(orig_shape, order);
        
        sv.constraint_residual = subtract(
          variable(sv.name, sv.shape),
          sv.definition
        );
        
        split_vars[sv.name] = sv;
      }
    }
  }
  
  return split_vars;
}

NodePtr
VariableSplittingAnalyzer::transformExpression(const NodePtr & expr,
                                                const std::map<std::string, SplitVariable> & split_vars)
{
  return transformNode(expr, split_vars, 0);
}

std::vector<NodePtr>
VariableSplittingAnalyzer::generateConstraintEquations(
    const std::map<std::string, SplitVariable> & split_vars)
{
  std::vector<NodePtr> constraints;
  
  for (const auto & [name, sv] : split_vars)
  {
    constraints.push_back(sv.constraint_residual);
  }
  
  return constraints;
}

NodePtr
VariableSplittingAnalyzer::createSplitVariableDefinition(const std::string & original_var,
                                                          unsigned int derivative_order)
{
  NodePtr var = fieldVariable(original_var);
  
  for (unsigned int i = 0; i < derivative_order; ++i)
    var = grad(var, _dim);
  
  return var;
}

bool
VariableSplittingAnalyzer::canHandleWithCurrentOrder(const NodePtr & expr) const
{
  return !requiresSplitting(expr);
}

void
VariableSplittingAnalyzer::analyzeNode(const NodePtr & node,
                                        std::map<std::string, unsigned int> & max_orders,
                                        unsigned int current_derivative_level)
{
  if (!node)
    return;
  
  switch (node->type())
  {
    case NodeType::FieldVariable:
    {
      auto field = std::static_pointer_cast<FieldVariableNode>(node);
      std::string name = field->name();
      max_orders[name] = std::max(max_orders[name], current_derivative_level);
      break;
    }
    
    case NodeType::Gradient:
    case NodeType::Divergence:
    case NodeType::Curl:
    {
      auto unary = std::static_pointer_cast<UnaryOpNode>(node);
      unsigned int increment = getDerivativeIncrement(node->type());
      analyzeNode(unary->operand(), max_orders, current_derivative_level + increment);
      break;
    }
    
    case NodeType::Laplacian:
    {
      auto unary = std::static_pointer_cast<UnaryOpNode>(node);
      analyzeNode(unary->operand(), max_orders, current_derivative_level + 2);
      break;
    }
    
    default:
    {
      for (const auto & child : node->children())
        analyzeNode(child, max_orders, current_derivative_level);
      break;
    }
  }
}

NodePtr
VariableSplittingAnalyzer::transformNode(const NodePtr & node,
                                          const std::map<std::string, SplitVariable> & split_vars,
                                          unsigned int current_derivative_level)
{
  if (!node)
    return node;
  
  if (current_derivative_level > _fe_order)
  {
    if (node->type() == NodeType::Gradient)
    {
      auto unary = std::static_pointer_cast<UnaryOpNode>(node);
      auto operand = unary->operand();
      
      if (operand->type() == NodeType::FieldVariable)
      {
        auto field = std::static_pointer_cast<FieldVariableNode>(operand);
        std::string split_name = generateSplitVariableName(field->name(), current_derivative_level);
        
        if (split_vars.count(split_name))
        {
          const auto & sv = split_vars.at(split_name);
          return variable(sv.name, sv.shape);
        }
      }
    }
  }
  
  switch (node->type())
  {
    case NodeType::Gradient:
    {
      auto unary = std::static_pointer_cast<UnaryOpNode>(node);
      auto transformed_operand = transformNode(unary->operand(), split_vars, 
                                                current_derivative_level + 1);
      return grad(transformed_operand, _dim);
    }
    
    case NodeType::Laplacian:
    {
      auto unary = std::static_pointer_cast<UnaryOpNode>(node);
      auto transformed_operand = transformNode(unary->operand(), split_vars,
                                                current_derivative_level + 2);
      return laplacian(transformed_operand);
    }
    
    case NodeType::Add:
    case NodeType::Subtract:
    case NodeType::Multiply:
    case NodeType::Divide:
    {
      auto binary = std::static_pointer_cast<BinaryOpNode>(node);
      auto left = transformNode(binary->left(), split_vars, current_derivative_level);
      auto right = transformNode(binary->right(), split_vars, current_derivative_level);
      
      switch (node->type())
      {
        case NodeType::Add: return add(left, right);
        case NodeType::Subtract: return subtract(left, right);
        case NodeType::Multiply: return multiply(left, right);
        case NodeType::Divide: return divide(left, right);
        default: break;
      }
    }
    
    default:
      return node->clone();
  }
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
                                                      unsigned int derivative_order)
{
  if (derivative_order == 1)
    return original_var + "_grad";
  else if (derivative_order == 2)
    return original_var + "_hess";
  else
    return original_var + "_d" + std::to_string(derivative_order);
}

Shape
VariableSplittingAnalyzer::computeSplitVariableShape(const Shape & original_shape,
                                                      unsigned int derivative_order)
{
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
HigherOrderSplittingStrategy::computeOptimalSplitting(const NodePtr & energy_density,
                                                       const std::string & primary_var,
                                                       unsigned int max_derivative_order,
                                                       unsigned int fe_order)
{
  std::vector<SplitPlan> candidates;
  
  candidates.push_back(createRecursiveSplitting(primary_var, max_derivative_order));
  candidates.push_back(createDirectSplitting(primary_var, max_derivative_order));
  
  if (max_derivative_order > 2)
    candidates.push_back(createMixedSplitting(primary_var, max_derivative_order));
  
  SplitPlan best_plan = candidates[0];
  Real best_cost = estimateComputationalCost(best_plan);
  
  for (size_t i = 1; i < candidates.size(); ++i)
  {
    Real cost = estimateComputationalCost(candidates[i]);
    if (cost < best_cost)
    {
      best_cost = cost;
      best_plan = candidates[i];
    }
  }
  
  best_plan.strategy = Strategy::OPTIMAL;
  optimizeBandwidth(best_plan);
  
  return best_plan;
}

HigherOrderSplittingStrategy::SplitPlan
HigherOrderSplittingStrategy::createRecursiveSplitting(const std::string & var_name,
                                                        unsigned int max_order)
{
  SplitPlan plan;
  plan.strategy = Strategy::RECURSIVE;
  
  std::string current_var = var_name;
  
  for (unsigned int order = 1; order <= max_order; ++order)
  {
    SplitVariable sv;
    sv.name = var_name + "_d" + std::to_string(order);
    sv.original_variable = current_var;
    sv.derivative_order = 1;
    sv.is_primary = (order == 1);
    
    plan.variables.push_back(sv);
    
    if (order > 1)
      plan.dependencies.push_back({sv.name, current_var});
    
    current_var = sv.name;
  }
  
  plan.total_dofs = plan.variables.size() + 1;
  
  return plan;
}

HigherOrderSplittingStrategy::SplitPlan
HigherOrderSplittingStrategy::createDirectSplitting(const std::string & var_name,
                                                     unsigned int max_order)
{
  SplitPlan plan;
  plan.strategy = Strategy::DIRECT;
  
  for (unsigned int order = 1; order <= max_order; ++order)
  {
    SplitVariable sv;
    sv.name = var_name + "_d" + std::to_string(order);
    sv.original_variable = var_name;
    sv.derivative_order = order;
    sv.is_primary = false;
    
    plan.variables.push_back(sv);
    plan.dependencies.push_back({sv.name, var_name});
  }
  
  plan.total_dofs = plan.variables.size() + 1;
  
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
  auto connectivity = buildConnectivityMatrix(plan);
  auto ordering = performRCMOrdering(connectivity);
  plan.bandwidth = computeBandwidth(plan, ordering);
}

std::vector<unsigned int>
HigherOrderSplittingStrategy::performRCMOrdering(const std::vector<std::vector<bool>> & connectivity)
{
  unsigned int n = connectivity.size();
  std::vector<unsigned int> ordering(n);
  std::vector<bool> visited(n, false);
  std::vector<unsigned int> degree(n);
  
  for (unsigned int i = 0; i < n; ++i)
  {
    degree[i] = std::count(connectivity[i].begin(), connectivity[i].end(), true);
  }
  
  auto min_it = std::min_element(degree.begin(), degree.end());
  unsigned int start = std::distance(degree.begin(), min_it);
  
  std::queue<unsigned int> q;
  q.push(start);
  visited[start] = true;
  
  std::vector<unsigned int> level_order;
  
  while (!q.empty())
  {
    unsigned int current = q.front();
    q.pop();
    level_order.push_back(current);
    
    std::vector<std::pair<unsigned int, unsigned int>> neighbors;
    for (unsigned int i = 0; i < n; ++i)
    {
      if (connectivity[current][i] && !visited[i])
      {
        neighbors.push_back({degree[i], i});
        visited[i] = true;
      }
    }
    
    std::sort(neighbors.begin(), neighbors.end());
    
    for (const auto & [d, neighbor] : neighbors)
      q.push(neighbor);
  }
  
  for (unsigned int i = 0; i < n; ++i)
    ordering[i] = level_order[n - 1 - i];
  
  return ordering;
}

Real
HigherOrderSplittingStrategy::estimateConditionNumber(const SplitPlan & plan)
{
  return static_cast<Real>(plan.total_dofs * plan.total_dofs);
}

}
}