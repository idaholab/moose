//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialStdVectorAux.h"

registerMooseObject("MooseApp", MaterialStdVectorAux);
registerMooseObject("MooseApp", ADMaterialStdVectorAux);

template <bool is_ad>
InputParameters
MaterialStdVectorAuxTempl<is_ad>::validParams()
{
  InputParameters params = MaterialStdVectorAuxBaseTempl<Real, is_ad>::validParams();
  params.addClassDescription("Extracts a component of a material type std::vector<Real> to an aux "
                             "variable.  If the std::vector is not of sufficient size then zero is "
                             "returned");
  return params;
}

template <bool is_ad>
MaterialStdVectorAuxTempl<is_ad>::MaterialStdVectorAuxTempl(const InputParameters & parameters)
  : MaterialStdVectorAuxBaseTempl<Real, is_ad>(parameters)
{
}

template <bool is_ad>
Real
MaterialStdVectorAuxTempl<is_ad>::getRealValue()
{
  if (this->_full_value.size() >= _index)
    return MetaPhysicL::raw_value(this->_full_value[_index]);
  else
    return 0.0;
}

template class MaterialStdVectorAuxTempl<false>;
template class MaterialStdVectorAuxTempl<true>;
