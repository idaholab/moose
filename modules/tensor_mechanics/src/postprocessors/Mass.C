//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

//  This post processor returns the mass value of an element.  It is used
//  to check that mass is conserved (per the evolving density calculation)
//  when volume changes occur.
//
#include "Mass.h"

#include "metaphysicl/raw_type.h"

registerMooseObject("TensorMechanicsApp", Mass);
registerMooseObject("TensorMechanicsApp", ADMass);

template <bool is_ad>
InputParameters
MassTempl<is_ad>::validParams()
{
  InputParameters params = ElementIntegralVariablePostprocessor::validParams();
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

template <bool is_ad>
MassTempl<is_ad>::MassTempl(const InputParameters & parameters)
  : ElementIntegralVariablePostprocessor(parameters),
    _density(getGenericMaterialProperty<Real, is_ad>("density"))
{
}

template <bool is_ad>
Real
MassTempl<is_ad>::computeQpIntegral()
{
  return MetaPhysicL::raw_value(_density[_qp]);
}

template class MassTempl<false>;
template class MassTempl<true>;
