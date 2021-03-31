#include "TotalLagrangianStressDivergence.h"

#include "HomogenizationConstraintIntegral.h"

registerMooseObject("TensorMechanicsApp", TotalLagrangianStressDivergence);

InputParameters
TotalLagrangianStressDivergence::validParams()
{
  InputParameters params = Kernel::validParams();

  params.addRequiredParam<unsigned int>("component", "Which direction this kernel acts in");
  params.addRequiredCoupledVar("displacements", "The displacement components");
  params.addParam<bool>("kernel_large_kinematics", false, "Use large displacement kinematics");

  // This kernel requires use_displaced_mesh to be off
  params.suppressParameter<bool>("use_displaced_mesh");

  params.addCoupledVar("macro_gradient", "Optional scalar field with the macro gradient");
  params.addParam<std::vector<std::string>>("constraint_types",
                                            "Type of each constraint: strain or stress");

  return params;
}

TotalLagrangianStressDivergence::TotalLagrangianStressDivergence(const InputParameters & parameters)
  : DerivativeMaterialInterface<Kernel>(parameters),
    _ld(getParam<bool>("kernel_large_kinematics")),
    _component(getParam<unsigned int>("component")),
    _ndisp(coupledComponents("displacements")),
    _disp_nums(_ndisp),
    _disp_vars(_ndisp),
    _grad_disp(_ndisp),
    _stress(getMaterialPropertyByName<RankTwoTensor>("cauchy_stress")),
    _material_jacobian(getMaterialPropertyByName<RankFourTensor>("material_jacobian")),
    _inv_def_grad(getMaterialPropertyByName<RankTwoTensor>("inv_def_grad")),
    _detJ(getMaterialPropertyByName<Real>("detJ")),
    _df(getMaterialPropertyByName<RankTwoTensor>("inc_def_grad")),
    _macro_gradient_num(isCoupledScalar("macro_gradient", 0) ? coupledScalar("macro_gradient") : 0),
    _macro_gradient(isCoupledScalar("macro_gradient", 0) ? getScalarVar("macro_gradient", 0)
                                                         : nullptr),
    _indices(HomogenizationConstants::indices.at(_ld)[_ndisp - 1])
{
  if (isCoupledScalar("macro_gradient", 0))
  {
    // Check the order of the scalar variable
    unsigned int needed = HomogenizationConstants::required.at(_ld)[_ndisp - 1];
    if (_macro_gradient->order() != needed)
      mooseError("The homogenization macro gradient variable must have order ", needed);

    // Check the number of constraints
    auto types = getParam<std::vector<std::string>>("constraint_types");
    if (types.size() != needed)
      mooseError("The kernel must be supplied ", needed, " constraint types.");
  }
}

void
TotalLagrangianStressDivergence::initialSetup()
{
  for (unsigned int i = 0; i < _ndisp; i++)
  {
    _disp_nums[i] = coupled("displacements", i);
    _disp_vars[i] = getVar("displacements", i);
    _grad_disp[i] = &coupledGradient("displacements", i);
  }

  if (isCoupledScalar("macro_gradient", 0))
  {
    auto types = getParam<std::vector<std::string>>("constraint_types");
    for (unsigned int i = 0; i < types.size(); i++)
      _ctypes.push_back(HomogenizationConstants::map_string(types[i]));
  }
}

Real
TotalLagrangianStressDivergence::computeQpResidual()
{
  if (_ld)
    return largeDeformationResidual(_grad_test[_i][_qp]);
  else
    return smallDeformationResidual(_grad_test[_i][_qp]);
}

Real
TotalLagrangianStressDivergence::largeDeformationResidual(const RealGradient & grad_phi)
{
  return _detJ[_qp] * _stress[_qp].row(_component) * (_inv_def_grad[_qp].transpose() * grad_phi);
}

Real
TotalLagrangianStressDivergence::smallDeformationResidual(const RealGradient & grad_phi)
{
  return _stress[_qp].row(_component) * grad_phi;
}

