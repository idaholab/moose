//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CrackTipEnrichmentCutOffBC.h"

template <>
InputParameters
validParams<CrackTipEnrichmentCutOffBC>()
{
  InputParameters p = validParams<PresetBC>();
  p.addRequiredParam<Real>("cut_off_radius",
                           "The cut off radius of crack tip enrichment functions");
  p.set<bool>("use_displaced_mesh") = false;
  p.addRequiredParam<UserObjectName>("crack_front_definition",
                                     "The CrackFrontDefinition user object name");
  return p;
}

CrackTipEnrichmentCutOffBC::CrackTipEnrichmentCutOffBC(const InputParameters & parameters)
  : PresetBC(parameters),
    _cut_off_radius(getParam<Real>("cut_off_radius")),
    _crack_front_definition(&getUserObject<CrackFrontDefinition>("crack_front_definition"))
{
}

bool
CrackTipEnrichmentCutOffBC::shouldApply()
{
  Real r, theta;
  _crack_front_definition->calculateRThetaToCrackFront((*_current_node), r, theta);

  if (r > _cut_off_radius)
    return true;
  else
    return false;
}
