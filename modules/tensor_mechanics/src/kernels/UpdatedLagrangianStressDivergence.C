#include "UpdatedLagrangianStressDivergence.h"

registerMooseObject("TensorMechanicsApp", UpdatedLagrangianStressDivergence);

InputParameters
UpdatedLagrangianStressDivergence::validParams()
{
  InputParameters params = Kernel::validParams();

  params.addRequiredParam<unsigned int>("component", "Which direction this kernel acts in");
  params.addRequiredCoupledVar("displacements", "The displacement components");

  // This kernel requires use_displaced_mesh to match large_kinematics
  params.addParam<bool>("large_kinematics", false, "Use large displacement kinematics");

  return params;
}

UpdatedLagrangianStressDivergence::UpdatedLagrangianStressDivergence(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<Kernel>(parameters),
    _ld(getParam<bool>("large_kinematics")),
    _component(getParam<unsigned int>("component")),
    _ndisp(coupledComponents("displacements")),
    _disp_nums(_ndisp),
    _disp_vars(_ndisp),
    _stress(getMaterialPropertyByName<RankTwoTensor>("stress")),
    _material_jacobian(getMaterialPropertyByName<RankFourTensor>("cauchy_jacobian")),
    _df(getMaterialPropertyByName<RankTwoTensor>("inv_inc_def_grad"))

{
  // This kernel requires used_displaced_mesh to be true if large kinematics
  // is on
  if (_ld && (!getParam<bool>("use_displaced_mesh")))
    mooseError("The UpdatedLagrangianStressDivergence kernel requires "
               "used_displaced_mesh = true for large_kinematics = true");

  // Similarly, if large kinematics is off so should use_displaced_mesh
  if (!_ld && (getParam<bool>("use_displaced_mesh")))
    mooseError("The UpdatedLagrangianStressDivergence kernel requires "
               "used_displaced_mesh = false for large_kinematics = false");

  // Do the vector coupling of the displacements
  for (unsigned int i = 0; i < _ndisp; i++)
  {
    _disp_nums[i] = coupled("displacements", i);
    _disp_vars[i] = getVar("displacements", i);
  }
}

Real
UpdatedLagrangianStressDivergence::computeQpResidual()
{
  return _stress[_qp].row(_component) * _grad_test[_i][_qp];
}

Real
UpdatedLagrangianStressDivergence::computeQpJacobian()
{
  Real value = 0.0;

  value += matJacobianComponent(_material_jacobian[_qp],
                                _component,
                                _component,
                                _grad_test[_i][_qp],
                                _grad_phi[_j][_qp],
                                _df[_qp]);

  if (_ld)
  {
    value += geomJacobianComponent(
        _component, _component, _grad_test[_i][_qp], _grad_phi[_j][_qp], _stress[_qp]);
  }

  return value;
}

Real
UpdatedLagrangianStressDivergence::computeQpOffDiagJacobian(unsigned int jvar)
{
  Real value = 0.0;

  for (unsigned int cc = 0; cc < _ndisp; cc++)
  {
    if (jvar == _disp_nums[cc])
    {
      value += matJacobianComponent(_material_jacobian[_qp],
                                    _component,
                                    cc,
                                    _grad_test[_i][_qp],
                                    _disp_vars[cc]->gradPhi()[_j][_qp],
                                    _df[_qp]);
      if (_ld)
      {
        value += geomJacobianComponent(
            _component, cc, _grad_test[_i][_qp], _disp_vars[cc]->gradPhi()[_j][_qp], _stress[_qp]);
      }
      break;
    }
  }

  return value;
}

Real
UpdatedLagrangianStressDivergence::matJacobianComponent(const RankFourTensor & C,
                                                        unsigned int i,
                                                        unsigned int m,
                                                        const RealGradient & grad_psi,
                                                        const RealGradient & grad_phi,
                                                        const RankTwoTensor & df)
{
  Real value = 0.0;

  for (unsigned int j = 0; j < _ndisp; j++)
  {
    for (unsigned int k = 0; k < _ndisp; k++)
    {
      for (unsigned int l = 0; l < _ndisp; l++)
      {
        value += C(i, j, k, l) * grad_psi(j) * df(k, m) * grad_phi(l);
      }
    }
  }

  return value;
}

Real
UpdatedLagrangianStressDivergence::geomJacobianComponent(unsigned int i,
                                                         unsigned int m,
                                                         const RealGradient & grad_psi,
                                                         const RealGradient & grad_phi,
                                                         const RankTwoTensor & stress)
{
  Real value = 0.0;

  for (unsigned int k = 0; k < _ndisp; k++)
  {
    value += stress(i, k) * grad_psi(k) * grad_phi(m);
    value -= stress(i, k) * grad_phi(k) * grad_psi(m);
  }

  return value;
}
