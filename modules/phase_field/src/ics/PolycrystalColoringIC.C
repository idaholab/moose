/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PolycrystalColoringIC.h"
#include "IndirectSort.h"
#include "MooseMesh.h"
#include "MooseRandom.h"
#include "NonlinearSystemBase.h"
#include "GrainTrackerInterface.h"
#include "PolycrystalUserObjectBase.h"

template <>
InputParameters
validParams<PolycrystalColoringIC>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addClassDescription(
      "Random Voronoi tesselation polycrystal (used by PolycrystalVoronoiICAction)");
  params.addRequiredParam<UserObjectName>("polycrystal_ic_uo", "TODO");
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
