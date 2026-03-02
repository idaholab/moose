//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MOOSEMaterialPropertyToNEML2.h"

#define registerMOOSEMaterialPropertyToNEML2(T)                                                    \
  registerMooseObject("MooseApp", MOOSE##T##MaterialPropertyToNEML2);                              \
  registerMooseObject("MooseApp", MOOSEOld##T##MaterialPropertyToNEML2);                           \
  registerMooseObject("MooseApp", MOOSEBoundary##T##MaterialPropertyToNEML2);                      \
  registerMooseObject("MooseApp", MOOSEBoundaryOld##T##MaterialPropertyToNEML2)

registerMOOSEMaterialPropertyToNEML2(Real);
registerMOOSEMaterialPropertyToNEML2(RankTwoTensor);
registerMOOSEMaterialPropertyToNEML2(SymmetricRankTwoTensor);
registerMOOSEMaterialPropertyToNEML2(RealVectorValue);

template <typename T, typename UOBase, unsigned int state>
InputParameters
MOOSEMaterialPropertyToNEML2<T, UOBase, state>::validParams()
{
  auto params = MOOSEToNEML2Batched<T, UOBase>::validParams();
  params.addClassDescription(
      "Gather a MOOSE material property of type " + demangle(typeid(T).name()) +
      " for insertion into the specified input or model parameter of a NEML2 model.");
  params.template addRequiredParam<MaterialPropertyName>("from_moose",
                                                         "MOOSE material property to read from");
  return params;
}

template <typename T, typename UOBase, unsigned int state>
MOOSEMaterialPropertyToNEML2<T, UOBase, state>::MOOSEMaterialPropertyToNEML2(
    const InputParameters & params)
  : MOOSEToNEML2Batched<T, UOBase>(params)
#ifdef NEML2_ENABLED
    ,
    _mat_prop(this->template getGenericMaterialProperty<T, false>("from_moose", state))
#endif
{
}

#define instantiateMOOSEMaterialPropertyToNEML2(T)                                                 \
  template class MOOSEMaterialPropertyToNEML2<T, ElementUserObject, 0>;                            \
  template class MOOSEMaterialPropertyToNEML2<T, ElementUserObject, 1>;                            \
  template class MOOSEMaterialPropertyToNEML2<T, SideUserObject, 0>;                               \
  template class MOOSEMaterialPropertyToNEML2<T, SideUserObject, 1>

instantiateMOOSEMaterialPropertyToNEML2(Real);
instantiateMOOSEMaterialPropertyToNEML2(RankTwoTensor);
instantiateMOOSEMaterialPropertyToNEML2(SymmetricRankTwoTensor);
instantiateMOOSEMaterialPropertyToNEML2(RealVectorValue);
