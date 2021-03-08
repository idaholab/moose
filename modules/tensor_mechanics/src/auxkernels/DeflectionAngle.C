//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DeflectionAngle.h"

registerMooseObject("TensorMechanicsApp", DeflectionAngle);

InputParameters
DeflectionAngle::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredParam<Point>("origin", "Axis origin");
  params.addParam<RealVectorValue>("direction", "Axis direction");
  params.addClassDescription("Compute the a deflection angle of the displaced mesh points "
                             "w.r.t. a point and an optional direction");
  params.addRequiredCoupledVar("displacements", "The displacements");
  params.set<bool>("use_displaced_mesh") = false;
  return params;
}

DeflectionAngle::DeflectionAngle(const InputParameters & parameters)
  : AuxKernel(parameters),
    _origin(getParam<Point>("origin")),
    _has_direction(isParamValid("direction")),
    _disp(coupledValues("displacements"))
{
  // get normal direction
  if (_has_direction)
  {
    _direction = getParam<RealVectorValue>("direction");
    _direction /= _direction.norm();
  }

  // make sure we are running on the undisplaced mesh and are operating on a nodal variable
  if (getParam<bool>("use_displaced_mesh"))
    paramError("use_displaced_mesh", "This AuxKernel must be run on the undisplaced mesh");
  if (!isNodal())
    paramError("variable", "This AuxKernel must operate on a nodal variable");
}

Real
DeflectionAngle::computeValue()
{
  // displacement vector
  RealVectorValue delta;
  for (unsigned int i = 0; i < LIBMESH_DIM && i < _disp.size(); ++i)
    delta(i) = (*_disp[i])[_qp];

  // undisplaced and displaced locations relative to the origin.
  RealVectorValue dr1 = *_current_node - _origin;
  RealVectorValue dr2 = dr1 + delta;

  // optionally project into plane defined by direction vector
  if (_has_direction)
  {
    // subtract out of plane projections
    dr1 -= _direction * (_direction * dr1);
    dr2 -= _direction * (_direction * dr2);
  }

  // product of the lengths
  auto norms = std::sqrt(dr1.norm_sq() * dr2.norm_sq());

  // angle between dr1 and dr2
  if (norms > libMesh::TOLERANCE)
    return std::acos((dr1 * dr2) / norms);
  else
    return 0.0;
}
