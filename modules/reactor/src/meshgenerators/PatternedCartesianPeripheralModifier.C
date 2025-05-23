//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PatternedCartesianPeripheralModifier.h"

#include "MooseMeshUtils.h"

registerMooseObject("ReactorApp", PatternedCartesianPeripheralModifier);

InputParameters
PatternedCartesianPeripheralModifier::validParams()
{
  InputParameters params = PatternedPolygonPeripheralModifierBase::validParams();
  return params;
}

PatternedCartesianPeripheralModifier::PatternedCartesianPeripheralModifier(
    const InputParameters & parameters)
  : PatternedPolygonPeripheralModifierBase(parameters)
{
  _num_sides = SQUARE_NUM_SIDES;
  declareMeshProperty<bool>("square_peripheral_trimmability", false);
  declareMeshProperty<bool>("square_center_trimmability", false);
}

std::unique_ptr<MeshBase>
PatternedCartesianPeripheralModifier::generate()
{
  if (hasMeshProperty<bool>("square_center_trimmability", _input_name))
    setMeshProperty("square_center_trimmability",
                    getMeshProperty<bool>("square_center_trimmability", _input_name));

  // Check if the input mesh is compatible
  if (!getMeshProperty<bool>("peripheral_modifier_compatible", _input_name))
    paramError("input",
               "The input mesh is generated by a mesh generator that is not compatible with "
               "PatternedCartesianPeripheralModifier.");
  // Whether the input mesh is cartesian
  if (!hasMeshProperty<bool>("square_center_trimmability", _input_name))
    paramError("input", "The input mesh is not declared as center-trimmable in its metadata.");

  return PatternedPolygonPeripheralModifierBase::generate();
}
