#include "CahnHilliardVariational.h"
#include "MooseError.h"

registerMooseObject("MooseApp", CahnHilliardVariational);
registerMooseObject("MooseApp", FourthOrderCahnHilliardVariational);
registerMooseObject("MooseApp", AllenCahnVariational);

InputParameters
CahnHilliardVariational::validParams()
{
  InputParameters params = moose::automatic_weak_form::VariationalKernelBase::validParams();

  params.addClassDescription("Cahn-Hilliard kernel derived from variational principles");

  params.addParam<Real>("well_height", 1.0, "Height of double-well potential");
  params.addParam<Real>("kappa", 1.0, "Gradient energy coefficient");
  params.addParam<Real>("mobility", 1.0, "Mobility coefficient");
  params.addParam<bool>("split_formulation", false, "Use split CH formulation with chemical potential");
  params.addCoupledVar("mu", "Chemical potential (for split formulation)");
  params.addParam<bool>("use_analytical_jacobian", true, "Use analytical Jacobian");

  return params;
}

CahnHilliardVariational::CahnHilliardVariational(const InputParameters & parameters)
  : moose::automatic_weak_form::VariationalKernelBase(parameters),
    _well_height(getParam<Real>("well_height")),
    _kappa(getParam<Real>("kappa")),
    _mobility(getParam<Real>("mobility")),
    _split_formulation(getParam<bool>("split_formulation")),
    _use_analytical_jacobian(getParam<bool>("use_analytical_jacobian"))
{
  if (_split_formulation)
  {
    _mu = &coupledValue("mu");
    _grad_mu = &coupledGradient("mu");
    _mu_var = coupled("mu");
  }
  else
  {
    _mu = nullptr;
    _grad_mu = nullptr;
    _mu_var = 0;
  }

  buildEnergyFunctional();
}

void
CahnHilliardVariational::initialSetup()
{
  VariationalKernelBase::initialSetup();

  if (_split_formulation)
    setupSplitFormulation();
}

void
CahnHilliardVariational::buildEnergyFunctional()
{
  using namespace moose::automatic_weak_form;

  auto c = _builder->field(_var.name());

  _bulk_energy = _builder->doubleWell(c, _well_height);

  auto grad_c = grad(c, _mesh.dimension());
  _gradient_energy = multiply(constant(0.5 * _kappa), dot(grad_c, grad_c));

  _total_energy = add(_bulk_energy, _gradient_energy);

  _energy_density = _total_energy;

  computeVariationalDerivative();
}

void
CahnHilliardVariational::setupSplitFormulation()
{
  using namespace moose::automatic_weak_form;

  if (!_split_formulation)
    return;

  DifferentiationVisitor dv(_var.name());
  _variational_derivative = dv.differentiate(_total_energy);
}

void
CahnHilliardVariational::computeResidual()
{
  if (_split_formulation && _mu && _grad_mu)
  {
    prepareVectorTag(_assembly, _var.number());

    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      Real residual = -_mobility * (*_grad_mu)[_qp] * _grad_test[_i][_qp];
      residual *= _JxW[_qp] * _coord[_qp];
      _local_re(_i) += residual;
    }

    accumulateTaggedLocalResidual();
  }
  else
  {
    VariationalKernelBase::computeResidual();
  }
}

void
CahnHilliardVariational::computeJacobian()
{
  if (_split_formulation && _use_analytical_jacobian)
  {
    prepareMatrixTag(_assembly, _var.number(), _var.number());

    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      Real jac = 0.0;
      _local_ke(_i, _j) += jac * _JxW[_qp] * _coord[_qp];
    }

    accumulateTaggedLocalMatrix();
  }
  else
  {
    VariationalKernelBase::computeJacobian();
  }
}

void
CahnHilliardVariational::computeOffDiagJacobian(const MooseVariableFEBase & jvar)
{
  if (_split_formulation && jvar.number() == _mu_var && _use_analytical_jacobian)
  {
    prepareMatrixTag(_assembly, _var.number(), jvar.number());

    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      for (_j = 0; _j < _phi.size(); _j++)
      {
        Real jac = -_mobility * _grad_phi[_j][_qp] * _grad_test[_i][_qp];
        _local_ke(_i, _j) += jac * _JxW[_qp] * _coord[_qp];
      }
    }

    accumulateTaggedLocalMatrix();
  }
  else
  {
    VariationalKernelBase::computeOffDiagJacobian(jvar);
  }
}

Real
CahnHilliardVariational::computeDoubleWellDerivative()
{
  Real c = _u[_qp];
  return 4.0 * _well_height * c * (c * c - 1.0);
}

RealVectorValue
CahnHilliardVariational::computeGradientCoefficient()
{
  return _kappa * _grad_u[_qp];
}

InputParameters
FourthOrderCahnHilliardVariational::validParams()
{
  InputParameters params = moose::automatic_weak_form::VariationalKernelBase::validParams();

  params.addClassDescription("Fourth-order Cahn-Hilliard with automatic variable splitting");

  params.addParam<Real>("lambda", 0.1, "Fourth-order regularization coefficient");
  params.addParam<Real>("gamma", 0.01, "Sixth-order coefficient");
  params.addParam<bool>("enable_sixth_order", false, "Enable sixth-order terms");
  params.addParam<Real>("beta", 0.001, "Eighth-order coefficient");

  params.addCoupledVar("grad_c_x", "x-component of concentration gradient (split variable)");
  params.addCoupledVar("grad_c_y", "y-component of concentration gradient (split variable)");
  params.addCoupledVar("grad_c_z", "z-component of concentration gradient (split variable)");

  return params;
}

