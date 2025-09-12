#pragma once

#include "Kernel.h"
#include "automatic_weak_form/MooseAST.h"
#include "automatic_weak_form/MooseExpressionBuilder.h"
#include "automatic_weak_form/WeakFormGenerator.h"

namespace moose
{
namespace automatic_weak_form
{

class VariationalKernelBase : public Kernel
{
public:
  static InputParameters validParams();
  
  VariationalKernelBase(const InputParameters & parameters);
  
protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;
  
  void initializeExpression();
  
  void computeVariationalDerivative();
  
  Real evaluateC0Contribution();
  
  Real evaluateC1Contribution();
  
  Real evaluateC2Contribution();
  
  Real evaluateC3Contribution();
  
  MooseValue evaluateExpression(const NodePtr & expr);
  
  MooseValue evaluateAtQP(const NodePtr & expr, unsigned int qp);
  
  void setupFieldVariables();
  
  void setupCoupledVariables();
  
  void updateVariableValues(unsigned int qp);
  
protected:
  std::unique_ptr<MooseExpressionBuilder> _builder;
  
  NodePtr _energy_density;
  
  std::unique_ptr<WeakFormGenerator> _weak_form_gen;
  
  std::map<std::string, const VariableValue *> _coupled_values;
  std::map<std::string, const VariableGradient *> _coupled_gradients;
  std::map<std::string, const VariableSecond *> _coupled_seconds;
  
  std::map<std::string, unsigned int> _coupled_var_nums;
  
  std::map<std::string, MooseValue> _variable_cache;
  
  unsigned int _max_derivative_order;
  
  bool _has_time_derivatives;
  
  bool _compute_jacobian_numerically;
  
  Real _fd_eps;
  
  enum class EnergyType
  {
    DOUBLE_WELL,
    ELASTIC,
    NEO_HOOKEAN,
    SURFACE,
    CAHN_HILLIARD,
    FOURTH_ORDER,
    CUSTOM
  } _energy_type;
  
  std::string _energy_expression;
  
  Real _gradient_coefficient;
  Real _fourth_order_coefficient;
  Real _elastic_lambda;
  Real _elastic_mu;
  Real _surface_energy_coefficient;
  
  std::vector<std::string> _coupled_variable_names;
  
  bool _use_automatic_differentiation;
  
  bool _enable_variable_splitting;
  
  unsigned int _fe_order;
  
  std::map<std::string, NodePtr> _split_variables;
  
  struct ContributionCache
  {
    bool computed = false;
    std::map<unsigned int, MooseValue> values;
  } _c0_cache, _c1_cache, _c2_cache, _c3_cache;
  
  void clearCache()
  {
    _c0_cache.computed = false;
    _c1_cache.computed = false;
    _c2_cache.computed = false;
    _c3_cache.computed = false;
    _variable_cache.clear();
  }
};

}
}