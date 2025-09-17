#pragma once

#include "MooseAST.h"
#include "MooseValueTypes.h"
#include <map>
#include <set>
#include <vector>
#include <memory>

namespace moose
{
namespace automatic_weak_form
{

struct SplitVariable
{
  std::string name;
  std::string original_variable;
  /// Derivative order of the original variable represented by this split variable
  unsigned int derivative_order = 0;
  Shape shape;

  NodePtr definition;

  NodePtr constraint_residual;

  bool is_primary;

  std::string toString() const
  {
    return name + " (order " + std::to_string(derivative_order) + ")";
  }
};

class VariableSplittingAnalyzer
{
public:
  VariableSplittingAnalyzer(unsigned int fe_order = 1, unsigned int dim = 3)
    : _fe_order(fe_order), _dim(dim) {}
  
  struct SplitRequirement
  {
    std::string variable_name;
    unsigned int max_derivative_order;
    unsigned int available_order;
    bool requires_splitting;
    std::vector<unsigned int> split_orders;
  };
  
  std::vector<SplitRequirement> analyzeExpression(const NodePtr & expr);

  bool requiresSplitting(const NodePtr & expr) const;

  unsigned int getMaxDerivativeOrder(const NodePtr & expr, const std::string & var_name) const;

  std::map<std::string, SplitVariable> generateSplitVariables(const NodePtr & expr);
  
  NodePtr transformExpression(const NodePtr & expr,
                               const std::map<std::string, SplitVariable> & split_vars);
  
  std::vector<NodePtr> generateConstraintEquations(
      const std::map<std::string, SplitVariable> & split_vars);
  
  bool canHandleWithCurrentOrder(const NodePtr & expr) const;
  
  void setMaxFEOrder(unsigned int order) { _fe_order = order; }
  
  void setUseHessians(bool use) { _use_hessians = use; }
  
private:
  unsigned int _fe_order;
  unsigned int _dim;
  bool _use_hessians = false;
  
  void analyzeNode(const NodePtr & node,
                   std::map<std::string, unsigned int> & max_orders,
                   std::map<std::string, std::map<unsigned int, std::set<NodeType>>> & derivative_ops,
                   std::vector<NodeType> & derivative_stack) const;

  NodePtr transformNode(const NodePtr & node,
                        const std::map<std::string, SplitVariable> & split_vars) const;

  bool isDerivativeOperator(NodeType type) const;

  unsigned int getDerivativeIncrement(NodeType type) const;

  std::string generateSplitVariableName(const std::string & original_var,
                                         unsigned int derivative_order) const;

  Shape computeSplitVariableShape(const Shape & original_shape,
                                   unsigned int derivative_order) const;

  Shape findVariableShape(const NodePtr & node, const std::string & var_name) const;

  NodePtr applyOperator(NodeType type, const NodePtr & operand) const;

  NodeType chooseOperator(const std::set<NodeType> & ops) const;

  NodePtr buildDerivativeExpression(
      const std::string & original_var,
      unsigned int order,
      const Shape & original_shape,
      const std::map<std::string, std::map<unsigned int, std::set<NodeType>>> & derivative_ops) const;

  struct DerivativeInfo
  {
    bool pure = false;
    std::string variable;
    unsigned int order = 0;
    bool base_is_split = false;
  };

  DerivativeInfo getDerivativeInfo(const NodePtr & node,
                                   const std::map<std::string, SplitVariable> & split_vars) const;
};

class SplitVariableKernelGenerator
{
public:
  struct KernelInfo
  {
    std::string kernel_name;
    std::string variable_name;
    std::string kernel_type;
    NodePtr weak_form;
    std::vector<std::string> coupled_variables;
    bool is_auxiliary;
  };
  
  std::vector<KernelInfo> generateKernels(
      const std::map<std::string, SplitVariable> & split_vars);
  
  KernelInfo generateConstraintKernel(const SplitVariable & split_var);
  
  KernelInfo generateCoupledKernel(const SplitVariable & split_var,
                                    const NodePtr & original_weak_form);
  
  NodePtr generateAuxiliaryKernelExpression(const SplitVariable & split_var);
  
  std::vector<std::string> identifyCoupledVariables(const NodePtr & expr);
  
private:
  NodePtr createGradientConstraint(const std::string & var_name,
                                    const std::string & split_var_name);
  
  NodePtr createHessianConstraint(const std::string & var_name,
                                   const std::string & split_var_name);
  
  NodePtr createLaplacianConstraint(const std::string & var_name,
                                     const std::string & split_var_name);
};

class MixedFormulationGenerator
{
public:
  struct MixedSystem
  {
    std::vector<std::string> primary_variables;
    std::vector<std::string> auxiliary_variables;
    std::map<std::string, NodePtr> weak_forms;
    std::map<std::string, std::map<std::string, NodePtr>> jacobian_blocks;
    bool is_saddle_point;
  };
  
