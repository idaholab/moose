//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialRealVectorValueAux.h"
#include "metaphysicl/raw_type.h"
#include <type_traits>

registerMooseObject("MooseApp", MaterialRealVectorValueAux);
registerMooseObject("MooseApp", ADMaterialRealVectorValueAux);
registerMooseObject("MooseApp", MaterialSymmetricRankTwoTensorAux);
registerMooseObject("MooseApp", ADMaterialSymmetricRankTwoTensorAux);

template <typename T, bool is_ad>
InputParameters
MaterialRealVectorValueAuxTempl<T, is_ad>::validParams()
{
  InputParameters params = MaterialAuxBaseTempl<T, is_ad>::validParams();
  params.addClassDescription(
      "Capture a component of a vector material property in an auxiliary variable.");
  params.addParam<unsigned int>("component", 0, "The vector component to consider for this kernel");

  return params;
}

template <typename T, bool is_ad>
MaterialRealVectorValueAuxTempl<T, is_ad>::MaterialRealVectorValueAuxTempl(
    const InputParameters & parameters)
  : MaterialAuxBaseTempl<T, is_ad>(parameters),
    _component(this->template getParam<unsigned int>("component"))
{
  if constexpr (std::is_same_v<T, RealVectorValue>)
  {
    if (_component > LIBMESH_DIM)
      this->mooseError("The component ",
                       _component,
                       " does not exist for ",
                       LIBMESH_DIM,
                       " dimensional problems");
  }
  else
  {
    if (_component > T::N)
      this->mooseError("The component ", _component, " does not exist.");
  }
}

template <typename T, bool is_ad>
Real
MaterialRealVectorValueAuxTempl<T, is_ad>::getRealValue()
{
  return MetaPhysicL::raw_value(this->_prop[this->_qp](_component));
}

template class MaterialRealVectorValueAuxTempl<RealVectorValue, false>;
template class MaterialRealVectorValueAuxTempl<RealVectorValue, true>;
template class MaterialRealVectorValueAuxTempl<SymmetricRankTwoTensor, false>;
template class MaterialRealVectorValueAuxTempl<SymmetricRankTwoTensor, true>;
