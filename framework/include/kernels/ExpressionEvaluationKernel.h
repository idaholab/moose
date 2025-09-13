#pragma once

#include "Kernel.h"
#include "MooseAST.h"
#include "ExpressionEvaluator.h"

/**
 * Generic kernel for evaluating arbitrary expressions derived from strong forms
 * This kernel evaluates the weak form residual and Jacobian from symbolic expressions
 */
class ExpressionEvaluationKernel : public Kernel
{
public:
  static InputParameters validParams();
  
  ExpressionEvaluationKernel(const InputParameters & parameters);
  
protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;
  
private:
  /// The weak form residual expression (already includes test function)
  moose::automatic_weak_form::NodePtr _residual_expr;
  
  /// The Jacobian expression (derivative of residual w.r.t. variable)
  moose::automatic_weak_form::NodePtr _jacobian_expr;
  
  /// Off-diagonal Jacobian expressions for coupled variables
  std::map<unsigned int, moose::automatic_weak_form::NodePtr> _off_diag_jacobian_exprs;
  
  /// Expression evaluator for runtime evaluation
  std::unique_ptr<moose::automatic_weak_form::ExpressionEvaluator> _evaluator;
  
  /// Coupled variable names and IDs
  std::vector<std::string> _coupled_var_names;
  std::vector<unsigned int> _coupled_var_ids;
  std::vector<const VariableValue *> _coupled_vals;
  std::vector<const VariableGradient *> _coupled_grads;
  
  /// Whether to use automatic differentiation for Jacobian
  bool _use_ad;
  
  /// Whether this is a transient term (handled by TimeDerivative kernel)
  bool _is_transient_term;
  
  /// Set up the evaluation context with current QP values
  void setupEvaluationContext();
};