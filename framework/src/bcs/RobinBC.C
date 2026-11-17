//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RobinBC.h"

registerMooseObject("MooseApp", RobinBC);
registerMooseObject("MooseApp", ADRobinBC);

template <bool is_ad>
InputParameters
RobinBCTempl<is_ad>::validParams()
{
  InputParameters params = GenericIntegratedBC<is_ad>::validParams();
  params.addClassDescription("Imposes the Robin integrated boundary condition "
                             "$\\frac{\\partial u}{\\partial n}=u$.");
  params.addParam<Real>(
      "value",
      0.0,
      "Inhomogeneous value subtracted from the variable for Robin boundary condition.");
  params.declareControllable("value");
  params.addParam<Real>(
      "coefficient", 1.0, "Coefficient multiplier for the Robin boundary condition term.");
  return params;
}

template <bool is_ad>
RobinBCTempl<is_ad>::RobinBCTempl(const InputParameters & parameters)
  : GenericIntegratedBC<is_ad>(parameters),
    _value(this->template getParam<Real>("value")),
    _coef(this->template getParam<Real>("coefficient"))
{
}

template <bool is_ad>
GenericReal<is_ad>
RobinBCTempl<is_ad>::computeQpResidual()
{
  return _coef * _test[_i][_qp] * (_u[_qp] - _value);
}

template <bool is_ad>
Real
RobinBCTempl<is_ad>::computeQpJacobian()
{
  return _coef * _test[_i][_qp] * _phi[_j][_qp];
}

template class RobinBCTempl<false>;
template class RobinBCTempl<true>;
