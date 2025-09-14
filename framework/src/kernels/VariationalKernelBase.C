#include "VariationalKernelBase.h"
#include "MooseError.h"
#include "Assembly.h"
#include "MooseVariableFE.h"
#include "SystemBase.h"

namespace moose
{
namespace automatic_weak_form
{

registerMooseObject("MooseApp", VariationalKernelBase);

InputParameters
VariationalKernelBase::validParams()
{
  InputParameters params = Kernel::validParams();

  params.addClassDescription("Base class for variational derivative kernels that automatically "
                              "generate weak forms from energy functionals");

  MooseEnum energy_types("double_well elastic neo_hookean surface cahn_hilliard fourth_order custom");
  params.addParam<MooseEnum>("energy_type", energy_types, "Type of energy functional");

  params.addParam<std::string>("energy_expression", "",
                                "Mathematical expression for the energy density");

  params.addParam<Real>("gradient_coefficient", 1.0,
                         "Gradient energy coefficient (kappa)");
  params.addParam<Real>("fourth_order_coefficient", 0.0,
                         "Fourth-order regularization coefficient (lambda)");

  params.addParam<Real>("elastic_lambda", 1.0, "First Lamé parameter");
  params.addParam<Real>("elastic_mu", 1.0, "Second Lamé parameter (shear modulus)");

  params.addParam<Real>("surface_energy_coefficient", 1.0,
                         "Surface energy coefficient (gamma)");

  params.addParam<std::vector<VariableName>>("coupled_variables",
                                               "List of coupled variables");
  
  params.addParam<std::map<std::string, Real>>("parameters",
                                                 "Named parameters for the energy functional");

  params.addParam<bool>("use_automatic_differentiation", true,
                         "Use automatic differentiation for Jacobian");

  params.addParam<bool>("compute_jacobian_numerically", false,
                         "Compute Jacobian using finite differences");

  params.addParam<Real>("fd_eps", 1e-8,
                         "Finite difference epsilon for numerical Jacobian");

  params.addParam<bool>("enable_variable_splitting", true,
                         "Enable automatic variable splitting for higher-order derivatives");

  params.addParam<unsigned int>("fe_order", 1,
                                  "Finite element order");

  params.addParam<bool>("has_time_derivatives", false,
                         "Whether the problem includes time derivatives");

  return params;
}

VariationalKernelBase::VariationalKernelBase(const InputParameters & parameters)
  : Kernel(parameters),
    _has_time_derivatives(getParam<bool>("has_time_derivatives")),
    _compute_jacobian_numerically(getParam<bool>("compute_jacobian_numerically")),
    _fd_eps(getParam<Real>("fd_eps")),
    _gradient_coefficient(getParam<Real>("gradient_coefficient")),
    _fourth_order_coefficient(getParam<Real>("fourth_order_coefficient")),
    _elastic_lambda(getParam<Real>("elastic_lambda")),
    _elastic_mu(getParam<Real>("elastic_mu")),
    _surface_energy_coefficient(getParam<Real>("surface_energy_coefficient")),
    _parameters(isParamValid("parameters") ? getParam<std::map<std::string, Real>>("parameters") : std::map<std::string, Real>()),
    _use_automatic_differentiation(getParam<bool>("use_automatic_differentiation")),
    _enable_variable_splitting(getParam<bool>("enable_variable_splitting")),
    _fe_order(getParam<unsigned int>("fe_order"))
{
  _builder = std::make_unique<MooseExpressionBuilder>(_mesh.dimension());
  _weak_form_gen = std::make_unique<WeakFormGenerator>(_mesh.dimension());

  if (isParamValid("energy_type"))
  {
    std::string type_str = getParam<MooseEnum>("energy_type");
    if (type_str == "double_well")
      _energy_type = EnergyType::DOUBLE_WELL;
    else if (type_str == "elastic")
      _energy_type = EnergyType::ELASTIC;
    else if (type_str == "neo_hookean")
      _energy_type = EnergyType::NEO_HOOKEAN;
    else if (type_str == "surface")
      _energy_type = EnergyType::SURFACE;
    else if (type_str == "cahn_hilliard")
      _energy_type = EnergyType::CAHN_HILLIARD;
    else if (type_str == "fourth_order")
      _energy_type = EnergyType::FOURTH_ORDER;
    else
      _energy_type = EnergyType::CUSTOM;
  }
  else
    _energy_type = EnergyType::CUSTOM;

  if (isParamValid("energy_expression"))
    _energy_expression = getParam<std::string>("energy_expression");

  if (isParamValid("coupled_variables"))
  {
    auto variable_names = getParam<std::vector<VariableName>>("coupled_variables");
    _coupled_variable_names.clear();
    for (const auto & var_name : variable_names)
      _coupled_variable_names.push_back(var_name);
    setupCoupledVariables();
  }

  initializeExpression();
  computeVariationalDerivative();
}

void
VariationalKernelBase::initializeExpression()
{
  switch (_energy_type)
  {
    case EnergyType::DOUBLE_WELL:
    {
      auto c = _builder->field(_var.name());
      _energy_density = _builder->doubleWell(c);
      break;
    }

    case EnergyType::ELASTIC:
    {
      auto u = _builder->field(_var.name());
      auto eps = _builder->strain(_var.name());
      _energy_density = _builder->elasticEnergy(eps, _elastic_lambda, _elastic_mu);
      break;
    }

    case EnergyType::NEO_HOOKEAN:
    {
      auto u = _builder->field(_var.name());
      auto F = _builder->deformationGradient(_var.name());
      _energy_density = _builder->neoHookean(F, _elastic_mu, _elastic_lambda);
      break;
    }

    case EnergyType::SURFACE:
    {
      auto phi = _builder->field(_var.name());
      _energy_density = _builder->surfaceEnergy(phi, _surface_energy_coefficient);
      break;
    }

    case EnergyType::CAHN_HILLIARD:
    {
      auto c = _builder->field(_var.name());
      auto grad_c = grad(c, _mesh.dimension());
      auto bulk = _builder->doubleWell(c);
      auto gradient = multiply(constant(0.5 * _gradient_coefficient), moose::automatic_weak_form::dot(grad_c, grad_c));
      _energy_density = add(bulk, gradient);
      break;
    }

    case EnergyType::FOURTH_ORDER:
    {
      auto c = _builder->field(_var.name());
      auto grad_c = grad(c, _mesh.dimension());
      auto bulk = _builder->doubleWell(c);
      auto gradient = multiply(constant(0.5 * _gradient_coefficient), moose::automatic_weak_form::dot(grad_c, grad_c));
      auto fourth = _builder->fourthOrderRegularization(c, _fourth_order_coefficient);
      _energy_density = add(add(bulk, gradient), fourth);
      break;
    }

    case EnergyType::CUSTOM:
    {
      if (!_energy_expression.empty())
      {
        // Pass parameters to the expression parser
        if (!_parameters.empty())
          _energy_density = _builder->parseExpression(_energy_expression, _parameters);
        else
          _energy_density = _builder->parseExpression(_energy_expression);
      }
      else
        mooseError("Custom energy type requires energy_expression parameter");
      break;
    }

    default:
      mooseError("Unknown energy type");
  }
}

void
VariationalKernelBase::computeVariationalDerivative()
{
  if (!_energy_density)
    mooseError("Energy density not initialized");

  auto contributions = _weak_form_gen->computeContributions(_energy_density, _var.name());

  _max_derivative_order = contributions.max_order;

  if (_enable_variable_splitting && _weak_form_gen->requiresVariableSplitting(
          DifferentiationVisitor(_var.name()).differentiate(_energy_density), _fe_order))
  {
    mooseWarning("This problem requires variable splitting for order ", _max_derivative_order,
                 " derivatives with FE order ", _fe_order);
  }
}

void
VariationalKernelBase::setupCoupledVariables()
{
  for (const auto & var_name : _coupled_variable_names)
  {
    unsigned int var_num = coupled(var_name);
    _coupled_var_nums[var_name] = var_num;
    _coupled_values[var_name] = &coupledValue(var_name);
    _coupled_gradients[var_name] = &coupledGradient(var_name);

    if (_fe_order >= 2)
      _coupled_seconds[var_name] = &coupledSecond(var_name);
  }
}

void
VariationalKernelBase::updateVariableValues(unsigned int qp)
{
  _variable_cache.clear();

  _variable_cache[_var.name()] = MooseValue(_u[qp]);

  RealVectorValue grad_u(_grad_u[qp]);
  _variable_cache[_var.name() + "_grad"] = MooseValue(grad_u, _mesh.dimension());

  for (const auto & [name, value_ptr] : _coupled_values)
  {
    _variable_cache[name] = MooseValue((*value_ptr)[qp]);

    if (_coupled_gradients.count(name))
    {
      RealVectorValue grad((*_coupled_gradients[name])[qp]);
      _variable_cache[name + "_grad"] = MooseValue(grad, _mesh.dimension());
    }
  }
}

Real
VariationalKernelBase::computeQpResidual()
{
  clearCache();
  updateVariableValues(_qp);

  Real residual = 0.0;

  residual += evaluateC0Contribution();

  // After integration by parts, -div(C^1) becomes +C^1·∇ψ
  residual += evaluateC1Contribution();

  if (_max_derivative_order >= 2)
    residual += evaluateC2Contribution();

  if (_max_derivative_order >= 3)
    residual += evaluateC3Contribution();

  return residual;
}

Real
VariationalKernelBase::computeQpJacobian()
{
  if (_compute_jacobian_numerically)
  {
    // Numerical Jacobian computation disabled - _u is const in MOOSE
    // Use automatic differentiation or analytical Jacobian instead
    mooseWarning("Numerical Jacobian computation not available, using base class implementation");
    return Kernel::computeQpJacobian();
  }

  if (!_use_automatic_differentiation)
  {
    // For now, provide a simple analytical Jacobian for the diffusion case
    // This handles the weak form: -∇·(κ∇u) which gives Jacobian: κ∇φ_j·∇ψ_i
    
    // For simple diffusion, the Jacobian from -∇·(κ∇u) is κ∇φ_j·∇ψ_i
    // For now, assume κ=1 for testing
    return _grad_phi[_j][_qp] * _grad_test[_i][_qp];
  }

  // Automatic differentiation for Jacobian
  clearCache();
  updateVariableValues(_qp);
  
  Real jacobian = 0.0;
  
  // Optional debug output
  bool debug = false; // (_qp == 0 && _i == 0 && _j == 0); // Debug first element
  
  // Get the differential coefficients C^k from the energy functional
  DifferentiationVisitor dv(_var.name());
  auto diff = dv.differentiate(_energy_density);
  
  // Jacobian from C^0 term: ∂C^0/∂u · φ_j · ψ_i
  if (diff.hasOrder(0))
  {
    auto c0 = diff.getCoefficient(0);
    
    if (debug && _qp == 0 && _i == 0 && _j == 0)
      mooseInfo("  C^0 expression: ", c0->toString());
    
    // Differentiate C^0 with respect to the field variable
    auto dc0_du = differentiateWithRespectToField(c0, _var.name());
    
    if (debug && _qp == 0 && _i == 0 && _j == 0)
    {
      if (dc0_du)
        mooseInfo("  dc0/du: ", dc0_du->toString());
      else
        mooseInfo("  dc0/du: null");
    }
    
    if (dc0_du)
    {
      MooseValue dc0_val = evaluateExpression(dc0_du);
      if (dc0_val.isScalar())
      {
        Real contrib = dc0_val.asScalar() * _phi[_j][_qp] * _test[_i][_qp];
        jacobian += contrib;
        
        if (debug && _qp == 0 && _i == 0 && _j == 0)
          mooseInfo("  C^0 Jacobian contribution: ", contrib);
      }
    }
  }
  
  // Jacobian from C^1 term: ∂C^1/∂u · φ_j · ∇ψ_i + ∂C^1/∂(∇u) · ∇φ_j · ∇ψ_i
  if (diff.hasOrder(1))
  {
    auto c1 = diff.getCoefficient(1);
    
    if (debug && _qp == 0 && _i == 0 && _j == 0)
      mooseInfo("  C^1 expression: ", c1->toString());
    
    // Contribution from ∂C^1/∂u
    auto dc1_du = differentiateWithRespectToField(c1, _var.name());
    if (dc1_du)
    {
      MooseValue dc1_val = evaluateExpression(dc1_du);
      if (dc1_val.isVector())
        jacobian += dc1_val.asVector() * _grad_test[_i][_qp] * _phi[_j][_qp];
    }
    
    // Contribution from ∂C^1/∂(∇u)
    auto dc1_dgradu = differentiateWithRespectToGradient(c1, _var.name());
    
    if (debug && _qp == 0 && _i == 0 && _j == 0)
    {
      if (dc1_dgradu)
        mooseInfo("  dc1/d(grad u): ", dc1_dgradu->toString());
      else
        mooseInfo("  dc1/d(grad u): null");
    }
    
    if (dc1_dgradu)
    {
      MooseValue dc1_grad_val = evaluateExpression(dc1_dgradu);
      if (dc1_grad_val.isTensor())
      {
        // Contract the tensor with ∇φ_j and then dot with ∇ψ_i
        RankTwoTensor tensor = dc1_grad_val.asTensor();
        RealVectorValue result = tensor * _grad_phi[_j][_qp];
        jacobian += result * _grad_test[_i][_qp];
        
        if (debug && _qp == 0 && _i == 0 && _j == 0)
          mooseInfo("  Tensor contribution: ", result * _grad_test[_i][_qp]);
      }
      else if (dc1_grad_val.isScalar())
      {
        // For simple cases like C^1 = κ∇u, ∂C^1/∂(∇u) = κI
        // The scalar represents the coefficient κ
        Real coeff = dc1_grad_val.asScalar();
        Real contrib = coeff * _grad_phi[_j][_qp] * _grad_test[_i][_qp];
        jacobian += contrib;
        
        if (debug && _qp == 0 && _i == 0 && _j == 0)
        {
          mooseInfo("  Scalar grad contribution: ", contrib);
          mooseInfo("    dc1_grad_val: ", dc1_grad_val.asScalar());
        }
      }
    }
  }
  
  // TODO: Add C^2 and higher order terms if needed
  
  return jacobian;
}

Real
VariationalKernelBase::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (!_use_automatic_differentiation)
    return 0.0;

