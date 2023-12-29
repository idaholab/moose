//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RotationAngle.h"

registerMooseObject("TensorMechanicsApp", RotationAngle);

InputParameters
RotationAngle::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredParam<Point>("origin", "Axis origin");
  params.addRequiredParam<RealVectorValue>("direction", "Axis direction");
  params.addClassDescription("Compute the field of angular rotations of points around an axis "
                             "defined by an origin point and a direction vector");
  params.addRequiredCoupledVar("displacements", "The displacements");
  params.set<bool>("use_displaced_mesh") = false;
  return params;
}

RotationAngle::RotationAngle(const InputParameters & parameters)
  : AuxKernel(parameters),
    _origin(getParam<Point>("origin")),
    _direction(getParam<RealVectorValue>("direction")),
    _disp(coupledValues("displacements"))
{
  // normalize direction
  _direction /= _direction.norm();

  // sanity checks
  if (getParam<bool>("use_displaced_mesh"))
    paramError("use_displaced_mesh", "This AuxKernel must be run on the undisplaced mesh");
  if (!isNodal())
    paramError("variable", "This AuxKernel must operate on a nodal variable");
  if (_disp.size() > LIBMESH_DIM)
    paramError("displacements",
               "Too many displacement variables were specified. The max is LIBMESH_DIM, which is ",
               LIBMESH_DIM);
}

Real
RotationAngle::computeValue()
{
  // displacement vector
  RealVectorValue delta;
  for (unsigned int i = 0; i < _disp.size(); ++i)
    delta(i) = (*_disp[i])[_qp];

  // undisplaced and displaced locations relative to the origin.
  RealVectorValue dr1 = *_current_node - _origin;
  RealVectorValue dr2 = dr1 + delta;

  // subtract out of plane projections
  dr1 -= _direction * (_direction * dr1);
  dr2 -= _direction * (_direction * dr2);

  // product of the lengths
  auto norms = std::sqrt(dr1.norm_sq() * dr2.norm_sq());

  // angle between dr1 and dr2
  if (norms > libMesh::TOLERANCE)
    return std::acos((dr1 * dr2) / norms) * ((dr1.cross(dr2) * _direction) > 0 ? 1.0 : -1.0);
  else
    return 0.0;
}
