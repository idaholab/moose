//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NearestPositionsDivision.h"
#include "MooseMesh.h"
#include "Positions.h"

#include "libmesh/mesh_base.h"
#include "libmesh/elem.h"

registerMooseObject("MooseApp", NearestPositionsDivision);

InputParameters
NearestPositionsDivision::validParams()
{
  InputParameters params = MeshDivision::validParams();
  params.addClassDescription("Divide the mesh using a nearest-point / voronoi algorithm, with the "
                             "points coming from a Positions object");
  params.addRequiredParam<PositionsName>(
      "positions", "The name of the Positions object to form the nearest-neighbor division with");
  return params;
}

NearestPositionsDivision::NearestPositionsDivision(const InputParameters & parameters)
  : MeshDivision(parameters),
    _nearest_positions_obj(&_fe_problem->getPositionsObject(getParam<PositionsName>("positions")))
{
  NearestPositionsDivision::initialize();
  _mesh_fully_indexed = true;
}

void
NearestPositionsDivision::initialize()
{
  setNumDivisions(_nearest_positions_obj->getNumPositions());
}

unsigned int
NearestPositionsDivision::divisionIndex(const Elem & elem) const
{
  const bool initial = _fe_problem->getCurrentExecuteOnFlag() == EXEC_INITIAL;
  return _nearest_positions_obj->getNearestPositionIndex(elem.vertex_average(), initial);
}

unsigned int
NearestPositionsDivision::divisionIndex(const Point & pt) const
{
  const bool initial = _fe_problem->getCurrentExecuteOnFlag() == EXEC_INITIAL;
  return _nearest_positions_obj->getNearestPositionIndex(pt, initial);
}
