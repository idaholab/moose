//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialRealAux.h"

registerMooseObject("MooseApp", MaterialRealAux);
registerMooseObject("MooseApp", ADMaterialRealAux);
registerMooseObject("MooseApp", FunctorMaterialRealAux);
registerMooseObject("MooseApp", ADFunctorMaterialRealAux);

template <bool is_ad, bool is_functor>
InputParameters
MaterialRealAuxTempl<is_ad, is_functor>::validParams()
{
  InputParameters params = MaterialAuxBaseTempl<Real, is_ad, is_functor>::validParams();
  params.addClassDescription("Outputs element volume-averaged material properties");
  return params;
}

template <bool is_ad, bool is_functor>
MaterialRealAuxTempl<is_ad, is_functor>::MaterialRealAuxTempl(const InputParameters & parameters)
  : MaterialAuxBaseTempl<Real, is_ad, is_functor>(parameters)
{
}

template <bool is_ad, bool is_functor>
Real
MaterialRealAuxTempl<is_ad, is_functor>::getRealValue()
{
  return MetaPhysicL::raw_value(this->_full_value);
}

template class MaterialRealAuxTempl<false, false>;
template class MaterialRealAuxTempl<true, false>;
template class MaterialRealAuxTempl<false, true>;
template class MaterialRealAuxTempl<true, true>;
