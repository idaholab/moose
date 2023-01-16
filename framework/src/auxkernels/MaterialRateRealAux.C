//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialRateRealAux.h"

registerMooseObject("MooseApp", MaterialRateRealAux);
registerMooseObject("MooseApp", ADMaterialRateRealAux);

template <bool is_ad>
InputParameters
MaterialRateRealAuxTempl<is_ad>::validParams()
{
  InputParameters params = MaterialAuxBaseTempl<Real, is_ad>::validParams();
  params.addClassDescription("Outputs element material properties rate of change");
  return params;
}

template <bool is_ad>
MaterialRateRealAuxTempl<is_ad>::MaterialRateRealAuxTempl(const InputParameters & parameters)
  : MaterialRateAuxBaseTempl<Real, is_ad>(parameters)
{
}

template <bool is_ad>
Real
MaterialRateRealAuxTempl<is_ad>::getRealValue()
{
  Real prop = MetaPhysicL::raw_value(this->_prop[this->_qp]);
  Real prop_old = MetaPhysicL::raw_value(this->_prop_old[this->_qp]);
  Real rate = (prop - prop_old) / this->_dt;
  return rate;
}

template class MaterialRateRealAuxTempl<false>;
template class MaterialRateRealAuxTempl<true>;
