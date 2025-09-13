#include "ExpressionEvaluationKernel.h"
#include "StringExpressionParser.h"

registerMooseObject("MooseApp", ExpressionEvaluationKernel);

InputParameters
ExpressionEvaluationKernel::validParams()
{
  InputParameters params = Kernel::validParams();
  
  params.addClassDescription("Generic kernel for evaluating arbitrary expressions from strong forms");
  
  params.addParam<std::string>("residual_expression", "", 
                                "Weak form residual expression (as string)");
  
  params.addParam<std::string>("jacobian_expression", "", 
                                "Jacobian expression (as string)");
  
  params.addParam<std::map<std::string, std::string>>("off_diagonal_jacobian_expressions",
                                                        "Off-diagonal Jacobian expressions for coupled variables");
  
  params.addParam<std::vector<std::string>>("coupled_variables", 
                                              "List of coupled variable names");
  
  params.addParam<bool>("use_automatic_differentiation", true,
                         "Use AD to compute Jacobian if not provided");
  
  params.addParam<bool>("is_transient_term", false,
                         "Whether this is a transient term (time derivative handled separately)");
  
  params.addParam<std::map<std::string, Real>>("parameters",
                                                 "Parameters used in expressions");
  
  params.addParam<std::string>("intermediate_expressions", "",
                                "Semicolon-separated intermediate expression definitions");
  
  return params;
}

ExpressionEvaluationKernel::ExpressionEvaluationKernel(const InputParameters & parameters)
  : Kernel(parameters),
    _use_ad(getParam<bool>("use_automatic_differentiation")),
    _is_transient_term(getParam<bool>("is_transient_term"))
{
  // Parse expressions using StringExpressionParser
  moose::automatic_weak_form::StringExpressionParser parser(_mesh.dimension());
  
  // Set up parameters
  if (isParamValid("parameters"))
  {
    const auto & params = getParam<std::map<std::string, Real>>("parameters");
    for (const auto & [name, value] : params)
      parser.setParameter(name, value);
  }
  
  // Define the primary variable
  parser.defineVariable(_var.name());
  
  // Set up coupled variables
  if (isParamValid("coupled_variables"))
  {
    _coupled_var_names = getParam<std::vector<std::string>>("coupled_variables");
    
    for (const auto & var_name : _coupled_var_names)
    {
      unsigned int coupled_var = coupled(var_name);
      _coupled_var_ids.push_back(coupled_var);
      _coupled_vals.push_back(&coupledValue(var_name));
      _coupled_grads.push_back(&coupledGradient(var_name));
      
      parser.defineVariable(var_name);
    }
  }
  
  // Parse intermediate expressions
  if (isParamValid("intermediate_expressions"))
  {
    const std::string & expr_list = getParam<std::string>("intermediate_expressions");
    parser.parseExpressions(expr_list);
  }
  
  // Parse residual expression
  const std::string & residual_expr_str = getParam<std::string>("residual_expression");
  if (!residual_expr_str.empty())
  {
    _residual_expr = parser.parse(residual_expr_str);
  }
  else
  {
    mooseError("Residual expression is required");
  }
  
  // Parse Jacobian expression if provided
  const std::string & jacobian_expr_str = getParam<std::string>("jacobian_expression");
  if (!jacobian_expr_str.empty())
  {
    _jacobian_expr = parser.parse(jacobian_expr_str);
  }
  else if (_use_ad)
  {
    // Will compute Jacobian via AD
    _jacobian_expr = nullptr;
  }
  else
  {
    mooseError("Jacobian expression required when not using AD");
  }
  
  // Parse off-diagonal Jacobian expressions
  if (isParamValid("off_diagonal_jacobian_expressions"))
  {
    const auto & off_diag_exprs = getParam<std::map<std::string, std::string>>("off_diagonal_jacobian_expressions");
    
    for (const auto & [var_name, expr_str] : off_diag_exprs)
    {
      // Find the variable ID
      auto it = std::find(_coupled_var_names.begin(), _coupled_var_names.end(), var_name);
      if (it != _coupled_var_names.end())
      {
        size_t idx = std::distance(_coupled_var_names.begin(), it);
        _off_diag_jacobian_exprs[_coupled_var_ids[idx]] = parser.parse(expr_str);
      }
    }
  }
  
  // Create the expression evaluator
  _evaluator = std::make_unique<moose::automatic_weak_form::ExpressionEvaluator>(_mesh.dimension());
  
  // Register variable name to number mappings for efficient lookup
  _evaluator->registerVariable(_var.name(), _var.number());
  
  // Register coupled variables
  for (size_t i = 0; i < _coupled_var_names.size(); ++i)
  {
    // Use the coupled variable ID directly - it's already the variable number
    _evaluator->registerVariable(_coupled_var_names[i], _coupled_var_ids[i]);
  }
}

void
ExpressionEvaluationKernel::setupEvaluationContext()
{
  // Set current quadrature point values using variable number for efficiency
  _evaluator->setFieldValue(_var.number(), _u[_qp]);
  _evaluator->setFieldGradient(_var.number(), _grad_u[_qp]);
  
  // Set test function values
  _evaluator->setTestFunction(_var.number(), _test[_i][_qp]);
  _evaluator->setTestGradient(_var.number(), _grad_test[_i][_qp]);
  
  // Set shape function values for Jacobian
  _evaluator->setShapeFunction(_var.number(), _phi[_j][_qp]);
  _evaluator->setShapeGradient(_var.number(), _grad_phi[_j][_qp]);
  
  // Set coupled variable values
  for (size_t i = 0; i < _coupled_var_ids.size(); ++i)
  {
    _evaluator->setFieldValue(_coupled_var_ids[i], (*_coupled_vals[i])[_qp]);
    _evaluator->setFieldGradient(_coupled_var_ids[i], (*_coupled_grads[i])[_qp]);
  }
  
  // Set current position
  _evaluator->setPosition(_q_point[_qp]);
  
  // Set current time
  _evaluator->setTime(_t);
}

Real
ExpressionEvaluationKernel::computeQpResidual()
{
  setupEvaluationContext();
  
  // Evaluate the residual expression
  // Note: The residual expression should already include the test function
  // from the weak form derivation in the action
  return _evaluator->evaluate(_residual_expr);
}

Real
ExpressionEvaluationKernel::computeQpJacobian()
{
  setupEvaluationContext();
  
  if (_jacobian_expr)
  {
    // Evaluate the provided Jacobian expression
    return _evaluator->evaluate(_jacobian_expr);
  }
  else if (_use_ad)
  {
    // Use automatic differentiation
    // This would require AD support in the evaluator
    return _evaluator->evaluateDerivative(_residual_expr, _var.name());
  }
  else
  {
    // Fall back to base class finite difference approximation
    return Kernel::computeQpJacobian();
  }
}

Real
ExpressionEvaluationKernel::computeQpOffDiagJacobian(unsigned int jvar)
{
  setupEvaluationContext();
  
  // Check if we have an expression for this off-diagonal term
  auto it = _off_diag_jacobian_exprs.find(jvar);
  if (it != _off_diag_jacobian_exprs.end())
  {
    return _evaluator->evaluate(it->second);
  }
  else if (_use_ad)
  {
    // Use variable number directly for AD - more efficient
    return _evaluator->evaluateDerivativeByVarNum(_residual_expr, jvar);
  }
  
  return 0.0;
}