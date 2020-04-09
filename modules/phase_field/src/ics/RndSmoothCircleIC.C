//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RndSmoothCircleIC.h"

// MOOSE includes
#include "MooseMesh.h"
#include "MooseVariable.h"

registerMooseObject("PhaseFieldApp", RndSmoothCircleIC);

InputParameters
RndSmoothCircleIC::validParams()
{
  InputParameters params = SmoothCircleIC::validParams();
  params.addClassDescription(
      "Random noise with different min/max inside/outside of a smooth circle");
  params.addRequiredParam<Real>("variation_invalue", "Plus or minus this amount on the invalue");
  params.addRequiredParam<Real>("variation_outvalue", "Plus or minus this amount on the outvalue");
  return params;
}

RndSmoothCircleIC::RndSmoothCircleIC(const InputParameters & parameters)
  : SmoothCircleIC(parameters),
    _variation_invalue(parameters.get<Real>("variation_invalue")),
    _variation_outvalue(parameters.get<Real>("variation_outvalue"))
{
  if (_profile == ProfileType::TANH)
    paramError("profile", "Hyperbolic tangent profile is not supported for this IC");
}

Real
RndSmoothCircleIC::computeCircleValue(const Point & p, const Point & center, const Real & radius)
{
  Point l_center = center;
  Point l_p = p;
  if (!_3D_spheres) // Create 3D cylinders instead of spheres
  {
    l_p(2) = 0.0;
    l_center(2) = 0.0;
  }
  // Compute the distance between the current point and the center
  Real dist = _mesh.minPeriodicDistance(_var.number(), l_p, l_center);

  // Return value
  Real value = 0.0;

  if (dist <= radius - _int_width / 2.0) // Random value inside circle
    value = _invalue - _variation_invalue + 2.0 * _random.rand(_tid) * _variation_invalue;
  else if (dist < radius + _int_width / 2.0) // Smooth interface
  {
    Real int_pos = (dist - radius + _int_width / 2.0) / _int_width;
    value = _outvalue + (_invalue - _outvalue) * (1.0 + std::cos(int_pos * libMesh::pi)) / 2.0;
  }
  else // Random value outside circle
    value = _outvalue - _variation_outvalue + 2.0 * _random.rand(_tid) * _variation_outvalue;

  return value;
}
