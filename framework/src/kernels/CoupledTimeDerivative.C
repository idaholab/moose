//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledTimeDerivative.h"

registerMooseObject("MooseApp", CoupledTimeDerivative);
registerMooseObject("MooseApp", VectorCoupledTimeDerivative);

template <typename K>
InputParameters
CoupledTimeDerivativeTempl<K>::validParams()
{
  InputParameters params = K::validParams();
  if (std::is_same_v<K, Kernel>)
    params.addClassDescription("Time derivative Kernel that acts on a coupled variable. "
                               "Weak form: $(\\psi_i, k\\frac{\\partial v_h}{\\partial t})$.");
  else if (std::is_same_v<K, VectorKernel>)
    params.addClassDescription("Time derivative Kernel that acts on a coupled vector variable. "
                               "Weak form: "
                               "$(\\vec{\\psi}_i, k\\frac{\\partial \\vec{v_h}}{\\partial t})$.");
  params.addRequiredCoupledVar("v", "Coupled variable");
  params.addParam<Real>("coeff", 1.0, "The constant coefficient");
  return params;
}

template <>
CoupledTimeDerivativeTempl<Kernel>::CoupledTimeDerivativeTempl(const InputParameters & parameters)
  : Kernel(parameters),
    _v_dot(coupledDot("v")),
    _dv_dot(coupledDotDu("v")),
    _v_var(coupled("v")),
    _coeff(getParam<Real>("coeff"))
{
}

template <>
CoupledTimeDerivativeTempl<VectorKernel>::CoupledTimeDerivativeTempl(
    const InputParameters & parameters)
  : VectorKernel(parameters),
    _v_dot(coupledVectorDot("v")),
    _dv_dot(coupledVectorDotDu("v")),
    _v_var(coupled("v")),
    _coeff(getParam<Real>("coeff"))
{
}

template <typename K>
Real
CoupledTimeDerivativeTempl<K>::computeQpResidual()
{
  return _coeff * K::_test[K::_i][K::_qp] * _v_dot[K::_qp];
}

template <typename K>
Real
CoupledTimeDerivativeTempl<K>::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _v_var)
    return _coeff * K::_test[K::_i][K::_qp] * K::_phi[K::_j][K::_qp] * _dv_dot[K::_qp];

  return 0.0;
}

template class CoupledTimeDerivativeTempl<Kernel>;
template class CoupledTimeDerivativeTempl<VectorKernel>;