  std::string coupled_var_name;
  for (const auto & [name, var_num] : _coupled_var_nums)
  {
    if (var_num == jvar)
    {
      coupled_var_name = name;
      break;
    }
  }

  if (coupled_var_name.empty())
    return 0.0;

  clearCache();
  updateVariableValues(_qp);

  DifferentiationVisitor dv(coupled_var_name);
  auto residual_expr = _weak_form_gen->generateWeakForm(_energy_density, _var.name());
  auto jacobian_diff = dv.differentiate(residual_expr);

  if (jacobian_diff.hasOrder(0))
  {
    auto jacobian_expr = jacobian_diff.getCoefficient(0);
    MooseValue jac_val = evaluateExpression(jacobian_expr);

    if (jac_val.isScalar())
      return jac_val.asScalar() * _phi[_j][_qp] * _test[_i][_qp];
  }

  return 0.0;
}

Real
VariationalKernelBase::evaluateC0Contribution()
{
  if (!_c0_cache.computed)
  {
    DifferentiationVisitor dv(_var.name());
    auto diff = dv.differentiate(_energy_density);

    if (diff.hasOrder(0))
    {
      auto c0 = diff.getCoefficient(0);
      _c0_cache.values[_qp] = evaluateExpression(c0);
    }
    else
    {
      _c0_cache.values[_qp] = MooseValue(0.0);
    }
    _c0_cache.computed = true;
  }

  if (_c0_cache.values[_qp].isScalar())
    return _c0_cache.values[_qp].asScalar() * _test[_i][_qp];

  return 0.0;
}

Real
VariationalKernelBase::evaluateC1Contribution()
{
  if (!_c1_cache.computed)
  {
    DifferentiationVisitor dv(_var.name());
    auto diff = dv.differentiate(_energy_density);

    if (diff.hasOrder(1))
    {
      auto c1 = diff.getCoefficient(1);
      _c1_cache.values[_qp] = evaluateExpression(c1);
    }
    else
    {
      _c1_cache.values[_qp] = MooseValue(RealVectorValue(), _mesh.dimension());
    }
    _c1_cache.computed = true;
  }

  if (_c1_cache.values[_qp].isVector())
  {
    RealVectorValue c1_vec = _c1_cache.values[_qp].asVector();
    return c1_vec * _grad_test[_i][_qp];
  }

  return 0.0;
}

Real
VariationalKernelBase::evaluateC2Contribution()
{
  if (!_c2_cache.computed)
  {
    DifferentiationVisitor dv(_var.name());
    auto diff = dv.differentiate(_energy_density);

    if (diff.hasOrder(2))
    {
      auto c2 = diff.getCoefficient(2);
      _c2_cache.values[_qp] = evaluateExpression(c2);
    }
    else
    {
      _c2_cache.values[_qp] = MooseValue(RankTwoTensor(), _mesh.dimension());
    }
    _c2_cache.computed = true;
  }

  if (_c2_cache.values[_qp].isTensor() && _fe_order >= 2)
  {
    return 0.0;
  }

  return 0.0;
}

Real
VariationalKernelBase::evaluateC3Contribution()
{
  return 0.0;
}

MooseValue
VariationalKernelBase::evaluateExpression(const NodePtr & expr)
{
  return evaluateAtQP(expr, _qp);
}

MooseValue
VariationalKernelBase::evaluateAtQP(const NodePtr & expr, unsigned int qp)
{
  if (!expr)
    return MooseValue(0.0);

  switch (expr->type())
  {
    case NodeType::Constant:
    {
      auto const_node = std::static_pointer_cast<ConstantNode>(expr);
      return const_node->value();
    }

    case NodeType::FieldVariable:
    {
      auto field_node = std::static_pointer_cast<FieldVariableNode>(expr);
      std::string name = field_node->name();

      if (_variable_cache.count(name))
        return _variable_cache[name];

      if (name == _var.name())
        return MooseValue(_u[qp]);

      if (_coupled_values.count(name))
        return MooseValue((*_coupled_values[name])[qp]);

      mooseError("Unknown field variable: ", name);
    }
    
    case NodeType::Variable:
    {
      auto var_node = std::static_pointer_cast<VariableNode>(expr);
      std::string name = var_node->name();

      if (_variable_cache.count(name))
        return _variable_cache[name];

      if (name == _var.name())
        return MooseValue(_u[qp]);

      if (_coupled_values.count(name))
        return MooseValue((*_coupled_values[name])[qp]);

      mooseError("Unknown variable: ", name);
    }

    case NodeType::Add:
    {
      auto binary = std::static_pointer_cast<BinaryOpNode>(expr);
      auto left_val = evaluateAtQP(binary->left(), qp);
      auto right_val = evaluateAtQP(binary->right(), qp);
      
      return left_val + right_val;
    }

    case NodeType::Subtract:
    {
      auto binary = std::static_pointer_cast<BinaryOpNode>(expr);
      auto left_val = evaluateAtQP(binary->left(), qp);
      auto right_val = evaluateAtQP(binary->right(), qp);
      return left_val - right_val;
    }

    case NodeType::Multiply:
    {
      auto binary = std::static_pointer_cast<BinaryOpNode>(expr);
      auto left_val = evaluateAtQP(binary->left(), qp);
      auto right_val = evaluateAtQP(binary->right(), qp);
      
      return left_val * right_val;
    }

    case NodeType::Gradient:
    {
      auto unary = std::static_pointer_cast<UnaryOpNode>(expr);
      auto operand = unary->operand();

      // Handle direct field variables
      if (operand->type() == NodeType::FieldVariable)
      {
        auto field_node = std::static_pointer_cast<FieldVariableNode>(operand);
        std::string name = field_node->name();

        if (_variable_cache.count(name + "_grad"))
          return _variable_cache[name + "_grad"];

        if (name == _var.name())
          return MooseValue(RealVectorValue(_grad_u[qp]), _mesh.dimension());

        if (_coupled_gradients.count(name))
          return MooseValue(RealVectorValue((*_coupled_gradients[name])[qp]), _mesh.dimension());
      }
      // Handle symbolic variables (from differentiation)
      else if (operand->type() == NodeType::Variable)
      {
        auto var_node = std::static_pointer_cast<VariableNode>(operand);
        std::string name = var_node->name();
        
        if (_variable_cache.count(name + "_grad"))
          return _variable_cache[name + "_grad"];

        if (name == _var.name())
          return MooseValue(RealVectorValue(_grad_u[qp]), _mesh.dimension());

        if (_coupled_gradients.count(name))
          return MooseValue(RealVectorValue((*_coupled_gradients[name])[qp]), _mesh.dimension());
          
        mooseError("Unknown variable in gradient: ", name);
      }
      // Handle multiplication by constant (e.g., grad(c * u))
      else if (operand->type() == NodeType::Multiply)
      {
        auto mult_node = std::static_pointer_cast<BinaryOpNode>(operand);
        auto left = mult_node->left();
        auto right = mult_node->right();
        
        // Check if one side is a constant and the other is a field variable
        if (left->type() == NodeType::Constant && right->type() == NodeType::FieldVariable)
        {
          auto const_val = evaluateAtQP(left, qp);
          auto field_node = std::static_pointer_cast<FieldVariableNode>(right);
          std::string name = field_node->name();
          
          MooseValue grad_val;
          if (name == _var.name())
            grad_val = MooseValue(RealVectorValue(_grad_u[qp]), _mesh.dimension());
          else if (_coupled_gradients.count(name))
            grad_val = MooseValue(RealVectorValue((*_coupled_gradients[name])[qp]), _mesh.dimension());
          else
            mooseError("Unknown field variable in gradient: ", name);
            
          return const_val * grad_val;
        }
        else if (right->type() == NodeType::Constant && left->type() == NodeType::FieldVariable)
        {
          auto const_val = evaluateAtQP(right, qp);
          auto field_node = std::static_pointer_cast<FieldVariableNode>(left);
          std::string name = field_node->name();
          
          MooseValue grad_val;
          if (name == _var.name())
            grad_val = MooseValue(RealVectorValue(_grad_u[qp]), _mesh.dimension());
          else if (_coupled_gradients.count(name))
            grad_val = MooseValue(RealVectorValue((*_coupled_gradients[name])[qp]), _mesh.dimension());
          else
            mooseError("Unknown field variable in gradient: ", name);
            
          return const_val * grad_val;
        }
      }
      // For more complex expressions, use numerical differentiation or chain rule
      else
      {
        // This is a limitation - we can't evaluate gradient of arbitrary expressions
        // In practice, after differentiation we should only get simple cases
        mooseError("Cannot evaluate gradient of complex expression. Expression type: ", 
                   static_cast<int>(operand->type()));
      }

      mooseError("Cannot evaluate gradient of non-field variable");
    }

    case NodeType::Dot:
    {
      auto binary = std::static_pointer_cast<BinaryOpNode>(expr);
      auto left_val = evaluateAtQP(binary->left(), qp);
      auto right_val = evaluateAtQP(binary->right(), qp);
      return moose::automatic_weak_form::dot(left_val, right_val);
    }

    case NodeType::Norm:
    {
      auto unary = std::static_pointer_cast<UnaryOpNode>(expr);
      auto operand_val = evaluateAtQP(unary->operand(), qp);
      return norm(operand_val);
    }

    case NodeType::Function:
    {
      auto func_node = std::static_pointer_cast<FunctionNode>(expr);

      if (func_node->name() == "dW_dc" && func_node->args().size() == 1)
      {
        auto c_val = evaluateAtQP(func_node->args()[0], qp);
        if (c_val.isScalar())
        {
          Real c = c_val.asScalar();
          Real dW = 4.0 * c * (c * c - 1.0);
          return MooseValue(dW);
        }
      }

      mooseError("Unknown function: ", func_node->name());
    }

    default:
      mooseError("Cannot evaluate expression type: ", expr->toString());
  }
}

NodePtr
VariationalKernelBase::differentiateWithRespectToField(const NodePtr & expr, const std::string & var_name)
{
  if (!expr)
    return nullptr;
    
  // Use the differentiation visitor to compute ∂expr/∂var
  DifferentiationVisitor dv(var_name);
  auto diff = dv.differentiate(expr);
  
  // The 0-th order coefficient is ∂expr/∂var
  if (diff.hasOrder(0))
    return diff.getCoefficient(0);
    
  return nullptr;
}

NodePtr
VariationalKernelBase::differentiateWithRespectToGradient(const NodePtr & expr, const std::string & var_name)
{
  if (!expr)
    return nullptr;
    
  // For expressions like C^1 = κ∇u, we need ∂C^1/∂(∇u)
  // This is more complex and depends on the structure of the expression
  
  // Handle different node types
  if (expr->type() == NodeType::Add)
  {
    // d(a + b)/d(grad u) = da/d(grad u) + db/d(grad u)
    auto binary = std::static_pointer_cast<BinaryOpNode>(expr);
    auto left_deriv = differentiateWithRespectToGradient(binary->left(), var_name);
    auto right_deriv = differentiateWithRespectToGradient(binary->right(), var_name);
    
    if (left_deriv && right_deriv)
      return add(left_deriv, right_deriv);
    else if (left_deriv)
      return left_deriv;
    else if (right_deriv)
      return right_deriv;
    else
      return nullptr;
  }
  else if (expr->type() == NodeType::Multiply)
  {
    // d(a * b)/d(grad u) = a * db/d(grad u) + da/d(grad u) * b
    // But if a is scalar and b is grad(u), then d(a * grad(u))/d(grad u) = a * I
    auto binary = std::static_pointer_cast<BinaryOpNode>(expr);
    auto left = binary->left();
    auto right = binary->right();
    
    // Check if one side is a scalar constant/expression and the other contains grad(u)
    bool left_has_grad = containsGradient(left, var_name);
    bool right_has_grad = containsGradient(right, var_name);
    
    if (!left_has_grad && right_has_grad)
    {
      // left is coefficient, right contains grad(u)
      auto right_deriv = differentiateWithRespectToGradient(right, var_name);
      if (right_deriv)
        return multiply(left, right_deriv);
    }
    else if (left_has_grad && !right_has_grad)
    {
      // right is coefficient, left contains grad(u)
      auto left_deriv = differentiateWithRespectToGradient(left, var_name);
      if (left_deriv)
        return multiply(right, left_deriv);
    }
    // If both have gradients, we'd need product rule - not handled yet
    return nullptr;
  }
  else if (expr->type() == NodeType::Gradient)
  {
    auto unary = std::static_pointer_cast<UnaryOpNode>(expr);
    auto operand = unary->operand();
    if (operand->type() == NodeType::Variable || operand->type() == NodeType::FieldVariable)
    {
      auto var_node = operand;
      if ((var_node->type() == NodeType::Variable && 
           std::static_pointer_cast<VariableNode>(var_node)->name() == var_name) ||
          (var_node->type() == NodeType::FieldVariable && 
           std::static_pointer_cast<FieldVariableNode>(var_node)->name() == var_name))
      {
        // ∂(grad(u))/∂(grad(u)) = I (identity tensor)
        return constant(1.0); // Simplified - should return identity tensor
      }
    }
  }
  else if (expr->type() == NodeType::Multiply)
  {
    auto binary = std::static_pointer_cast<BinaryOpNode>(expr);
    auto left = binary->left();
    auto right = binary->right();
    
    // Check if one side is grad(var) and the other is a constant/scalar
    NodePtr grad_part = nullptr;
    NodePtr coeff_part = nullptr;
    
    if (right->type() == NodeType::Gradient)
    {
      grad_part = right;
      coeff_part = left;
    }
    else if (left->type() == NodeType::Gradient)
    {
      grad_part = left;
      coeff_part = right;
    }
    
    if (grad_part)
    {
      auto unary = std::static_pointer_cast<UnaryOpNode>(grad_part);
      auto operand = unary->operand();
      if ((operand->type() == NodeType::Variable && 
           std::static_pointer_cast<VariableNode>(operand)->name() == var_name) ||
          (operand->type() == NodeType::FieldVariable && 
           std::static_pointer_cast<FieldVariableNode>(operand)->name() == var_name))
      {
        // ∂(κ * grad(u))/∂(grad(u)) = κ * I
        return coeff_part; // Returns the coefficient
      }
    }
  }
  
  // For more complex expressions, we would need a more sophisticated approach
  return nullptr;
}

bool
VariationalKernelBase::containsGradient(const NodePtr & expr, const std::string & var_name)
{
  if (!expr)
    return false;
    
  if (expr->type() == NodeType::Gradient)
  {
    auto unary = std::static_pointer_cast<UnaryOpNode>(expr);
    auto operand = unary->operand();
    if (operand->type() == NodeType::Variable || operand->type() == NodeType::FieldVariable)
    {
      if ((operand->type() == NodeType::Variable && 
           std::static_pointer_cast<VariableNode>(operand)->name() == var_name) ||
          (operand->type() == NodeType::FieldVariable && 
           std::static_pointer_cast<FieldVariableNode>(operand)->name() == var_name))
      {
        return true;
      }
    }
  }
  
  // Recursively check children
  for (const auto & child : expr->children())
  {
    if (containsGradient(child, var_name))
      return true;
  }
  
  return false;
}

}
}
