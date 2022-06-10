//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PeriodicDistanceAux.h"
#include "GeneratedMesh.h"

registerMooseObject("MooseTestApp", PeriodicDistanceAux);

InputParameters
PeriodicDistanceAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredParam<Point>("point", "Some point in the domain");

  return params;
}

PeriodicDistanceAux::PeriodicDistanceAux(const InputParameters & parameters)
  : AuxKernel(parameters), _point(getParam<Point>("point"))
{
  // Make sure the point is in the domain
  for (const auto i : make_range(Moose::dim))
    if (_point(i) < _mesh.getMinInDimension(i) || _point(i) > _mesh.getMaxInDimension(i))
      paramError("point",
                 _mesh.getMinInDimension(i),
                 "\t",
                 _mesh.getMaxInDimension(i),
                 "\n\"point\" is outside of the domain.");
}

Real
PeriodicDistanceAux::computeValue()
{
  // Compute the periodic distance from a given feature
  // Note: For this test kernel we are just going to use the first nonlinear variable (index: 0)
  return _mesh.minPeriodicDistance(0, *_current_node, _point);
}
