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
#include "MaterialBase.h"

registerMooseObject("TensorMechanicsApp", ProjectedStatefulMaterialAux);
registerMooseObject("TensorMechanicsApp", ADProjectedStatefulMaterialAux);

template <bool is_ad>
InputParameters
ProjectedStatefulMaterialAuxTempl<is_ad>::validParams()
{
  auto params = IndexableProperty<AuxKernel, is_ad>::validParams();
  params.addClassDescription(
      "Picks a component of an indexable material property to get projected on an elemental "
      "Auxvariable. For use by ProjectedStatefulMaterialStorageAction.");
  return params;
}

template <bool is_ad>
ProjectedStatefulMaterialAuxTempl<is_ad>::ProjectedStatefulMaterialAuxTempl(
    const InputParameters & parameters)
  : AuxKernel(parameters), _prop(this), _current_subdomain_id(_assembly.currentSubdomainID())
{
}

template <bool is_ad>
void
ProjectedStatefulMaterialAuxTempl<is_ad>::initialSetup()
{
  _prop.check();

  // get all material classes that provide properties for this object
  _all_materials = getSupplyerMaterials();
}

template <bool is_ad>
void
ProjectedStatefulMaterialAuxTempl<is_ad>::subdomainSetup()
{
  // get materials for the current subdomain
  _active_materials.clear();
  for (const auto & mat : _all_materials)
    if (mat->hasBlocks(_current_subdomain_id))
      _active_materials.push_back(mat);
}

template <bool is_ad>
Real
ProjectedStatefulMaterialAuxTempl<is_ad>::computeValue()
{
  if (_t_step == 0)
    for (const auto & mat : _active_materials)
      mat->initStatefulProperties(_qrule->size());

  return MetaPhysicL::raw_value(_prop[_qp]);
}

template class ProjectedStatefulMaterialAuxTempl<false>;
template class ProjectedStatefulMaterialAuxTempl<true>;
