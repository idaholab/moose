/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PolycrystalReducedIC.h"
#include "IndirectSort.h"
#include "MooseMesh.h"
#include "MooseRandom.h"
#include "NonlinearSystemBase.h"
#include "GrainTrackerInterface.h"
#include "PolycrystalUserObjectBase.h"

template <>
InputParameters
validParams<PolycrystalReducedIC>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addClassDescription(
      "Random Voronoi tesselation polycrystal (used by PolycrystalVoronoiICAction)");
  params.addRequiredParam<unsigned int>("op_index", "The index for the current order parameter");
  params.addRequiredParam<UserObjectName>("polycrystal_ic_uo", "TODO");

  return params;
}

PolycrystalReducedIC::PolycrystalReducedIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _op_index(getParam<unsigned int>("op_index")),
    _poly_ic_uo(getUserObject<PolycrystalUserObjectBase>("polycrystal_ic_uo"))
{
}

Real
PolycrystalReducedIC::value(const Point & p)
{
  const auto grain_to_op = _poly_ic_uo.getGrainToOps();
  auto grain_id = _poly_ic_uo.getGrainBasedOnPoint(p);
  auto assigned_op = grain_to_op[grain_id];

  return (assigned_op == _op_index) ? 1.0 : 0.0;
}
