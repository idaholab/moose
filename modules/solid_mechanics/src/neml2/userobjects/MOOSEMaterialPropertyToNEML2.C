//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MOOSEMaterialPropertyToNEML2.h"

#define RegisterMOOSEMaterialPropertyToNEML2(alias)                                                \
  registerMooseObject("SolidMechanicsApp", MOOSE##alias##MaterialPropertyToNEML2);                 \
  registerMooseObject("SolidMechanicsApp", MOOSEOld##alias##MaterialPropertyToNEML2)

RegisterMOOSEMaterialPropertyToNEML2(Real);
RegisterMOOSEMaterialPropertyToNEML2(RankTwoTensor);
RegisterMOOSEMaterialPropertyToNEML2(SymmetricRankTwoTensor);
RegisterMOOSEMaterialPropertyToNEML2(RealVectorValue);

template <typename T, unsigned int state>
InputParameters
MOOSEMaterialPropertyToNEML2<T, state>::validParams()
{
  auto params = MOOSEToNEML2Batched<T>::validParams();
  params.addClassDescription(NEML2Utils::docstring(
      "Gather a MOOSE material property of type " + demangle(typeid(T).name()) +
      " for insertion into the specified input or model parameter of a NEML2 model."));
  params.template addRequiredParam<MaterialPropertyName>(
      "from_moose", NEML2Utils::docstring("MOOSE material property to read from"));
  return params;
}

template <typename T, unsigned int state>
MOOSEMaterialPropertyToNEML2<T, state>::MOOSEMaterialPropertyToNEML2(const InputParameters & params)
  : MOOSEToNEML2Batched<T>(params)
#ifdef NEML2_ENABLED
    ,
    _mat_prop(this->template getGenericMaterialProperty<T, false>("from_moose", state))
#endif
{
}

#define InstantiateMOOSEMaterialPropertyToNEML2(T)                                                 \
  template class MOOSEMaterialPropertyToNEML2<T, 0>;                                               \
  template class MOOSEMaterialPropertyToNEML2<T, 1>

InstantiateMOOSEMaterialPropertyToNEML2(Real);
InstantiateMOOSEMaterialPropertyToNEML2(RankTwoTensor);
InstantiateMOOSEMaterialPropertyToNEML2(SymmetricRankTwoTensor);
InstantiateMOOSEMaterialPropertyToNEML2(RealVectorValue);
