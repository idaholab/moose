//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DotCouplingAux.h"

registerMooseObject("MooseTestApp", DotCouplingAux);
registerMooseObject("MooseTestApp", ADDotCouplingAux);

template <bool is_ad>
InputParameters
DotCouplingAuxTempl<is_ad>::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredCoupledVar("v", "Coupled variable");

  return params;
}

template <bool is_ad>
DotCouplingAuxTempl<is_ad>::DotCouplingAuxTempl(const InputParameters & parameters)
  : AuxKernel(parameters), _v_dot(coupledGenericDot<is_ad>("v"))
{
}

template <bool is_ad>
Real
DotCouplingAuxTempl<is_ad>::computeValue()
{
  return MetaPhysicL::raw_value(_v_dot[_qp]);
}

template class DotCouplingAuxTempl<false>;
template class DotCouplingAuxTempl<true>;
