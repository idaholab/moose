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
registerMooseObject("MooseApp", FunctorMaterialRealVectorValueAux);
registerMooseObject("MooseApp", ADFunctorMaterialRealVectorValueAux);
registerMooseObject("MooseApp", MaterialSymmetricRankTwoTensorAux);
registerMooseObject("MooseApp", ADMaterialSymmetricRankTwoTensorAux);

template <typename T, bool is_ad, bool is_functor>
InputParameters
MaterialRealVectorValueAuxTempl<T, is_ad, is_functor>::validParams()
{
  InputParameters params = MaterialAuxBaseTempl<T, is_ad, is_functor>::validParams();
  params.addClassDescription(
      "Capture a component of a vector material property in an auxiliary variable.");
  params.addParam<unsigned int>("component", 0, "The vector component to consider for this kernel");

  return params;
}

template <typename T, bool is_ad, bool is_functor>
MaterialRealVectorValueAuxTempl<T, is_ad, is_functor>::MaterialRealVectorValueAuxTempl(
    const InputParameters & parameters)
  : MaterialAuxBaseTempl<T, is_ad, is_functor>(parameters),
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

template <typename T, bool is_ad, bool is_functor>
Real
MaterialRealVectorValueAuxTempl<T, is_ad, is_functor>::getRealValue()
{
  return MetaPhysicL::raw_value(this->_full_value(_component));
}

template class MaterialRealVectorValueAuxTempl<RealVectorValue, false, false>;
template class MaterialRealVectorValueAuxTempl<RealVectorValue, true, false>;
template class MaterialRealVectorValueAuxTempl<RealVectorValue, false, true>;
template class MaterialRealVectorValueAuxTempl<RealVectorValue, true, true>;
template class MaterialRealVectorValueAuxTempl<SymmetricRankTwoTensor, false, false>;
template class MaterialRealVectorValueAuxTempl<SymmetricRankTwoTensor, true, false>;
