/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CoefCoupledTimeDerivative.h"

template<>
InputParameters validParams<CoefCoupledTimeDerivative>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("Time derivative Kernel that acts on a coupled variable");
  params.addRequiredCoupledVar("v", "Coupled variable");
  params.addParam<Real>("coef", 0.0, "Coefficient"); 
  return params;
}

CoefCoupledTimeDerivative::CoefCoupledTimeDerivative(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters),
    _coef(getParam<Real>("coef")),
    _v_dot(coupledDot("v")),
    _dv_dot(coupledDotDu("v")),
    _v_var(coupled("v"))
{}

Real
CoefCoupledTimeDerivative::computeQpResidual()
{
  return _test[_i][_qp] *_coef* _v_dot[_qp];
}

Real
CoefCoupledTimeDerivative::computeQpJacobian()
{
  return 0.0;
}

Real
CoefCoupledTimeDerivative::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _v_var)
    return _test[_i][_qp] *_coef* _phi[_j][_qp] * _dv_dot[_qp];

  return 0.0;
}
