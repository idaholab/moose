/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "StressDivergenceRZ.h"

#include "Material.h"
#include "SymmElasticityTensor.h"

template<>
InputParameters validParams<StressDivergenceRZ>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<unsigned int>("component", "An integer corresponding to the direction the variable this kernel acts in. (0 for r, 1 for z)");
  params.addCoupledVar("disp_r", "The r displacement");
  params.addCoupledVar("disp_z", "The z displacement");
  params.addCoupledVar("temp", "The temperature");

  params.set<bool>("use_displaced_mesh") = true;

  return params;
}


StressDivergenceRZ::StressDivergenceRZ(const std::string & name, InputParameters parameters)
  :Kernel(name, parameters),
   _stress(getMaterialProperty<SymmTensor>("stress")),
   _Jacobian_mult(getMaterialProperty<SymmElasticityTensor>("Jacobian_mult")),
   _d_stress_dT(getMaterialProperty<SymmTensor>("d_stress_dT")),
   _component(getParam<unsigned int>("component")),
   _rdisp_coupled(isCoupled("disp_r")),
   _zdisp_coupled(isCoupled("disp_z")),
   _temp_coupled(isCoupled("temp")),
   _rdisp_var(_rdisp_coupled ? coupled("disp_r") : 0),
   _zdisp_var(_zdisp_coupled ? coupled("disp_z") : 0),
   _temp_var(_temp_coupled ? coupled("temp") : 0)
{}

Real
StressDivergenceRZ::computeQpResidual()
{
  Real div(0);
  if (_component == 0)
  {
    div =
      _grad_test[_i][_qp](0)            * _stress[_qp].xx() +
//    0                                 * _stress[_qp].yy() +
    + _test[_i][_qp] / _q_point[_qp](0) * _stress[_qp].zz() +
    + _grad_test[_i][_qp](1)            * _stress[_qp].xy();
  }
  else if (_component == 1)
  {
    div =
//    0                      * _stress[_qp].xx() +
      _grad_test[_i][_qp](1) * _stress[_qp].yy() +
//    0                      * _stress[_qp].zz() +
      _grad_test[_i][_qp](0) * _stress[_qp].xy();

  }
  else
  {
    mooseError("Invalid component");
  }

  return div;
}

Real
StressDivergenceRZ::computeQpJacobian()
{
  return calculateJacobian( _component, _component );
}

Real
StressDivergenceRZ::calculateJacobian( unsigned int ivar, unsigned int jvar )
{
  SymmTensor test, phi;
  if (ivar == 0)
  {
    test.xx() = _grad_test[_i][_qp](0);
    test.xy() = 0.5*_grad_test[_i][_qp](1);
    test.zz() = _test[_i][_qp] / _q_point[_qp](0);
  }
  else
  {
    test.xy() = 0.5*_grad_test[_i][_qp](0);
    test.yy() = _grad_test[_i][_qp](1);
  }
  if (jvar == 0)
  {
    phi.xx()  = _grad_phi[_j][_qp](0);
    phi.xy()  = 0.5*_grad_phi[_j][_qp](1);
    phi.zz()  = _phi[_j][_qp] / _q_point[_qp](0);
  }
  else
  {
    phi.xy()  = 0.5*_grad_phi[_j][_qp](0);
    phi.yy()  = _grad_phi[_j][_qp](1);
  }

  SymmTensor tmp( _Jacobian_mult[_qp] * phi );
  const Real val( test.doubleContraction( tmp ) );

  return val;
}

Real
StressDivergenceRZ::computeQpOffDiagJacobian(unsigned int jvar)
{

  if ( _rdisp_coupled && jvar == _rdisp_var )
  {
    return calculateJacobian( _component, 0 );
  }
  else if ( _zdisp_coupled && jvar == _zdisp_var )
  {
    return calculateJacobian( _component, 1 );
  }
  else if ( _temp_coupled && jvar == _temp_var )
  {
    SymmTensor test;
    if (_component == 0)
    {
      test.xx() = _grad_test[_i][_qp](0);
      test.xy() = 0.5*_grad_test[_i][_qp](1);
      test.zz() = _test[_i][_qp] / _q_point[_qp](0);
    }
    else
    {
      test.xy() = 0.5*_grad_test[_i][_qp](0);
      test.yy() = _grad_test[_i][_qp](1);
    }
    return _d_stress_dT[_qp].doubleContraction(test) * _phi[_j][_qp];
  }

  return 0;
}
