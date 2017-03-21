/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "RndSmoothCircleIC.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<RndSmoothCircleIC>()
{
  InputParameters params = validParams<SmoothCircleIC>();
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
