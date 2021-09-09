//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiscreteNucleationMarker.h"
#include "DiscreteNucleationMap.h"

registerMooseObject("PhaseFieldApp", DiscreteNucleationMarker);

InputParameters
DiscreteNucleationMarker::validParams()
{
  InputParameters params = Marker::validParams();
  params.addClassDescription("Mark new nucleation sites for refinement");
  params.addRequiredParam<UserObjectName>("map", "DiscreteNucleationMap user object");
  return params;
}

DiscreteNucleationMarker::DiscreteNucleationMarker(const InputParameters & parameters)
  : Marker(parameters),
    _map(getUserObject<DiscreteNucleationMap>("map")),
    _periodic(_map.getPeriodic()),
    _inserter(_map.getInserter()),
    _int_width(_map.getWidth()),
    _nucleus_list(_inserter.getNucleusList())
{
}

Marker::MarkerValue
DiscreteNucleationMarker::computeElementMarker()
{
  const RealVectorValue centroid = _current_elem->vertex_average();
  const Real size = 0.5 * _current_elem->hmax();

  // check if the surface of a nucleus might touch the element
  for (unsigned i = 0; i < _nucleus_list.size(); ++i)
  {
    // get the radius of the current nucleus
    const Real radius = _nucleus_list[i].radius;

    // use a non-periodic or periodic distance
    const Real r = _periodic < 0
                       ? (centroid - _nucleus_list[i].center).norm()
                       : _mesh.minPeriodicDistance(_periodic, centroid, _nucleus_list[i].center);
    if (r < radius + size && r > radius - size && size > _int_width)
      return REFINE;
  }

  // We return don't mark to allow coarsening when used in a ComboMarker
  return DONT_MARK;
}
