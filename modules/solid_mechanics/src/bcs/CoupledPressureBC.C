//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledPressureBC.h"

registerMooseObject("SolidMechanicsApp", CoupledPressureBC);

template <bool is_ad>
InputParameters
CoupledPressureBCTempl<is_ad>::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addClassDescription(
      "Applies a pressure from a variable on a given boundary in a given direction");
  params.addRequiredCoupledVar("pressure", "Coupled variable containing the pressure");
  return params;
}

template <bool is_ad>
CoupledPressureBCTempl<is_ad>::CoupledPressureBCTempl(const InputParameters & parameters)
  : PressureParent<is_ad>(parameters),
    _pressure(this->template coupledGenericValue<is_ad>("pressure"))
{
}

template <bool is_ad>
GenericReal<is_ad>
CoupledPressureBCTempl<is_ad>::computePressure() const
{
  return _pressure[_qp];
}

template class CoupledPressureBCTempl<true>;
template class CoupledPressureBCTempl<false>;
