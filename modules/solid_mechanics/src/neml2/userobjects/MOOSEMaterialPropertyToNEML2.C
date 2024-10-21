//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MOOSEMaterialPropertyToNEML2.h"

registerMooseObject("SolidMechanicsApp", MOOSERealMaterialPropertyToNEML2);
registerMooseObject("SolidMechanicsApp", MOOSERankTwoTensorMaterialPropertyToNEML2);
registerMooseObject("SolidMechanicsApp", MOOSESymmetricRankTwoTensorMaterialPropertyToNEML2);
registerMooseObject("SolidMechanicsApp", MOOSEStdVectorRealMaterialPropertyToNEML2);

registerMooseObject("SolidMechanicsApp", MOOSEOldRealMaterialPropertyToNEML2);
registerMooseObject("SolidMechanicsApp", MOOSEOldRankTwoTensorMaterialPropertyToNEML2);
registerMooseObject("SolidMechanicsApp", MOOSEOldSymmetricRankTwoTensorMaterialPropertyToNEML2);
registerMooseObject("SolidMechanicsApp", MOOSEOldStdVectorRealMaterialPropertyToNEML2);

#ifndef NEML2_ENABLED
#define MOOSEMaterialPropertyToNEML2Stub(name)                                                     \
  NEML2ObjectStubImplementationOpen(name, ElementUserObject);                                      \
  NEML2ObjectStubParam(MaterialPropertyName, "moose_material_property");                           \
  NEML2ObjectStubParam(std::string, "neml2_variable");                                             \
  NEML2ObjectStubImplementationClose(name, ElementUserObject)

MOOSEMaterialPropertyToNEML2Stub(MOOSERealMaterialPropertyToNEML2);
MOOSEMaterialPropertyToNEML2Stub(MOOSERankTwoTensorMaterialPropertyToNEML2);
MOOSEMaterialPropertyToNEML2Stub(MOOSESymmetricRankTwoTensorMaterialPropertyToNEML2);
MOOSEMaterialPropertyToNEML2Stub(MOOSEStdVectorRealMaterialPropertyToNEML2);

MOOSEMaterialPropertyToNEML2Stub(MOOSEOldRealMaterialPropertyToNEML2);
MOOSEMaterialPropertyToNEML2Stub(MOOSEOldRankTwoTensorMaterialPropertyToNEML2);
MOOSEMaterialPropertyToNEML2Stub(MOOSEOldSymmetricRankTwoTensorMaterialPropertyToNEML2);
MOOSEMaterialPropertyToNEML2Stub(MOOSEOldStdVectorRealMaterialPropertyToNEML2);
#else

template <typename T, unsigned int state>
InputParameters
MOOSEMaterialPropertyToNEML2<T, state>::validParams()
{
  auto params = MOOSEToNEML2::validParams();
  params.addClassDescription("Gather a MOOSE material property of type " +
                             demangle(typeid(T).name()) +
                             " for insertion into the specified input of a "
                             "NEML2 model.");

  params.addRequiredParam<MaterialPropertyName>("moose_material_property",
                                                "MOOSE material property to read from");
  return params;
}

template <typename T, unsigned int state>
MOOSEMaterialPropertyToNEML2<T, state>::MOOSEMaterialPropertyToNEML2(const InputParameters & params)
  : MOOSEToNEML2(params),
    _mat_prop(getGenericMaterialProperty<T, false>("moose_material_property", state))
{
  if constexpr (state == 0)
    if (!_neml2_variable.start_with("forces") && !_neml2_variable.start_with("state"))
      paramError("neml2_variable",
                 "neml2_variable should be defined on the forces or the state sub-axis, got ",
                 _neml2_variable.slice(0, 1),
                 " instead");

  if constexpr (state == 1)
    if (!_neml2_variable.start_with("old_forces") && !_neml2_variable.start_with("old_state"))
      paramError(
          "neml2_variable",
          "neml2_variable should be defined on the old_forces or the old_state sub-axis, got ",
          _neml2_variable.slice(0, 1),
          " instead");
}

template <typename T, unsigned int state>
torch::Tensor
MOOSEMaterialPropertyToNEML2<T, state>::convertQpMOOSEData() const
{
  return NEML2Utils::toNEML2<T>(_mat_prop[_qp]);
}

template class MOOSEMaterialPropertyToNEML2<Real, 0>;
template class MOOSEMaterialPropertyToNEML2<RankTwoTensor, 0>;
template class MOOSEMaterialPropertyToNEML2<SymmetricRankTwoTensor, 0>;
template class MOOSEMaterialPropertyToNEML2<std::vector<Real>, 0>;

template class MOOSEMaterialPropertyToNEML2<Real, 1>;
template class MOOSEMaterialPropertyToNEML2<RankTwoTensor, 1>;
template class MOOSEMaterialPropertyToNEML2<SymmetricRankTwoTensor, 1>;
template class MOOSEMaterialPropertyToNEML2<std::vector<Real>, 1>;

#endif
