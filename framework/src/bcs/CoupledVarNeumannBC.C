//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledVarNeumannBC.h"

registerMooseObject("MooseApp", CoupledVarNeumannBC);
registerMooseObject("MooseApp", ADCoupledVarNeumannBC);

template <bool is_ad>
InputParameters
CoupledVarNeumannBCTempl<is_ad>::validParams()
{
  InputParameters params = IntegratedBCParent<is_ad>::validParams();
  params.addRequiredCoupledVar("v", "Coupled variable setting the gradient on the boundary.");
  params.addCoupledVar("scale_factor", 1., "Scale factor to multiply the heat flux with");
  params.addParam<Real>(
      "coef", 1.0, "Coefficent ($\\sigma$) multiplier for the coupled force term.");
  params.addClassDescription("Imposes the integrated boundary condition "
                             "$\\frac{\\partial u}{\\partial n}=v$, "
                             "where $v$ is a variable.");
  return params;
}
template <bool is_ad>
CoupledVarNeumannBCTempl<is_ad>::CoupledVarNeumannBCTempl(const InputParameters & parameters)
  : IntegratedBCParent<is_ad>(parameters),
    _coupled_var(this->template coupledGenericValue<is_ad>("v")),
    _coupled_num(this->coupled("v")),
    _coef(this->template getParam<Real>("coef")),
    _scale_factor(this->template coupledGenericValue<is_ad>("scale_factor"))
{
}

template <bool is_ad>
GenericReal<is_ad>
CoupledVarNeumannBCTempl<is_ad>::computeQpResidual()
{
  return -_scale_factor[_qp] * _coef * _test[_i][_qp] * _coupled_var[_qp];
}

Real
CoupledVarNeumannBC::computeQpOffDiagJacobian(const unsigned int jvar)
{
  if (jvar == _coupled_num)
    return -_scale_factor[_qp] * _coef * _test[_i][_qp] * _phi[_j][_qp];
  else
    return 0;
}

template class CoupledVarNeumannBCTempl<false>;
template class CoupledVarNeumannBCTempl<true>;
