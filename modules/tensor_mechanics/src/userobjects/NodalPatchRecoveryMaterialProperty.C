//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalPatchRecoveryMaterialProperty.h"

registerMooseObject("TensorMechanicsApp", NodalPatchRecoveryMaterialProperty);

InputParameters
NodalPatchRecoveryMaterialProperty::validParams()
{
  InputParameters params = IndexableProperty<NodalPatchRecoveryBase, false>::validParams();
  params.addClassDescription(
      "Prepare patches for use in nodal patch recovery based on a material property.");
  return params;
}

NodalPatchRecoveryMaterialProperty::NodalPatchRecoveryMaterialProperty(
    const InputParameters & parameters)
  : NodalPatchRecoveryBase(parameters), _prop(this)
{
}
