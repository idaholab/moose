//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolycrystalColoringIC.h"
#include "IndirectSort.h"
#include "MooseMesh.h"
#include "MooseRandom.h"
#include "NonlinearSystemBase.h"
#include "GrainTrackerInterface.h"
#include "PolycrystalUserObjectBase.h"

registerMooseObject("PhaseFieldApp", PolycrystalColoringIC);

InputParameters
PolycrystalColoringIC::validParams()
{
  InputParameters params = InitialCondition::validParams();
  params.addClassDescription(
      "Random Voronoi tesselation polycrystal (used by PolycrystalVoronoiICAction)");
  params.addRequiredParam<UserObjectName>("polycrystal_ic_uo",
                                          "User object generating a point to grain number mapping");
  params.addRequiredParam<unsigned int>("op_index", "The index for the current order parameter");

  return params;
}

PolycrystalColoringIC::PolycrystalColoringIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _op_index(getParam<unsigned int>("op_index")),
    _poly_ic_uo(getUserObject<PolycrystalUserObjectBase>("polycrystal_ic_uo"))
{
}

Real
PolycrystalColoringIC::value(const Point & p)
{
  if (_current_node)
    return _poly_ic_uo.getNodalVariableValue(_op_index, *_current_node);
  else
    return _poly_ic_uo.getVariableValue(_op_index, p);
}