Real
TotalLagrangianStressDivergence::computeQpJacobian()
{
  Real value = 0.0;

  if (_ld)
  {
    value +=
        largeDeformationMatJac(_component, _component, _grad_test[_i][_qp], _grad_phi[_j][_qp]);
    value +=
        largeDeformationGeoJac(_component, _component, _grad_test[_i][_qp], _grad_phi[_j][_qp]);
  }
  else
  {
    value +=
        smallDeformationMatJac(_component, _component, _grad_test[_i][_qp], _grad_phi[_j][_qp]);
  }

  return value;
}

Real
TotalLagrangianStressDivergence::computeQpOffDiagJacobian(unsigned int jvar)
{
  Real value = 0.0;

  for (unsigned int cc = 0; cc < _ndisp; cc++)
  {
    if (jvar == _disp_nums[cc])
    {
      if (_ld)
      {
        value += largeDeformationMatJac(
            _component, cc, _grad_test[_i][_qp], _disp_vars[cc]->gradPhi()[_j][_qp]);
        value += largeDeformationGeoJac(
            _component, cc, _grad_test[_i][_qp], _disp_vars[cc]->gradPhi()[_j][_qp]);
      }
      else
      {
        value += smallDeformationMatJac(
            _component, cc, _grad_test[_i][_qp], _disp_vars[cc]->gradPhi()[_j][_qp]);
      }
      break;
    }
  }

  return value;
}

Real
TotalLagrangianStressDivergence::smallDeformationMatJac(unsigned int i,
                                                        unsigned int k,
                                                        const RealGradient & grad_phi,
                                                        const RealGradient & grad_psi)
{
  Real value = 0.0;
  for (unsigned int j = 0; j < _ndisp; j++)
    for (unsigned int l = 0; l < _ndisp; l++)
      value += _material_jacobian[_qp](i, j, k, l) * grad_phi(j) * grad_psi(l);

  return value;
}

Real
TotalLagrangianStressDivergence::largeDeformationMatJac(unsigned int i,
                                                        unsigned int k,
                                                        const RealGradient & grad_phi,
                                                        const RealGradient & grad_psi)
{
  auto GPhi = _inv_def_grad[_qp].transpose() * grad_phi;
  auto GPsi = _inv_def_grad[_qp].transpose() * grad_psi;

  Real value = 0.0;

  for (unsigned int j = 0; j < _ndisp; j++)
    for (unsigned int m = 0; m < _ndisp; m++)
      for (unsigned int n = 0; n < _ndisp; n++)
        value +=
            _detJ[_qp] * _material_jacobian[_qp](i, j, m, n) * _df[_qp](m, k) * GPsi(n) * GPhi(j);

  return value;
}

Real
TotalLagrangianStressDivergence::largeDeformationGeoJac(unsigned int i,
                                                        unsigned int k,
                                                        const RealGradient & grad_phi,
                                                        const RealGradient & grad_psi)
{
  auto GPhi = _inv_def_grad[_qp].transpose() * grad_phi;
  auto GPsi = _inv_def_grad[_qp].transpose() * grad_psi;

  Real value = 0.0;

  for (unsigned int j = 0; j < _ndisp; j++)
    value += _detJ[_qp] * _stress[_qp](i, j) * (GPsi(k) * GPhi(j) - GPsi(j) * GPhi(k));

  return value;
}

void
TotalLagrangianStressDivergence::computeOffDiagJacobianScalar(unsigned int jvar)
{
  if (jvar == _macro_gradient_num)
  {
    DenseMatrix<Number> & ken = _assembly.jacobianBlock(_var.number(), jvar);
    DenseMatrix<Number> & kne = _assembly.jacobianBlock(jvar, _var.number());

    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      Real dV = _JxW[_qp] * _coord[_qp];
      for (_h = 0; _h < _macro_gradient->order(); _h++)
      {
        for (_i = 0; _i < _test.size(); _i++)
        { // This assumes Galerkin, i.e. the test and trial functions are the
          // same
          ken(_i, _h) += computeBaseJacobian() * dV;
          kne(_h, _i) += computeConstraintJacobian() * dV;
        }
      }
    }
  }
}

Real
TotalLagrangianStressDivergence::computeBaseJacobian()
{
  if (_ld)
    return ldBaseJacobian();
  else
    return sdBaseJacobian();
}

