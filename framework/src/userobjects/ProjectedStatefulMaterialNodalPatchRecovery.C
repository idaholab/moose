//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ProjectedStatefulMaterialNodalPatchRecovery.h"
#include "MaterialBase.h"
#include "Assembly.h"

registerMooseObject("MooseApp", ProjectedStatefulMaterialNodalPatchRecovery);

InputParameters
ProjectedStatefulMaterialNodalPatchRecovery::validParams()
{
  InputParameters params = NodalPatchRecoveryMaterialProperty::validParams();
  return params;
}

ProjectedStatefulMaterialNodalPatchRecovery::ProjectedStatefulMaterialNodalPatchRecovery(
    const InputParameters & parameters)
  : NodalPatchRecoveryMaterialProperty(parameters),
    _current_subdomain_id(_assembly.currentSubdomainID())
{
}

void
ProjectedStatefulMaterialNodalPatchRecovery::initialSetup()
{
  NodalPatchRecoveryMaterialProperty::initialSetup();

  // get all material classes that provide properties for this object
  _required_materials = buildRequiredMaterials();
}

Real
ProjectedStatefulMaterialNodalPatchRecovery::computeValue()
{
  if (_t_step == 0)
    for (const auto & mat : _required_materials[_current_subdomain_id])
      mat->initStatefulProperties(_qrule->size());

  return _prop[_qp];
}