FourthOrderCahnHilliardVariational::FourthOrderCahnHilliardVariational(const InputParameters & parameters)
  : moose::automatic_weak_form::VariationalKernelBase(parameters),
    _lambda(getParam<Real>("lambda")),
    _gamma(getParam<Real>("gamma")),
    _enable_sixth_order(getParam<bool>("enable_sixth_order")),
    _beta(getParam<Real>("beta"))
{
  if (isCoupled("grad_c_x"))
  {
    _grad_c_x = &coupledValue("grad_c_x");
    _grad_grad_c_x = &coupledGradient("grad_c_x");
    _split_var_nums.push_back(coupled("grad_c_x"));
  }

  if (isCoupled("grad_c_y"))
  {
    _grad_c_y = &coupledValue("grad_c_y");
    _grad_grad_c_y = &coupledGradient("grad_c_y");
    _split_var_nums.push_back(coupled("grad_c_y"));
  }

  if (_mesh.dimension() == 3 && isCoupled("grad_c_z"))
  {
    _grad_c_z = &coupledValue("grad_c_z");
    _grad_grad_c_z = &coupledGradient("grad_c_z");
    _split_var_nums.push_back(coupled("grad_c_z"));
  }
}

void
FourthOrderCahnHilliardVariational::initialSetup()
{
  VariationalKernelBase::initialSetup();

  setupHigherOrderSplitting();

  using namespace moose::automatic_weak_form;

  auto c = _builder->field(_var.name());
  _fourth_order_term = _builder->fourthOrderRegularization(c, _lambda);

  if (_enable_sixth_order)
  {
    auto hess_c = _builder->hessianField(_var.name());
    auto grad_hess = grad(hess_c, _mesh.dimension());
    _sixth_order_term = multiply(constant(0.5 * _gamma),
                                  contract(grad_hess, grad_hess));
  }
}

void
FourthOrderCahnHilliardVariational::setupHigherOrderSplitting()
{
  if (_split_var_nums.empty())
  {
    mooseWarning("Fourth-order Cahn-Hilliard requires split variables for gradient components");
    _enable_variable_splitting = true;
  }
}

Real
FourthOrderCahnHilliardVariational::computeQpResidual()
{
  Real residual = VariationalKernelBase::computeQpResidual();

  if (_grad_grad_c_x && _grad_grad_c_y)
  {
    RealVectorValue laplacian_grad_c;
    laplacian_grad_c(0) = (*_grad_grad_c_x)[_qp](0);
    laplacian_grad_c(1) = (*_grad_grad_c_y)[_qp](1);
    if (_mesh.dimension() == 3 && _grad_grad_c_z)
      laplacian_grad_c(2) = (*_grad_grad_c_z)[_qp](2);

    residual += _lambda * laplacian_grad_c * _grad_test[_i][_qp];
  }

  return residual;
}

Real
FourthOrderCahnHilliardVariational::computeQpJacobian()
{
  Real jacobian = VariationalKernelBase::computeQpJacobian();

  return jacobian;
}

InputParameters
AllenCahnVariational::validParams()
{
  InputParameters params = moose::automatic_weak_form::VariationalKernelBase::validParams();

  params.addClassDescription("Allen-Cahn kernel from variational derivative");

  params.addParam<Real>("L", 1.0, "Allen-Cahn mobility");
  params.addParam<Real>("kappa", 1.0, "Gradient energy coefficient");
  params.addParam<MaterialPropertyName>("f_derivative", "dF_dc",
                                          "Free energy derivative material property");
  params.addParam<MaterialPropertyName>("f_second_derivative", "d2F_dc2",
                                          "Free energy second derivative");
  params.addParam<bool>("use_material_properties", false,
                         "Use material properties for free energy");

  return params;
}

AllenCahnVariational::AllenCahnVariational(const InputParameters & parameters)
  : moose::automatic_weak_form::VariationalKernelBase(parameters),
    _L(getParam<Real>("L")),
    _kappa(getParam<Real>("kappa")),
    _f_derivative(getMaterialProperty<Real>("f_derivative")),
    _f_second_derivative(getMaterialProperty<Real>("f_second_derivative")),
    _use_material_properties(getParam<bool>("use_material_properties"))
{
  if (!_use_material_properties)
  {
    using namespace moose::automatic_weak_form;

    auto eta = _builder->field(_var.name());
    auto bulk = _builder->doubleWell(eta);
    auto grad_eta = grad(eta, _mesh.dimension());
    auto gradient = multiply(constant(0.5 * _kappa), dot(grad_eta, grad_eta));

    _energy_density = add(bulk, gradient);
    computeVariationalDerivative();
  }
}

Real
AllenCahnVariational::computeQpResidual()
{
  if (_use_material_properties)
  {
    Real residual = _L * _f_derivative[_qp] * _test[_i][_qp];
    residual -= _L * _kappa * _grad_u[_qp] * _grad_test[_i][_qp];
    return residual;
  }
  else
  {
    return VariationalKernelBase::computeQpResidual();
  }
}

Real
AllenCahnVariational::computeQpJacobian()
{
  if (_use_material_properties)
  {
    Real jacobian = _L * _f_second_derivative[_qp] * _phi[_j][_qp] * _test[_i][_qp];
    jacobian -= _L * _kappa * _grad_phi[_j][_qp] * _grad_test[_i][_qp];
    return jacobian;
  }
  else
  {
    return VariationalKernelBase::computeQpJacobian();
  }
}