  MixedSystem generateMixedFormulation(const NodePtr & energy_density,
                                        const std::vector<std::string> & variables,
                                        unsigned int fe_order = 1);
  
  bool requiresMixedFormulation(const NodePtr & energy_density,
                                 unsigned int fe_order) const;
  
  NodePtr generateSaddlePointForm(const MixedSystem & system);
  
  std::map<std::string, NodePtr> generateBlockPreconditioner(const MixedSystem & system);
  
  struct StabilityAnalysis
  {
    bool is_inf_sup_stable;
    Real inf_sup_constant;
    bool needs_stabilization;
    std::string recommended_stabilization;
  };
  
  StabilityAnalysis analyzeStability(const MixedSystem & system);
  
  NodePtr addStabilization(const MixedSystem & system,
                           const std::string & method = "PSPG");
  
private:
  NodePtr computeSchurComplement(const MixedSystem & system);
  
  bool checkInfSupCondition(const NodePtr & B_operator);
  
  NodePtr generatePSPGStabilization(const NodePtr & momentum_eq,
                                     const NodePtr & continuity_eq,
                                     Real tau);
  
  NodePtr generateGLSStabilization(const MixedSystem & system, Real tau);
  
  Real computeStabilizationParameter(const NodePtr & weak_form,
                                      Real element_size,
                                      Real velocity_scale);
};

class HigherOrderSplittingStrategy
{
public:
  enum class Strategy
  {
    RECURSIVE,
    DIRECT,
    MIXED,
    OPTIMAL
  };
  
  struct SplitPlan
  {
    std::vector<SplitVariable> variables;
    std::vector<std::pair<std::string, std::string>> dependencies;
    Strategy strategy;
    unsigned int total_dofs;
    unsigned int bandwidth;
    Real estimated_condition_number;
  };
  
  SplitPlan computeOptimalSplitting(const NodePtr & energy_density,
                                     const std::string & primary_var,
                                     unsigned int max_derivative_order,
                                     unsigned int fe_order);
  
  SplitPlan createRecursiveSplitting(const std::string & var_name,
                                      unsigned int max_order);
  
  SplitPlan createDirectSplitting(const std::string & var_name,
                                   unsigned int max_order);
  
  SplitPlan createMixedSplitting(const std::string & var_name,
                                  unsigned int max_order,
                                  unsigned int threshold = 2);
  
  Real estimateComputationalCost(const SplitPlan & plan);
  
  Real estimateMemoryUsage(const SplitPlan & plan, unsigned int num_elements);
  
  void optimizeBandwidth(SplitPlan & plan);
  
  std::vector<unsigned int> computeOptimalOrdering(const SplitPlan & plan);
  
private:
  unsigned int computeBandwidth(const SplitPlan & plan,
                                 const std::vector<unsigned int> & ordering);
  
  std::vector<std::vector<bool>> buildConnectivityMatrix(const SplitPlan & plan);
  
  std::vector<unsigned int> performRCMOrdering(
      const std::vector<std::vector<bool>> & connectivity);
  
  Real estimateConditionNumber(const SplitPlan & plan);
};

class AdaptiveSplittingController
{
public:
  AdaptiveSplittingController() = default;
  
  void initialize(const NodePtr & energy_density,
                  const std::vector<std::string> & variables);
  
  bool shouldRefineVariable(const std::string & var_name,
                             const std::vector<Real> & error_indicators);
  
  bool shouldCoarsenVariable(const std::string & var_name,
                              const std::vector<Real> & error_indicators);
  
  void updateSplittingStrategy(const std::string & var_name,
                                unsigned int new_order);
  
  struct AdaptivityMetrics
  {
    Real global_error;
    std::map<std::string, Real> variable_errors;
    std::map<std::string, Real> splitting_efficiency;
    Real computational_cost;
  };
  
  AdaptivityMetrics computeMetrics(const std::map<std::string, std::vector<Real>> & solutions);
  
  void suggestRefinement(const AdaptivityMetrics & metrics);
  
private:
  std::map<std::string, unsigned int> _current_orders;
  std::map<std::string, std::vector<Real>> _error_history;
  std::map<std::string, HigherOrderSplittingStrategy::SplitPlan> _current_plans;
  
  Real _refinement_threshold = 1e-3;
  Real _coarsening_threshold = 1e-5;
  unsigned int _max_splitting_order = 4;
  
  bool checkConvergenceRate(const std::vector<Real> & error_history);
  
  Real predictErrorReduction(const std::string & var_name,
                              unsigned int new_order);
};

}
}
