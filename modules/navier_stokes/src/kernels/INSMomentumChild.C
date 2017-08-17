/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "INSMomentumChild.h"

template <>
InputParameters
validParams<INSMomentumChild>()
{
  InputParameters params = validParams<INSBase>();

  params.addClassDescription(
      "This class computes the PSPG stabilization components for the incompressibility equation.");
  params.addRequiredParam<unsigned>("component", "The velocity component that this is applied to.");
  return params;
}

INSMomentumChild::INSMomentumChild(const InputParameters & parameters)
  : INSBase(parameters), _component(getParam<unsigned>("component"))
{
}

Real
INSMomentumChild::computeQpResidual()
{
  return _test[_i][_qp] * (computeStrongConvectiveTerm() + computeStrongPressureTerm() +
                           computeStrongGravityTerm())(_component) +
         _grad_test[_i][_qp] * weakViscousTerm(_component);
  // return _test[_i][_qp] * (computeStrongConvectiveTerm() + computeStrongViscousTerm() +
  //                          computeStrongPressureTerm() + computeStrongGravityTerm())(_component);
}

Real
INSMomentumChild::computeQpJacobian()
{
  return _test[_i][_qp] * dConvecDUComp(_component)(_component) +
         _grad_test[_i][_qp] * dWeakViscDUComp();
  // return _test[_i][_qp] * (dConvecDUComp(_component) + dViscDUComp(_component))(_component);
}

Real
INSMomentumChild::computeQpOffDiagJacobian(unsigned jvar)
{
  if (jvar == _u_vel_var_number)
    return _test[_i][_qp] * dConvecDUComp(0)(_component) + _grad_test[_i][_qp] * dWeakViscDUComp();
  // return _test[_i][_qp] * (dConvecDUComp(0) + dViscDUComp(0))(_component);

  else if (jvar == _v_vel_var_number)
    return _test[_i][_qp] * dConvecDUComp(1)(_component) + _grad_test[_i][_qp] * dWeakViscDUComp();
  // return _test[_i][_qp] * (dConvecDUComp(1) + dViscDUComp(1))(_component);

  else if (jvar == _w_vel_var_number)
    return _test[_i][_qp] * dConvecDUComp(2)(_component) + _grad_test[_i][_qp] * dWeakViscDUComp();
  // return _test[_i][_qp] * (dConvecDUComp(2) + dViscDUComp(2))(_component);

  else if (jvar == _p_var_number)
    return _test[_i][_qp] * dPressureDPressure()(_component);

  else
    return 0;
}
