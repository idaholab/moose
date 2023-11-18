//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ProjectedStatefulMaterialAux.h"
#include "NodalPatchRecoveryBase.h"
#include "SerialAccess.h"
#include "MaterialBase.h"
#include "Assembly.h"

registerMooseObject("MooseApp", ProjectedStatefulMaterialRealAux);
registerMooseObject("MooseApp", ADProjectedStatefulMaterialRealAux);
registerMooseObject("MooseApp", ProjectedStatefulMaterialRealVectorValueAux);
registerMooseObject("MooseApp", ADProjectedStatefulMaterialRealVectorValueAux);
registerMooseObject("MooseApp", ProjectedStatefulMaterialRankTwoTensorAux);
registerMooseObject("MooseApp", ADProjectedStatefulMaterialRankTwoTensorAux);
registerMooseObject("MooseApp", ProjectedStatefulMaterialRankFourTensorAux);
registerMooseObject("MooseApp", ADProjectedStatefulMaterialRankFourTensorAux);

template <typename T, bool is_ad>
InputParameters
ProjectedStatefulMaterialAuxTempl<T, is_ad>::validParams()
{
  auto params = AuxKernel::validParams();
  params.addClassDescription(
      "Picks a component of an indexable material property to get projected on an elemental "
      "Auxvariable. For use by ProjectedStatefulMaterialStorageAction.");
  params.addRequiredParam<MaterialPropertyName>(
      "prop", "material property to project onto an elemental aux variable");
  params.addRequiredParam<unsigned int>("component", "scalar component of the property to fetch");
  return params;
}

template <typename T, bool is_ad>
ProjectedStatefulMaterialAuxTempl<T, is_ad>::ProjectedStatefulMaterialAuxTempl(
    const InputParameters & parameters)
  : AuxKernel(parameters),
    _current_subdomain_id(_assembly.currentSubdomainID()),
    _prop(getGenericMaterialProperty<T, is_ad>("prop")),
    _component(getParam<unsigned int>("component"))
{
}

template <typename T, bool is_ad>
void
ProjectedStatefulMaterialAuxTempl<T, is_ad>::initialSetup()
{
  // get all material classes that provide properties for this object
  _required_materials = buildRequiredMaterials();
}

template <typename T, bool is_ad>
Real
ProjectedStatefulMaterialAuxTempl<T, is_ad>::computeValue()
{
  if (_t_step == 0)
    for (const auto & mat : _required_materials[_current_subdomain_id])
      mat->initStatefulProperties(_qrule->size());

  return MetaPhysicL::raw_value(Moose::serialAccess(_prop[_qp])[_component]);
}

template class ProjectedStatefulMaterialAuxTempl<Real, false>;
template class ProjectedStatefulMaterialAuxTempl<Real, true>;
template class ProjectedStatefulMaterialAuxTempl<RealVectorValue, false>;
template class ProjectedStatefulMaterialAuxTempl<RealVectorValue, true>;
template class ProjectedStatefulMaterialAuxTempl<RankTwoTensor, false>;
template class ProjectedStatefulMaterialAuxTempl<RankTwoTensor, true>;
template class ProjectedStatefulMaterialAuxTempl<RankFourTensor, false>;
template class ProjectedStatefulMaterialAuxTempl<RankFourTensor, true>;
