#include "TotalLagrangianStressDivergence.h"

#include "HomogenizationConstraintIntegral.h"

registerMooseObject("TensorMechanicsApp", TotalLagrangianStressDivergence);

InputParameters
TotalLagrangianStressDivergence::validParams()
{
  InputParameters params = Kernel::validParams();

  params.addRequiredParam<unsigned int>("component", "Which direction this kernel acts in");
  params.addRequiredCoupledVar("displacements", "The displacement components");
  params.addParam<bool>("large_kinematics", false, "Use large displacement kinematics");

  // This kernel requires use_displaced_mesh to be off
  params.suppressParameter<bool>("use_displaced_mesh");

  params.addCoupledVar("macro_gradient", "Optional scalar field with the macro gradient");

  params.addParam<MultiMooseEnum>("constraint_types",
                                  HomogenizationConstants::mooseConstraintType,
                                  "Type of each constraint: strain or stress");

  return params;
}

TotalLagrangianStressDivergence::TotalLagrangianStressDivergence(const InputParameters & parameters)
  : DerivativeMaterialInterface<Kernel>(parameters),
    _ld(getParam<bool>("large_kinematics")),
    _component(getParam<unsigned int>("component")),
    _ndisp(coupledComponents("displacements")),
    _disp_nums(_ndisp),
    _disp_vars(_ndisp),
    _pk1(getMaterialPropertyByName<RankTwoTensor>("pk1_stress")),
    _dpk1(getMaterialPropertyByName<RankFourTensor>("pk1_jacobian")),
    _macro_gradient_num(isCoupledScalar("macro_gradient", 0) ? coupledScalar("macro_gradient") : 0),
    _macro_gradient(isCoupledScalar("macro_gradient", 0) ? getScalarVar("macro_gradient", 0)
                                                         : nullptr),
    _indices(HomogenizationConstants::indices.at(_ld)[_ndisp - 1])
{
  // Setup variables
  for (unsigned int i = 0; i < _ndisp; i++)
  {
    _disp_nums[i] = coupled("displacements", i);
    _disp_vars[i] = getVar("displacements", i);
  }

  if (isCoupledScalar("macro_gradient", 0))
  {
    // Check the order of the scalar variable
    unsigned int needed = HomogenizationConstants::required.at(_ld)[_ndisp - 1];
    if (_macro_gradient->order() != needed)
      mooseError("The homogenization macro gradient variable must have order ", needed);

    // Check the number of constraints
    auto types = getParam<MultiMooseEnum>("constraint_types");
    if (types.size() != needed)
      mooseError("The kernel must be supplied ", needed, " constraint types.");

    // Setup the types of constraints
    if (isCoupledScalar("macro_gradient", 0))
    {
      auto types = getParam<MultiMooseEnum>("constraint_types");
      for (unsigned int i = 0; i < types.size(); i++)
        _ctypes.push_back(static_cast<HomogenizationConstants::ConstraintType>(types.get(i)));
    }
  }
}

Real
TotalLagrangianStressDivergence::computeQpResidual()
{
  return _pk1[_qp].row(_component) * _grad_test[_i][_qp];
}

Real
TotalLagrangianStressDivergence::computeQpJacobian()
{
  Real value = 0.0;
  value += materialJacobian(_component, _component, _grad_test[_i][_qp], _grad_phi[_j][_qp]);

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
      value +=
          materialJacobian(_component, cc, _grad_test[_i][_qp], _disp_vars[cc]->gradPhi()[_j][_qp]);
      break;
    }
  }

  return value;
}

Real
TotalLagrangianStressDivergence::materialJacobian(unsigned int i,
                                                  unsigned int k,
                                                  const RealGradient & grad_phi,
                                                  const RealGradient & grad_psi)
{
  Real value = 0.0;
  for (unsigned int j = 0; j < _ndisp; j++)
    for (unsigned int l = 0; l < _ndisp; l++)
      value += _dpk1[_qp](i, j, k, l) * grad_phi(j) * grad_psi(l);

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
  return materialBaseJacobian();
}

Real
TotalLagrangianStressDivergence::materialBaseJacobian()
{
  Real value = 0.0;
  for (unsigned int j = 0; j < _ndisp; j++)
    value +=
        _dpk1[_qp](_component, j, _indices[_h].first, _indices[_h].second) * _grad_test[_i][_qp](j);

  return value;
}

Real
TotalLagrangianStressDivergence::computeConstraintJacobian()
{
  if (_ctypes[_h] == HomogenizationConstants::ConstraintType::Stress)
  {
    return materialConstraintJacobianStress();
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
TotalLagrangianStressDivergence::materialConstraintJacobianStress()
{
  Real value = 0.0;
  for (unsigned int l = 0; l < _ndisp; l++)
    value +=
        _dpk1[_qp](_indices[_h].first, _indices[_h].second, _component, l) * _grad_phi[_i][_qp](l);
  return value;
}
