//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PlaneDeletionGenerator.h"

#include "libmesh/type_vector.h"
#include "libmesh/point.h"
#include "libmesh/elem.h"

registerMooseObject("MooseApp", PlaneDeletionGenerator);

InputParameters
PlaneDeletionGenerator::validParams()
{
  InputParameters params = ElementDeletionGeneratorBase::validParams();

  params.addClassDescription(
      "Removes elements lying 'above' the plane (in the direction of the normal).");

  params.addRequiredParam<Point>("point", "The point that defines the plane");
  params.addRequiredParam<RealVectorValue>("normal", "The normal that defines the plane");

  return params;
}

PlaneDeletionGenerator::PlaneDeletionGenerator(const InputParameters & parameters)
  : ElementDeletionGeneratorBase(parameters),
    _point(getParam<Point>("point")),
    _normal(getParam<RealVectorValue>("normal"))
{
  if (!_normal.norm())
    paramError("normal", "Normal vector must have a size!");

  // Make sure it's a unit vector
  _normal /= _normal.norm();
}

bool
PlaneDeletionGenerator::shouldDelete(const Elem * elem)
{
  auto centroid = elem->vertex_average();

  auto vec_from_plane_point = centroid - _point;

  auto norm = vec_from_plane_point.norm();

  // If we _perfectly_ hit a centroid... default to deleting the element
  if (!norm)
    return true;

  // Unitize it
  vec_from_plane_point /= norm;

  return vec_from_plane_point * _normal > 0;
}
