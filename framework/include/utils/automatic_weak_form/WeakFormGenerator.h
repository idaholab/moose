#pragma once

#include "MooseAST.h"
#include "MooseValueTypes.h"
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>

namespace moose
{
namespace automatic_weak_form
{

struct Differential
{
  std::map<unsigned int, NodePtr> coefficients;
  
  NodePtr getCoefficient(unsigned int order) const
  {
    auto it = coefficients.find(order);
    if (it != coefficients.end())
      return it->second;
    return nullptr;
  }
  
  unsigned int maxOrder() const
  {
    unsigned int max_order = 0;
    for (const auto & [order, coeff] : coefficients)
      if (coeff)
        max_order = std::max(max_order, order);
    return max_order;
  }
  
  bool hasOrder(unsigned int order) const
  {
    return coefficients.count(order) > 0 && coefficients.at(order) != nullptr;
  }
};

class DifferentiationVisitor
{
public:
  DifferentiationVisitor(const std::string & var_name) : _var_name(var_name) {}
  
  Differential differentiate(const NodePtr & expr);
  
private:
  using Handler = Differential (DifferentiationVisitor::*)(const NodePtr &);

  std::string _var_name;
  std::unordered_map<const Node *, Differential> _cache;
  
  Differential visit(const NodePtr & node);
  static const std::unordered_map<NodeType, Handler> & handlerMap();
  Differential differentiateConstant(const NodePtr & node);
  Differential differentiateVariable(const NodePtr & node);
  Differential differentiateFieldVariable(const NodePtr & node);
  Differential differentiateUnaryOp(const NodePtr & node);
  Differential differentiateBinaryOp(const NodePtr & node);
  Differential differentiateFunction(const NodePtr & node);
  Differential differentiateVectorAssembly(const NodePtr & node);
  Differential differentiateComponent(const NodePtr & node);

  Differential handleGradient(const NodePtr & operand);
  Differential handleCurl(const NodePtr & operand);
  Differential handleDivergence(const NodePtr & operand);
  Differential handleLaplacian(const NodePtr & operand);
  Differential handleNorm(const NodePtr & operand);
  Differential handleNormalize(const NodePtr & operand);
  
  Differential handleAdd(const NodePtr & left, const NodePtr & right);
  Differential handleSubtract(const NodePtr & left, const NodePtr & right);
  Differential handleMultiply(const NodePtr & left, const NodePtr & right);
  Differential handleDivide(const NodePtr & left, const NodePtr & right);
  Differential handlePower(const NodePtr & left, const NodePtr & right);
  Differential handleDot(const NodePtr & left, const NodePtr & right);
  Differential handleCross(const NodePtr & left, const NodePtr & right);
  Differential handleContract(const NodePtr & left, const NodePtr & right);
  Differential handleOuter(const NodePtr & left, const NodePtr & right);
  
  NodePtr shiftDerivativeOrder(const NodePtr & expr, int shift);
  
  bool dependsOnVariable(const NodePtr & expr) const;
  
  NodePtr simplify(const NodePtr & expr);
  
  NodePtr applyChainRule(const NodePtr & df_du, const Differential & du_dx);
  
  Differential combineDifferentials(const Differential & d1, const Differential & d2, 
                                    std::function<NodePtr(const NodePtr &, const NodePtr &)> combiner);
};

class WeakFormGenerator
{
public:
  WeakFormGenerator(unsigned int dim = 3) : _dim(dim) {}
  
  NodePtr generateWeakForm(const NodePtr & energy_density, const std::string & var_name);
  
  NodePtr computeEulerLagrange(const Differential & diff);
  
  NodePtr computeResidualContribution(const Differential & diff, 
                                       const std::string & var_name,
                                       bool use_test_functions = true);
  
  NodePtr computeJacobianContribution(const Differential & residual_diff,
                                       const std::string & trial_var,
                                       bool use_shape_functions = true);
  
  struct WeakFormContributions
  {
    NodePtr c0_term;
    NodePtr c1_term;
    NodePtr c2_term;
    NodePtr c3_term;
    
    NodePtr total_residual;
    
    unsigned int max_order;
  };
  
  WeakFormContributions computeContributions(const NodePtr & energy_density,
                                              const std::string & var_name);
  
  bool requiresVariableSplitting(const Differential & diff, unsigned int fe_order) const;
  
  std::vector<std::string> identifyRequiredSplitVariables(const NodePtr & energy_density,
                                                           const std::string & var_name,
                                                           unsigned int fe_order);
  
  NodePtr transformForSplitVariables(const NodePtr & expr,
                                      const std::map<std::string, NodePtr> & split_vars);
  