Real
TotalLagrangianStressDivergence::sdBaseJacobian()
{
  Real value = 0.0;
  for (unsigned int j = 0; j < _ndisp; j++)
    value += _material_jacobian[_qp](_component, j, _indices[_h].first, _indices[_h].second) *
             _grad_test[_i][_qp](j);

  return value;
}

Real
TotalLagrangianStressDivergence::ldBaseJacobian()
{
  unsigned int i = _component;
  unsigned int a = _indices[_h].first;
  unsigned int b = _indices[_h].second;
  Real value = 0.0;
  for (unsigned int j = 0; j < _ndisp; j++)
  {
    for (unsigned int k = 0; k < _ndisp; k++)
    {
      value += _detJ[_qp] * _stress[_qp](i, j) * _grad_test[_i][_qp](k) *
               (_inv_def_grad[_qp](b, a) * _inv_def_grad[_qp](k, j) -
                _inv_def_grad[_qp](k, a) * _inv_def_grad[_qp](b, j));

      for (unsigned int m = 0; m < _ndisp; m++)
      {
        for (unsigned int n = 0; n < _ndisp; n++)
        {
          value += _detJ[_qp] * _material_jacobian[_qp](i, j, m, n) * _df[_qp](m, a) *
                   _inv_def_grad[_qp](b, n) * _grad_test[_i][_qp](k) * _inv_def_grad[_qp](k, j);
        }
      }
    }
  }
  return value;
}

Real
TotalLagrangianStressDivergence::computeConstraintJacobian()
{
  if (_ctypes[_h] == HomogenizationConstants::ConstraintType::Stress)
  {
    if (_ld)
      return ldConstraintJacobianStress();
    else
      return sdConstraintJacobianStress();
  }
  else if (_ctypes[_h] == HomogenizationConstants::ConstraintType::Strain)
  {
    if (_ld)
      return ldConstraintJacobianStrain();
    else
      return sdConstraintJacobianStrain();
  }
  else
  {
    mooseError("Unknown constraint type in kernel calculation!");
  }
}

Real
TotalLagrangianStressDivergence::sdConstraintJacobianStrain()
{
  Real value = 0.0;
  if (_indices[_h].first == _component)
    value += 0.5 * _grad_phi[_i][_qp](_indices[_h].second);
  if (_indices[_h].second == _component)
    value += 0.5 * _grad_phi[_i][_qp](_indices[_h].first);
  return value;
}

Real
TotalLagrangianStressDivergence::sdConstraintJacobianStress()
{
  Real value = 0.0;
  for (unsigned int l = 0; l < _ndisp; l++)
    value += _material_jacobian[_qp](_indices[_h].first, _indices[_h].second, _component, l) *
             _grad_phi[_i][_qp](l);
  return value;
}

Real
TotalLagrangianStressDivergence::ldConstraintJacobianStrain()
{
  unsigned int i = _indices[_h].first;
  unsigned int j = _indices[_h].second;
  if (i == _component)
    return _grad_phi[_i][_qp](j);
  else
    return 0;
}

Real
TotalLagrangianStressDivergence::ldConstraintJacobianStress()
{
  unsigned int i = _indices[_h].first;
  unsigned int j = _indices[_h].second;

  unsigned int a = _component;

  Real value = 0.0;

  for (unsigned int k = 0; k < _ndisp; k++)
  {
    for (unsigned int b = 0; b < _ndisp; b++)
    {
      value += _detJ[_qp] * _stress[_qp](i, k) *
               (_inv_def_grad[_qp](b, a) * _inv_def_grad[_qp](j, k) -
                _inv_def_grad[_qp](j, a) * _inv_def_grad[_qp](b, k)) *
               _grad_phi[_i][_qp](b);
      for (unsigned int s = 0; s < _ndisp; s++)
      {
        for (unsigned int t = 0; t < _ndisp; t++)
        {
          value += _detJ[_qp] * _material_jacobian[_qp](i, k, s, t) * _df[_qp](s, a) *
                   _inv_def_grad[_qp](b, t) * _inv_def_grad[_qp](j, k) * _grad_phi[_i][_qp](b);
        }
      }
    }
  }

  return value;
}
