//*This file is part of the MOOSE framework
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

template <bool is_ad>
InputParameters
MaterialRealAuxTempl<is_ad>::validParams()
{
  InputParameters params = MaterialAuxBaseTempl<Real, is_ad>::validParams();
  params.addClassDescription("Outputs element volume-averaged material properties");
  return params;
}

template <bool is_ad>
MaterialRealAuxTempl<is_ad>::MaterialRealAuxTempl(const InputParameters & parameters)
  : MaterialAuxBaseTempl<Real, is_ad>(parameters)
{
}

template <bool is_ad>
Real
MaterialRealAuxTempl<is_ad>::getRealValue()
{
  return MetaPhysicL::raw_value(this->_prop[this->_qp]);
}

template class MaterialRealAuxTempl<false>;
template class MaterialRealAuxTempl<true>;
