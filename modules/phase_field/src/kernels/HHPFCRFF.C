/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "HHPFCRFF.h"

template <>
InputParameters
validParams<HHPFCRFF>()
{
  InputParameters params = validParams<KernelValue>();
  params.addCoupledVar("coupled_var",
                       "The name of the coupled variable, if one is used in the kernel");
  params.addRequiredParam<MaterialPropertyName>(
      "prop_name", "Name of material property to be used in the kernel");
  params.addRequiredParam<bool>(
      "positive", "If the kernel is positive, this is true, if negative, it is false");
  return params;
}

HHPFCRFF::HHPFCRFF(const InputParameters & parameters)
  : KernelValue(parameters),
    _kernel_sign(getParam<bool>("positive") ? 1.0 : -1.0),
    _prop(getMaterialProperty<Real>("prop_name")),
    _has_coupled_var(isCoupled("coupled_var")),
    _coupled_var(_has_coupled_var ? &coupledValue("coupled_var") : NULL),
    _coupled_var_var(_has_coupled_var ? coupled("coupled_var") : 0)
{
}

Real
HHPFCRFF::precomputeQpResidual()
{
  // Assign value of the variable, whether coupled or not
  Real var;
  if (_has_coupled_var)
    var = (*_coupled_var)[_qp];
  else
    var = _u[_qp];

  return _kernel_sign * _prop[_qp] * var;
}

Real
HHPFCRFF::precomputeQpJacobian()
{
  if (_has_coupled_var)
    return 0.0;

  return _kernel_sign * _prop[_qp] * _phi[_j][_qp];
}

Real
HHPFCRFF::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (_has_coupled_var && jvar == _coupled_var_var)
    return _kernel_sign * _prop[_qp] * _phi[_j][_qp] * _test[_i][_qp];

  return 0.0;
}
