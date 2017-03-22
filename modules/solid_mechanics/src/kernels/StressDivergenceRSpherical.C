/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "StressDivergenceRSpherical.h"

#include "Material.h"
#include "SymmElasticityTensor.h"

template <>
InputParameters
validParams<StressDivergenceRSpherical>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<unsigned int>("component",
                                        "An integer corresponding to the direction "
                                        "the variable this kernel acts in. (0 for r, "
                                        "1 for z)");
  params.addCoupledVar("disp_r", "The r displacement");
  //  params.addCoupledVar("disp_z", "The z displacement");
  params.addCoupledVar("temp", "The temperature");

  params.set<bool>("use_displaced_mesh") = true;

  return params;
}

StressDivergenceRSpherical::StressDivergenceRSpherical(const InputParameters & parameters)
  : Kernel(parameters),
    _stress(getMaterialProperty<SymmTensor>("stress")),
    _Jacobian_mult(getMaterialProperty<SymmElasticityTensor>("Jacobian_mult")),
    _d_stress_dT(getMaterialProperty<SymmTensor>("d_stress_dT")),
    _component(getParam<unsigned int>("component")),
    _temp_coupled(isCoupled("temp")),
    _temp_var(_temp_coupled ? coupled("temp") : 0)
{
}

Real
StressDivergenceRSpherical::computeQpResidual()
{
  Real div(0);
  if (_component == 0)
  {
    div = _grad_test[_i][_qp](0) * _stress[_qp].xx() +
          _test[_i][_qp] / _q_point[_qp](0) * _stress[_qp].yy() +
          _test[_i][_qp] / _q_point[_qp](0) * _stress[_qp].zz();
  }
  return div;
}

Real
StressDivergenceRSpherical::computeQpJacobian()
{
  Real val(1);
  if (_component == 0)
  {
    SymmTensor test, phi;
    test.xx() = _grad_test[_i][_qp](0);
    test.yy() = _test[_i][_qp] / _q_point[_qp](0);
    test.zz() = test.yy();
    phi.xx() = _grad_phi[_j][_qp](0);
    phi.yy() = _phi[_j][_qp] / _q_point[_qp](0);
    phi.zz() = phi.yy();

    SymmTensor tmp(_Jacobian_mult[_qp] * phi);
    val = test.doubleContraction(tmp);
  }
  else if (_i != _j)
  {
    val = 0;
  }
  return val;
}

Real
StressDivergenceRSpherical::computeQpOffDiagJacobian(unsigned int jvar)
{

  if (_temp_coupled && jvar == _temp_var)
  {
    SymmTensor test;
    test.xx() = _grad_test[_i][_qp](0);
    test.yy() = _test[_i][_qp] / _q_point[_qp](0);
    test.zz() = test.yy();
    return _d_stress_dT[_qp].doubleContraction(test) * _phi[_j][_qp];
  }

  return 0;
}