  NodePtr applyIntegrationByParts(const NodePtr & strong_form,
                                   const std::string & var_name,
                                   unsigned int order);
  
  NodePtr extractBoundaryTerms(const Differential & diff,
                                const std::string & var_name);
  
  struct ConservationCheck
  {
    bool is_conservative;
    NodePtr conserved_quantity;
    NodePtr flux;
    std::string message;
  };
  
  ConservationCheck checkConservation(const NodePtr & weak_form,
                                       const std::string & var_name);
  
  NodePtr computeLinearization(const NodePtr & nonlinear_form,
                                const std::string & var_name,
                                const NodePtr & increment);
  
  NodePtr computeSecondVariation(const NodePtr & energy_density,
                                  const std::string & var1,
                                  const std::string & var2);
  
  bool isSymmetric(const NodePtr & second_variation);
  
  bool isElliptic(const NodePtr & second_variation);
  
  bool isCoercive(const NodePtr & bilinear_form);
  
private:
  unsigned int _dim;
  
  NodePtr applyDivergence(const NodePtr & expr, unsigned int times = 1);
  
  NodePtr multiplyByTestFunction(const NodePtr & expr, const std::string & var_name,
                                  unsigned int derivative_order = 0);
  
  NodePtr multiplyByShapeFunction(const NodePtr & expr, const std::string & var_name,
                                   unsigned int derivative_order = 0);
  
  NodePtr integrateOverDomain(const NodePtr & expr);
  
  NodePtr extractVolumeIntegral(const NodePtr & expr);
  
  NodePtr extractSurfaceIntegral(const NodePtr & expr);
  
  bool hasHigherOrderDerivatives(const NodePtr & expr, unsigned int order) const;
  
  unsigned int getMaxDerivativeOrder(const NodePtr & expr) const;
  
  NodePtr substituteTestFunctions(const NodePtr & expr, const std::string & var_name);
  
  NodePtr substituteShapeFunctions(const NodePtr & expr, const std::string & var_name);
  
  std::vector<std::string> extractVariableNames(const NodePtr & expr);
  
  std::map<std::string, unsigned int> analyzeVariableDependencies(const NodePtr & expr);
  
  NodePtr performSymbolicSimplification(const NodePtr & expr);
  
  NodePtr eliminateZeroTerms(const NodePtr & expr);
  
  NodePtr factorCommonTerms(const NodePtr & expr);
  
  NodePtr collectLikeTerms(const NodePtr & expr);
};

class SymbolicSimplifier
{
public:
  NodePtr simplify(const NodePtr & expr);
  
private:
  NodePtr simplifyNode(const NodePtr & node);
  NodePtr simplifyVectorAssembly(const VectorAssemblyNode * node);
  
  NodePtr simplifyAdd(const BinaryOpNode * node);
  NodePtr simplifyMultiply(const BinaryOpNode * node);
  NodePtr simplifyDivide(const BinaryOpNode * node);
  NodePtr simplifyPower(const BinaryOpNode * node);
  
  bool isZero(const NodePtr & node) const;
  bool isOne(const NodePtr & node) const;
  bool isConstant(const NodePtr & node) const;
  
  Real getConstantValue(const NodePtr & node) const;
  
  NodePtr foldConstants(const NodePtr & left, const NodePtr & right, NodeType op);
  
  std::map<std::string, NodePtr> _simplified_cache;
};

class VariationalProblemAnalyzer
{
public:
  struct Analysis
  {
    bool is_well_posed;
    bool is_elliptic;
    bool is_parabolic;
    bool is_hyperbolic;
    bool has_unique_solution;
    bool is_conservative;
    bool is_symmetric;
    bool is_coercive;
    
    std::vector<std::string> warnings;
    std::vector<std::string> suggestions;
    
    unsigned int required_continuity;
    unsigned int recommended_fe_order;
    
    std::map<std::string, std::string> stabilization_recommendations;
  };
  
  Analysis analyze(const NodePtr & energy_density,
                   const std::vector<std::string> & variables);
  
  bool checkWellPosedness(const NodePtr & weak_form);
  
  bool checkExistence(const NodePtr & bilinear_form, const NodePtr & linear_form);
  
  bool checkUniqueness(const NodePtr & bilinear_form);
  
  bool checkStability(const NodePtr & weak_form, const std::string & norm_type = "L2");
  
  Real estimateConditionNumber(const NodePtr & bilinear_form);
  
  std::string recommendPreconditioner(const NodePtr & weak_form);
  
  std::vector<std::string> identifyPotentialIssues(const NodePtr & energy_density);
};

}
}
